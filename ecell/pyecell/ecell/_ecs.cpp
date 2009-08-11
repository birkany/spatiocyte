//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//       This file is part of the E-Cell System
//
//       Copyright (C) 1996-2009 Keio University
//       Copyright (C) 2005-2008 The Molecular Sciences Institute
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// E-Cell System is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// E-Cell System is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with E-Cell System -- see the file COPYING.
// If not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// 
//END_HEADER
//
// written by Koichi Takahashi <shafi@e-cell.org> for
// E-Cell Project.
//


#include <cstring>
#include <cstdlib>
#include <utility>
#include <cctype>
#include <functional>

#include <boost/bind.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/range/size_type.hpp>
#include <boost/range/const_iterator.hpp>
#include <boost/cast.hpp>
#include <boost/format.hpp>
#include <boost/format/group.hpp>
#include <boost/python.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/object.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/object/inheritance.hpp>
#include <boost/python/object/find_instance.hpp>
#include <boost/python/reference_existing_object.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <numpy/arrayobject.h>
#include <stringobject.h>
#include <weakrefobject.h>

#include "dmtool/SharedModuleMakerInterface.hpp"

#include "libecs/Model.hpp"
#include "libecs/libecs.hpp"
#include "libecs/Process.hpp"
#include "libecs/Exceptions.hpp"
#include "libecs/Polymorph.hpp"
#include "libecs/DataPointVector.hpp"
#include "libecs/VariableReference.hpp"

using namespace libecs;
namespace py = boost::python;

inline py::object generic_getattr( py::object anObj, const char* aName )
{
    py::handle<> aRetval( py::allow_null( PyObject_GenericGetAttr(
        anObj.ptr(),
        py::handle<>(
            PyString_InternFromString(
                const_cast< char* >( aName ) ) ).get() ) ) );
    if ( !aRetval )
    {
        py::throw_error_already_set();
    }

    return py::object( aRetval );
}


struct PolymorphToPythonConverter
{
    static void addToRegistry()
    {
        py::to_python_converter< Polymorph, PolymorphToPythonConverter >();
    }

    static PyObject* convert( Polymorph const& aPolymorph )
    {
        switch( aPolymorph.getType() )
        {
        case PolymorphValue::REAL :
            return PyFloat_FromDouble( aPolymorph.as<Real>() );
        case PolymorphValue::INTEGER :
            return PyInt_FromLong( aPolymorph.as<Integer>() );
        case PolymorphValue::TUPLE :
            return rangeToPyTuple( aPolymorph.as<PolymorphValue::Tuple const&>() );
        case PolymorphValue::STRING :
            return PyString_FromStringAndSize(
                static_cast< const char * >(
                    aPolymorph.as< PolymorphValue::RawString const& >() ),
                aPolymorph.as< PolymorphValue::RawString const& >().size() );
        case PolymorphValue::NONE :
            return 0;
        }
        NEVER_GET_HERE;
    }

    template< typename Trange_ >
    static PyObject* 
    rangeToPyTuple( Trange_ const& aRange )
    {
        typename boost::range_size< Trange_ >::type
                aSize( boost::size( aRange ) );
        
        PyObject* aPyTuple( PyTuple_New( aSize ) );
       
        typename boost::range_const_iterator< Trange_ >::type j( boost::begin( aRange ) );
        for( std::size_t i( 0 ) ; i < aSize ; ++i, ++j )
        {
            PyTuple_SetItem( aPyTuple, i, PolymorphToPythonConverter::convert( *j ) );
        }
        
        return aPyTuple;
    }
};

struct PolymorphRetriever
{
    struct PySeqSTLIterator
    {
    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef Py_ssize_t difference_type;
        typedef Polymorph value_type;
        typedef void pointer;
        typedef Polymorph reference;

        PySeqSTLIterator( PyObject* seq, difference_type idx )
            : theSeq( seq ), theIdx( idx ) {}

        PySeqSTLIterator& operator++()
        {
            ++theIdx;
            return *this;
        }

        PySeqSTLIterator operator++( int )
        {
            PySeqSTLIterator retval( *this );
            ++theIdx;
            return retval;
        }

        PySeqSTLIterator& operator--()
        {
            --theIdx;
            return *this;
        }

        PySeqSTLIterator operator--( int )
        {
            PySeqSTLIterator retval( *this );
            --theIdx;
            return retval;
        }

        PySeqSTLIterator operator+( difference_type offset ) const
        {
            return PySeqSTLIterator( theSeq, theIdx + offset );
        }

        PySeqSTLIterator operator-( difference_type offset ) const
        {
            return PySeqSTLIterator( theSeq, theIdx - offset );
        }

        difference_type operator-( PySeqSTLIterator const& rhs ) const
        {
            return theIdx - rhs.theIdx;
        }

        PySeqSTLIterator& operator+=( difference_type offset )
        {
            theIdx += offset;
            return *this;
        }

        PySeqSTLIterator& operator-=( difference_type offset )
        {
            theIdx -= offset;
            return *this;
        }

        bool operator==( PySeqSTLIterator const& rhs )
        {
            return theIdx == rhs.theIdx;
        }

        bool operator!=( PySeqSTLIterator const& rhs )
        {
            return !operator==( rhs );
        }

        bool operator<( PySeqSTLIterator const& rhs )
        {
            return theIdx < rhs.theIdx;
        }

        bool operator>=( PySeqSTLIterator const& rhs )
        {
            return theIdx >= rhs.theIdx;
        }

        bool operator>( PySeqSTLIterator const& rhs )
        {
            return theIdx > rhs.theIdx;
        }

        bool operator<=( PySeqSTLIterator const& rhs )
        {
            return theIdx <= rhs.theIdx;
        }

        value_type operator*();
        
    private:
        PyObject* theSeq;
        Py_ssize_t theIdx;
    };

    static boost::iterator_range< PySeqSTLIterator > pyseq_range( PyObject *pyo )
    {
        return boost::make_iterator_range(
            PySeqSTLIterator( pyo, 0 ),
            PySeqSTLIterator( pyo, PySequence_Length( pyo ) ) );
    }

    static void addToRegistry()
    { 
        py::converter::registry::insert( &convertible, &construct,
                                          py::type_id< Polymorph >() );
    }

    static const Polymorph convert( PyObject* aPyObjectPtr )
    {
        if( PyFloat_Check( aPyObjectPtr ) )
        {
            return PyFloat_AS_DOUBLE( aPyObjectPtr );
        }
        else if( PyInt_Check( aPyObjectPtr ) )
        {
            return PyInt_AS_LONG( aPyObjectPtr );
        }
        else if( PyString_Check( aPyObjectPtr ) )
        {
            return Polymorph( PyString_AsString( aPyObjectPtr ) );
        }
        else if( PyUnicode_Check( aPyObjectPtr ) )
        {
            aPyObjectPtr = PyUnicode_AsEncodedString( aPyObjectPtr, NULL, NULL );
            if ( aPyObjectPtr )
                return Polymorph( PyString_AsString( aPyObjectPtr ) );
        }
        else if ( PySequence_Check( aPyObjectPtr ) )
        {
            return Polymorph( PolymorphValue::create( pyseq_range( aPyObjectPtr ) ) );
        }            
        // conversion is failed. ( convert with repr() ? )
        PyErr_SetString( PyExc_TypeError, 
                                         "Unacceptable type of an object in the tuple." );
        py::throw_error_already_set();
        // never get here: the following is for suppressing warnings
        return Polymorph();
    }

    static bool isConvertible( PyObject* aPyObjectPtr )
    {
        return PyFloat_Check( aPyObjectPtr )
                || PyInt_Check( aPyObjectPtr )
                || PyString_Check( aPyObjectPtr )
                || PyUnicode_Check( aPyObjectPtr )
                || PySequence_Check( aPyObjectPtr );
    }

    static void* convertible( PyObject* aPyObject )
    {
        // always passes the test for efficiency.    overload won't work.
        return aPyObject;
    }

    static void construct( PyObject* aPyObjectPtr, 
                           py::converter::rvalue_from_python_stage1_data* data )
    {
        void* storage( reinterpret_cast<
            py::converter::rvalue_from_python_storage<Polymorph>* >(
                data )->storage.bytes );
        new (storage) Polymorph( convert( aPyObjectPtr ) );
        data->convertible = storage;
    }
};

PolymorphRetriever::PySeqSTLIterator::value_type
PolymorphRetriever::PySeqSTLIterator::operator*()
{
    return PolymorphRetriever::convert(
            PySequence_GetItem( theSeq, theIdx ) );
}


struct PropertySlotMapToPythonConverter
{
    typedef PropertyInterfaceBase::PropertySlotMap argument_type;

    static void addToRegistry()
    {
        py::to_python_converter< argument_type, PropertySlotMapToPythonConverter >();
    }

    static PyObject* convert( argument_type const& map )
    {
        PyObject* aPyDict( PyDict_New() );
        for ( argument_type::const_iterator i( map.begin() );
              i != map.end(); ++i )
        {
            PyDict_SetItem( aPyDict, PyString_FromStringAndSize(
                i->first.data(), i->first.size() ),
                py::incref( py::object( PropertyAttributes( *i->second ) ).ptr() ) );
                                            
        }
        return aPyDict;
    }
};

struct StringVectorToPythonConverter
{
    static void addToRegistry()
    {
        py::to_python_converter< StringVector, StringVectorToPythonConverter >();
    }

    static PyObject* convert( StringVector const& aStringVector )
    {
        py::list retval;

        for ( StringVector::const_iterator i( aStringVector.begin() );
                i != aStringVector.end(); ++i )
        {
            retval.append( py::object( *i ) );
        }

        return py::incref( retval.ptr() );
    }
};


template< typename Ttcell_ >
inline void buildPythonTupleFromTuple(PyObject* pyt, const Ttcell_& cell,
        Py_ssize_t idx = 0)
{
    PyTuple_SetItem( pyt, idx,
        py::incref(
            py::object( cell.get_head()).ptr() ) );

    buildPythonTupleFromTuple(pyt, cell.get_tail(), idx + 1);
}

template<>
inline void buildPythonTupleFromTuple<boost::tuples::null_type>(
        PyObject*, const boost::tuples::null_type&, Py_ssize_t) {}

template< typename Ttuple_ >
struct TupleToPythonConverter
{
    typedef Ttuple_ argument_value_type;
    typedef const argument_value_type& argument_type;
    static PyObject* convert(argument_type val)
    {
        PyObject* retval(
            PyTuple_New( boost::tuples::length<Ttuple_>::value ) );
        buildPythonTupleFromTuple(retval, val);
        return retval;
    }
};

template< typename Tfirst_, typename Tsecond_ >
struct TupleToPythonConverter<std::pair<Tfirst_, Tsecond_> >
{
    typedef std::pair<Tfirst_, Tsecond_> argument_value_type;
    typedef const argument_value_type& argument_type;

    static PyObject* convert( argument_type val )
    {
        return py::incref(
                py::make_tuple(
                    val.first, val.second).ptr());
    }
};

template< typename Ttuple_ >
struct TupleToPythonConverter<boost::shared_ptr<Ttuple_> >
{
    typedef Ttuple_ argument_value_type;
    typedef boost::shared_ptr< argument_value_type > argument_type;
    static PyObject* convert( argument_type val )
    {
        return TupleToPythonConverter< argument_value_type >::convert( *val );
    }
};

template< typename Ttuple_ >
void registerTupleConverters()
{
    py::to_python_converter<
        Ttuple_, TupleToPythonConverter<Ttuple_> >();
    py::to_python_converter<
        boost::shared_ptr<Ttuple_>,
        TupleToPythonConverter<boost::shared_ptr<Ttuple_> > >();
}

template< typename T_ >
struct StringKeyedMapToPythonConverter
{
    static void addToRegistry()
    {
        py::to_python_converter< T_, StringKeyedMapToPythonConverter>();
    }

    static PyObject* convert( T_ const& aStringKeyedMap )
    {
        PyObject* aPyDict( PyDict_New() );
        for ( typename T_::const_iterator i( aStringKeyedMap.begin() );
              i != aStringKeyedMap.end(); ++i )
        {
            PyDict_SetItem( aPyDict, PyString_FromStringAndSize(
                i->first.data(), i->first.size() ),
                py::incref( py::object( i->second ).ptr() ) );
                                            
        }
        return aPyDict;
    }
};

struct FullIDToPythonConverter
{
    static void addToRegistry()
    {
        py::to_python_converter< FullID, FullIDToPythonConverter >();
    }

    static PyObject* convert( FullID const& aFullID )
    {
        return py::incref( py::object( aFullID.asString() ).ptr() );
    }
};



// exception translators

static void translateException( const std::exception& anException )
{
    PyErr_SetString( PyExc_RuntimeError, anException.what() );
}

static void translateRangeError( const std::range_error& anException )
{
    PyErr_SetString( PyExc_KeyError, anException.what() );
}

static PyObject* getLibECSVersionInfo()
{
    PyObject* aPyTuple( PyTuple_New( 3 ) );
        
    PyTuple_SetItem( aPyTuple, 0, PyInt_FromLong( getMajorVersion() ) );
    PyTuple_SetItem( aPyTuple, 1, PyInt_FromLong( getMinorVersion() ) );
    PyTuple_SetItem( aPyTuple, 2, PyInt_FromLong( getMicroVersion() ) );
    
    return aPyTuple;
}

static class PyEcsModule
{
public:
    PyEcsModule()
    {
        if (!initialize())
        {
            throw std::runtime_error( "Failed to initialize libecs" );
        }
    }

    ~PyEcsModule()
    {
        finalize();
    }
} theModule;

template< typename Tdp_ >
class DataPointVectorWrapper
{
public:
    typedef Tdp_ element_type;

private:
    struct GetItemFunc
    {
    };

public:
    class Iterator
    {
    protected:
        PyObject_VAR_HEAD
        DataPointVectorWrapper* theDPVW;
        std::size_t theIdx;
        
    public:
        static PyTypeObject __class__;

    public:
        void* operator new( size_t )
        {
            return PyObject_New( Iterator, &__class__ );
        }

        Iterator( DataPointVectorWrapper* dpvw, std::size_t idx )
            : theDPVW( dpvw ), theIdx( idx )
        {
            Py_INCREF( dpvw );
        }

        ~Iterator()
        {
            Py_XDECREF( theDPVW );
        }

    public:
        static PyTypeObject* __class_init__()
        {
            PyType_Ready( &__class__ );
            return &__class__;
        }

        static Iterator* create( DataPointVectorWrapper* dpvw,
                                 std::size_t idx = 0 )
        {
            return new Iterator( dpvw, idx );
        }

        static void __dealloc__( Iterator* self )
        {
            self->~Iterator();
        }

        static PyObject* __next__( Iterator* self )
        {
            DataPointVector const& vec( *self->theDPVW->theVector );
            if ( self->theIdx < vec.getSize() )
            {
                return toPyObject( &getItem( vec, self->theIdx++ ) );
            }
            return NULL;
        }
    };

protected:
    PyObject_VAR_HEAD
    boost::shared_ptr< DataPointVector > theVector;

public:
    static PyTypeObject __class__;
    static PySequenceMethods __seq__;
    static PyGetSetDef __getset__[];
    static const std::size_t theNumOfElemsPerEntry = sizeof( Tdp_ ) / sizeof( double );

private:
    void* operator new( size_t )
    {
        return PyObject_New( DataPointVectorWrapper, &__class__);
    }

    DataPointVectorWrapper( boost::shared_ptr< DataPointVector > const& aVector )
        : theVector( aVector ) {}

    ~DataPointVectorWrapper()
    {
    }

    PyObject* asPyArray()
    {
        PyArray_Descr* descr( PyArray_DescrFromObject(
            reinterpret_cast< PyObject* >( this ), 0 ) );
        BOOST_ASSERT( descr != NULL );

        return PyArray_CheckFromAny(
                reinterpret_cast< PyObject* >( this ),
                descr, 0, 0, 0, NULL );
    }

public:
    static PyTypeObject* __class_init__()
    {
        Iterator::__class_init__();
        PyType_Ready( &__class__ );
        return &__class__;
    }

    static DataPointVectorWrapper* create( boost::shared_ptr< DataPointVector > const& aVector )
    {
        return new DataPointVectorWrapper( aVector ); 
    }

    static void __dealloc__( DataPointVectorWrapper* self )
    {
        self->~DataPointVectorWrapper();
    }

    static PyObject* __repr__( DataPointVectorWrapper* self )
    {
        return PyObject_Repr( self->asPyArray() );
    }

    static PyObject* __str__( DataPointVectorWrapper* self )
    {
        return PyObject_Str( self->asPyArray() );
    }

    static long __hash__( DataPointVectorWrapper* self )
    {
        PyErr_SetString(PyExc_TypeError, "DataPointVectors are unhashable");
        return -1;
    }

    static int __traverse__( DataPointVectorWrapper* self, visitproc visit,
            void *arg)
    {
        DataPointVector const& vec( *self->theVector );
        for ( std::size_t i( 0 ), len( vec.getSize() ); i < len; ++i )
        {
            Py_VISIT( toPyObject( &getItem( *self->theVector, i ) ) );
        }
        return 0;
    }

    static PyObject* __iter__( DataPointVectorWrapper* self )
    {
        Iterator* i( Iterator::create( self ) );
        return reinterpret_cast< PyObject* >( i );
    }

    static Py_ssize_t __len__( DataPointVectorWrapper* self )
    {
        return self->theVector->getSize();
    }

    static PyObject* __getitem__( DataPointVectorWrapper* self, Py_ssize_t idx )
    {
        if ( idx < 0 || idx >= static_cast< Py_ssize_t >( self->theVector->getSize() ) )
        {
            PyErr_SetObject(PyExc_IndexError,
                    PyString_FromString("index out of range"));
		    return NULL;
        }
            
        return toPyObject( &getItem( *self->theVector, idx ) );
    }

    static void __dealloc_array_struct( void* ptr,
                                        DataPointVectorWrapper* self )
    {
        Py_XDECREF( self );
        PyMem_FREE( ptr );
    }

    static PyObject* __get___array__struct( DataPointVectorWrapper* self,
                                            void* closure )
    {
        PyArrayInterface* aif(
            reinterpret_cast< PyArrayInterface* >(
                PyMem_MALLOC( sizeof( PyArrayInterface )
                              + sizeof ( Py_intptr_t ) * 4 ) ) );
        if ( !aif )
        {
            return NULL;
        }
        aif->two = 2;
        aif->nd = 2;
        aif->typekind = 'f';
        aif->itemsize = sizeof( double );
        aif->flags = NPY_CONTIGUOUS | NPY_ALIGNED | NPY_NOTSWAPPED;
        aif->shape = reinterpret_cast< Py_intptr_t* >( aif + 1 );
        aif->shape[ 0 ] = self->theVector->getSize();
        aif->shape[ 1 ] = theNumOfElemsPerEntry;
        aif->strides = reinterpret_cast< Py_intptr_t* >( aif + 1 ) + 2;
        aif->strides[ 0 ] = sizeof( double ) * aif->shape[ 1 ];
        aif->strides[ 1 ] = sizeof( double );
        aif->data = const_cast< void* >( self->theVector->getRawArray() );
        aif->descr = NULL;

        Py_INCREF( self );
        return PyCObject_FromVoidPtrAndDesc( aif, self,
                reinterpret_cast< void(*)(void*, void*) >(
                    __dealloc_array_struct ) );
    }

    static int __contains__( DataPointVectorWrapper* self, PyObject *e )
    {
        if ( !PyArray_Check( e ) || PyArray_NDIM( e ) != 1
                || PyArray_DIMS( e )[ 0 ] < static_cast< Py_ssize_t >( theNumOfElemsPerEntry ) )
        {
            return 1;
        }


        DataPoint const& dp( *reinterpret_cast< DataPoint* >( PyArray_DATA( e ) ) );
        DataPoint const* begin( reinterpret_cast< DataPoint const* >(
                          self->theVector->getRawArray() ) );
        DataPoint const* end( begin + self->theVector->getSize() );
        return end == std::find( begin, end, dp );
    }

    static PyObject* toPyObject( DataPoint const* dp )
    {
        static const npy_intp dims[] = { theNumOfElemsPerEntry };
        PyArrayObject* arr( reinterpret_cast< PyArrayObject* >(
            PyArray_NewFromDescr( &PyArray_Type,
                PyArray_DescrFromType( NPY_DOUBLE ),
                1, const_cast< npy_intp* >( dims ), NULL, 0, NPY_CONTIGUOUS, NULL )
            ) );
        std::memcpy( PyArray_DATA( arr ), const_cast< DataPoint* >( dp ), sizeof( double ) * theNumOfElemsPerEntry );
        return reinterpret_cast< PyObject * >( arr );
    }

    static Tdp_ const& getItem( DataPointVector const& vec, std::size_t idx )
    {
        return GetItemFunc()( vec, idx );
    }
};

template<>
struct DataPointVectorWrapper< DataPoint >::GetItemFunc
{
    DataPoint const& operator()( DataPointVector const& vec, std::size_t idx ) const
    {
        return vec.asShort( idx );
    }
};

template<>
struct DataPointVectorWrapper< LongDataPoint >::GetItemFunc
{
    LongDataPoint const& operator()( DataPointVector const& vec, std::size_t idx ) const
    {
        return vec.asLong( idx );
    }
};


template< typename Tdp_ >
PyTypeObject DataPointVectorWrapper< Tdp_ >::Iterator::__class__ = {
	PyObject_HEAD_INIT( &PyType_Type )
	0,					/* ob_size */
	"ecell._ecs.DataPointVectorWrapper.Iterator", /* tp_name */
	sizeof( typename DataPointVectorWrapper::Iterator ), /* tp_basicsize */
	0,					/* tp_itemsize */
	/* methods */
	(destructor)&DataPointVectorWrapper::Iterator::__dealloc__, /* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_HAVE_CLASS | Py_TPFLAGS_HAVE_WEAKREFS | Py_TPFLAGS_HAVE_ITER,/* tp_flags */
	0,					/* tp_doc */
	0,	/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	PyObject_SelfIter,  /* tp_iter */
	(iternextfunc)&DataPointVectorWrapper::Iterator::__next__,		/* tp_iternext */
	0,		        	/* tp_methods */
	0					/* tp_members */
};

template< typename Tdp_ >
PyTypeObject DataPointVectorWrapper< Tdp_ >::__class__ = {
	PyObject_HEAD_INIT( &PyType_Type )
	0,
	"ecell._ecs.DataPointVector",
	sizeof(DataPointVectorWrapper),
	0,
	(destructor)&DataPointVectorWrapper::__dealloc__, /* tp_dealloc */
	0,      			/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	(reprfunc)&DataPointVectorWrapper::__repr__,			/* tp_repr */
	0,					/* tp_as_number */
	&DataPointVectorWrapper::__seq__,			/* tp_as_sequence */
	0,			/* tp_as_mapping */
	(hashfunc)&DataPointVectorWrapper::__hash__,				/* tp_hash */
	0,					/* tp_call */
	(reprfunc)&DataPointVectorWrapper::__str__,				/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_HAVE_CLASS | Py_TPFLAGS_HAVE_WEAKREFS | Py_TPFLAGS_HAVE_SEQUENCE_IN,		/* tp_flags */
 	0,				/* tp_doc */
 	(traverseproc)&DataPointVectorWrapper::__traverse__,		/* tp_traverse */
 	0,			/* tp_clear */
	0,			/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	(getiterfunc)&DataPointVectorWrapper::__iter__,				/* tp_iter */
	0,					/* tp_iternext */
	0,				/* tp_methods */
	0,					/* tp_members */
	DataPointVectorWrapper::__getset__,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,			/* tp_init */
	PyType_GenericAlloc,			/* tp_alloc */
	PyType_GenericNew,			/* tp_new */
	PyObject_Free,			/* tp_free */
};

template< typename Tdp_ >
PySequenceMethods DataPointVectorWrapper< Tdp_ >::__seq__ = {
	(lenfunc)&DataPointVectorWrapper::__len__,			/* sq_length */
	(binaryfunc)0,		/* sq_concat */
	(ssizeargfunc)0,		/* sq_repeat */
	(ssizeargfunc)&DataPointVectorWrapper::__getitem__,		/* sq_item */
	(ssizessizeargfunc)0,		/* sq_slice */
	(ssizeobjargproc)0,		/* sq_ass_item */
	(ssizessizeobjargproc)0,	/* sq_ass_slice */
	(objobjproc)&DataPointVectorWrapper::__contains__,		/* sq_contains */
	(binaryfunc)0,	/* sq_inplace_concat */
	(ssizeargfunc)0	/* sq_inplace_repeat */
};

template< typename Tdp_ >
PyGetSetDef DataPointVectorWrapper< Tdp_ >::__getset__[] = {
    { const_cast< char* >( "__array_struct__" ), (getter)&DataPointVectorWrapper::__get___array__struct, NULL },
    { NULL }
};


template< typename Titer_ >
class STLIteratorWrapper
{
protected:
    PyObject_VAR_HEAD
    Titer_ theIdx;
    Titer_ theEnd; 

public:
    static PyTypeObject __class__;

public:
    void* operator new( size_t )
    {
        return PyObject_New( STLIteratorWrapper, &__class__ );
    }

    template< typename Trange_ >
    STLIteratorWrapper( Trange_ const& range )
        : theIdx( boost::begin( range ) ), theEnd( boost::end( range ) )
    {
    }

    ~STLIteratorWrapper()
    {
    }

public:
    static PyTypeObject* __class_init__()
    {
        PyType_Ready( &__class__ );
        return &__class__;
    }

    template< typename Trange_ >
    static STLIteratorWrapper* create( Trange_ const& range )
    {
        return new STLIteratorWrapper( range );
    }

    static void __dealloc__( STLIteratorWrapper* self )
    {
        self->~STLIteratorWrapper();
    }

    static PyObject* __next__( STLIteratorWrapper* self )
    {
        if ( self->theIdx == self->theEnd )
            return NULL;

        return py::incref( py::object( *self->theIdx ).ptr() );
    }
};

template< typename Titer_ >
PyTypeObject STLIteratorWrapper< Titer_ >::__class__ = {
	PyObject_HEAD_INIT( &PyType_Type )
	0,					/* ob_size */
	"ecell._ecs.STLIteratorWrapper", /* tp_name */
	sizeof( STLIteratorWrapper ), /* tp_basicsize */
	0,					/* tp_itemsize */
	/* methods */
	(destructor)&STLIteratorWrapper::__dealloc__, /* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0,					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_HAVE_CLASS | Py_TPFLAGS_HAVE_WEAKREFS | Py_TPFLAGS_HAVE_ITER,/* tp_flags */
	0,					/* tp_doc */
	0,	/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	PyObject_SelfIter,  /* tp_iter */
	(iternextfunc)&STLIteratorWrapper::__next__,		/* tp_iternext */
	0,		        	/* tp_methods */
	0					/* tp_members */
};


static std::string VariableReference___repr__( VariableReference const* self )
{
    return std::string( "[" ) + self->getName() + ": "
            + "coefficient=" + stringCast( self->getCoefficient() ) + ", "
            + "variable=" + self->getVariable()->getFullID().asString() + ", "
            + "accessor=" + ( self->isAccessor() ? "true": "false" ) + "]";
}


class VariableReferences
{
public:
    VariableReferences( Process* proc ): theProc( proc ) {}

    void add( String const& name, String const& fullID, Integer const& coef,
              bool isAccessor )
    {
        theProc->registerVariableReference( name, FullID( fullID ),
                                            coef, isAccessor );
    }

    void add( String const& name, String const& fullID, Integer const& coef )
    {
        theProc->registerVariableReference( name, FullID( fullID ),
                                            coef, false );
    }


    void remove( String const& name )
    {
        theProc->removeVariableReference( name );
    }

    VariableReference const& __getitem__( py::object name )
    {
        if ( PyInt_Check( name.ptr() ) )
        {
            long idx( PyInt_AS_LONG( name.ptr() ) );
            VariableReferenceVector const& refs(
                    theProc->getVariableReferenceVector() );
            if ( idx < 0
                 || static_cast< VariableReferenceVector::size_type >( idx )
                    >= refs.size() )
            {
                throw std::range_error( "Index out of bounds");
            }
            return refs[ idx ];
        }
        else if ( PyString_Check( name.ptr() ) )
        {
            std::string nameStr( PyString_AS_STRING( name.ptr() ),
                                 PyString_GET_SIZE( name.ptr() ) );
            return theProc->getVariableReference( nameStr );
        }
        PyErr_SetString( PyExc_TypeError,
                         "The argument is neither an integer nor a string" );
        py::throw_error_already_set();
        throw std::exception();
    }

    Py_ssize_t __len__()
    {
        return theProc->getVariableReferenceVector().size();
    }

    py::list getPositivesReferences()
    {
        VariableReferenceVector const& refs(
                theProc->getVariableReferenceVector() );

        py::list retval;
        std::for_each(
            refs.begin() + theProc->getPositiveVariableReferenceOffset(),
            refs.end(), boost::bind(
                &py::list::append< VariableReference >, &retval,
                _1 ) );
        return retval;
    }

    py::list getNegativeReferences()
    {
        VariableReferenceVector const& refs(
                theProc->getVariableReferenceVector() );

        py::list retval;
        std::for_each(
            refs.begin(),
            refs.begin() + theProc->getZeroVariableReferenceOffset(),
            boost::bind( &py::list::append< VariableReference >, &retval,
                         _1 ) );
        return retval;
    }

    py::list getZeroReferences()
    {
        VariableReferenceVector const& refs(
                theProc->getVariableReferenceVector() );

        py::list retval;
        std::for_each(
            refs.begin() + theProc->getZeroVariableReferenceOffset(),
            refs.begin() + theProc->getPositiveVariableReferenceOffset(),
            boost::bind( &py::list::append< VariableReference >, &retval,
                         _1 ) );
        return retval;
    }

    py::object __iter__()
    {
        return py::object( STLIteratorWrapper< VariableReferenceVector::const_iterator >( theProc->getVariableReferenceVector() ) );
    }

    std::string __repr__()
    {
        VariableReferenceVector const& refs(
                theProc->getVariableReferenceVector() );

        std::string retval;

        retval += '[';
        for ( VariableReferenceVector::const_iterator b( refs.begin() ),
                                                      i( b ),
                                                      e( refs.end() );
                i != e; ++i )
        {
            if ( i != b )
                retval += ", ";
            retval += VariableReference___repr__( &*i );
        }
        retval += ']';

        return retval;
    } 

private:
    Process* theProc;
};

class DataPointVectorSharedPtrConverter
{
public:
    static void addToRegistry()
    {
        py::to_python_converter< boost::shared_ptr< DataPointVector >,
                                 DataPointVectorSharedPtrConverter >();
    }

    static PyObject* 
    convert( boost::shared_ptr< DataPointVector > const& aVectorSharedPtr )
    {
        return aVectorSharedPtr->getElementSize() == sizeof( DataPoint ) ?
                reinterpret_cast< PyObject* >(
                    DataPointVectorWrapper< DataPoint >::create(
                        aVectorSharedPtr ) ):
                reinterpret_cast< PyObject* >(
                    DataPointVectorWrapper< LongDataPoint >::create(
                        aVectorSharedPtr ) );
    }
};

class PythonWarningHandler: public libecs::WarningHandler
{
public:
    PythonWarningHandler() {}

    PythonWarningHandler( py::handle<> aCallable )
        : thePyObject( aCallable )
    {
    }
      
    virtual ~PythonWarningHandler() {}

    virtual void operator()( String const& msg ) const
    {
        if ( thePyObject )
          PyObject_CallFunctionObjArgs( thePyObject.get(), py::object( msg ).ptr(), NULL );
    }

private:
    py::handle<> thePyObject;
};

template< typename T >
class PythonDynamicModule;

struct PythonEntityBaseBase
{
protected:
    static void appendDictToSet( std::set< String >& retval, std::string const& aPrivPrefix, PyObject* aObject )
    {
        py::handle<> aSelfDict( py::allow_null( PyObject_GetAttrString( aObject, const_cast< char* >( "__dict__" ) ) ) );
        if ( !aSelfDict )
        {
            PyErr_Clear();
            return;
        }

        if ( !PyMapping_Check( aSelfDict.get() ) )
        {
            return;
        }

        py::handle<> aKeyList( PyMapping_Items( aSelfDict.get() ) );
        BOOST_ASSERT( PyList_Check( aKeyList.get() ) );
        for ( py::ssize_t i( 0 ), e( PyList_GET_SIZE( aKeyList.get() ) ); i < e; ++i )
        {
            py::handle<> aKeyValuePair( py::borrowed( PyList_GET_ITEM( aKeyList.get(), i ) ) );
            BOOST_ASSERT( PyTuple_Check( aKeyValuePair.get() ) && PyTuple_GET_SIZE( aKeyValuePair.get() ) == 2 );
            py::handle<> aKey( py::borrowed( PyTuple_GET_ITEM( aKeyValuePair.get(), 0 ) ) );
            BOOST_ASSERT( PyString_Check( aKey.get() ) );
            if ( PyString_GET_SIZE( aKey.get() ) >= static_cast< py::ssize_t >( aPrivPrefix.size() )
                    && memcmp( PyString_AS_STRING( aKey.get() ), aPrivPrefix.data(), aPrivPrefix.size() ) == 0 )
            {
                continue;
            }

            if ( PyString_GET_SIZE( aKey.get() ) >= 2
                    && memcmp( PyString_AS_STRING( aKey.get() ), "__", 2 ) == 0 )
            {
                continue;
            }

            py::handle<> aValue( py::borrowed( PyTuple_GET_ITEM( aKeyValuePair.get(), 1 ) ) );
            if ( !PolymorphRetriever::isConvertible( aValue.get() ) )
            {
                continue;
            }

            retval.insert( String( PyString_AS_STRING( aKey.get() ), PyString_GET_SIZE( aKey.get() ) ) );
        }
    }

    static void addAttributesFromBases( std::set< String >& retval, std::string const& aPrivPrefix, PyObject* anUpperBound, PyObject* tp )
    {
        BOOST_ASSERT( PyType_Check( tp ) );

        if ( anUpperBound == tp )
        {
            return;
        }

        py::handle<> aBasesList( py::allow_null( PyObject_GetAttrString( tp, const_cast< char* >( "__bases__" ) ) ) );
        if ( !aBasesList )
        {
            PyErr_Clear();
            return;
        }

        if ( !PyTuple_Check( aBasesList.get() ) )
        {
            return;
        }

        for ( py::ssize_t i( 0 ), ie( PyTuple_GET_SIZE( aBasesList.get() ) ); i < ie; ++i )
        {
            py::handle<> aBase( py::borrowed( PyTuple_GET_ITEM( aBasesList.get(), i ) ) );
            appendDictToSet( retval, aPrivPrefix, aBase.get() );
            addAttributesFromBases( retval, aPrivPrefix, anUpperBound, aBase.get() );
        }
    }

    static void removeAttributesFromBases( std::set< String >& retval, PyObject *tp )
    {
        BOOST_ASSERT( PyType_Check( tp ) );

        py::handle<> aBasesList( py::allow_null( PyObject_GetAttrString( tp, const_cast< char* >( "__bases__" ) ) ) );
        if ( !aBasesList )
        {
            PyErr_Clear();
            return;
        }

        if ( !PyTuple_Check( aBasesList.get() ) )
        {
            return;
        }

        for ( py::ssize_t i( 0 ), ie( PyTuple_GET_SIZE( aBasesList.get() ) ); i < ie; ++i )
        {
            py::handle<> aBase( py::borrowed( PyTuple_GET_ITEM( aBasesList.get(), i ) ) );
            removeAttributesFromBases( retval, aBase.get() );

            py::handle<> aBaseDict( py::allow_null( PyObject_GetAttrString( aBase.get(), const_cast< char* >( "__dict__" ) ) ) );
            if ( !aBaseDict )
            {
                PyErr_Clear();
                return;
            }

            if ( !PyMapping_Check( aBaseDict.get() ) )
            {
                return;
            }

            py::handle<> aKeyList( PyMapping_Keys( aBaseDict.get() ) );
            BOOST_ASSERT( PyList_Check( aKeyList.get() ) );
            for ( py::ssize_t j( 0 ), je( PyList_GET_SIZE( aKeyList.get() ) ); j < je; ++j )
            {
                py::handle<> aKey( py::borrowed( PyList_GET_ITEM( aKeyList.get(), i ) ) );
                BOOST_ASSERT( PyString_Check( aKey.get() ) );
                String aKeyStr( PyString_AS_STRING( aKey.get() ), PyString_GET_SIZE( aKey.get() ) );  
                retval.erase( aKeyStr );
            }
        }
    }

    PythonEntityBaseBase() {}
};

template< typename Tderived_, typename Tbase_ >
class PythonEntityBase: public Tbase_, public PythonEntityBaseBase, public py::wrapper< Tbase_ >
{
public:
    virtual ~PythonEntityBase()
    {
        py::decref( py::detail::wrapper_base_::owner( this ) );
    }

    PythonDynamicModule< Tderived_ > const& getModule() const
    {
        return theModule;
    }

    const Polymorph defaultGetProperty( String const& aPropertyName ) const
    {
        PyObject* aSelf( py::detail::wrapper_base_::owner( this ) );
        String lowerCasedPropertyName( aPropertyName );
        lowerCasedPropertyName[ 0 ] = std::tolower( lowerCasedPropertyName[ 0 ] );
        py::handle<> aValue( py::allow_null( PyObject_GenericGetAttr( aSelf, py::handle<>( PyString_InternFromString( const_cast< char* >( lowerCasedPropertyName.c_str() ) ) ).get() ) ) );
        if ( !aValue )
        {
            PyErr_Clear();
            THROW_EXCEPTION_INSIDE( NoSlot, 
                    "failed to retrieve property attributes "
                    "for [" + aPropertyName + "]" );
        }

        return py::extract< Polymorph >( aValue.get() );
    }

    const PropertyAttributes defaultGetPropertyAttributes( String const& aPropertyName ) const
    {
        return PropertyAttributes( PropertySlotBase::POLYMORPH, true, true, true, true, true );
    }


    void defaultSetProperty( String const& aPropertyName, Polymorph const& aValue )
    {
        PyObject* aSelf( py::detail::wrapper_base_::owner( this ) );
        PyObject_GenericSetAttr( aSelf, py::handle<>( PyString_InternFromString( const_cast< char* >( aPropertyName.c_str() ) ) ).get(), py::object( aValue ).ptr() );
        if ( PyErr_Occurred() )
        {
            PyErr_Clear();
            THROW_EXCEPTION_INSIDE( NoSlot, 
                            "failed to set property [" + aPropertyName + "]" );
        }
    }

    const StringVector defaultGetPropertyList() const
    {
        PyObject* aSelf( py::detail::wrapper_base_::owner( this ) );
        std::set< String > aPropertySet;

        if ( thePrivPrefix.empty() )
        {
            PyObject* anOwner( py::detail::wrapper_base_::owner( this ) );
            BOOST_ASSERT( anOwner != NULL );
            thePrivPrefix = String( "_" ) + anOwner->ob_type->tp_name;
        }

        appendDictToSet( aPropertySet, thePrivPrefix, aSelf );

        PyObject* anUpperBound(
                reinterpret_cast< PyObject* >(
                    py::objects::registered_class_object(
                        typeid( Tbase_ ) ).get() ) );
        addAttributesFromBases( aPropertySet, thePrivPrefix, anUpperBound,
                reinterpret_cast< PyObject* >( aSelf->ob_type ) );
        removeAttributesFromBases( aPropertySet, anUpperBound );

        StringVector retval;
        for ( std::set< String >::iterator i( aPropertySet.begin() ), e( aPropertySet.end() ); i != e; ++i )
        {
            retval.push_back( *i );
            retval.back()[ 0 ] = std::toupper( retval.back()[ 0 ] );
        }

        return retval;
    }

    PropertyInterface< Tbase_ > const& _getPropertyInterface() const;

    PropertySlotBase const* getPropertySlot( String const& aPropertyName ) const
    {
        return _getPropertyInterface().getPropertySlot( aPropertyName ); \
    }

    virtual void setProperty( String const& aPropertyName, Polymorph const& aValue )
    {
        return _getPropertyInterface().setProperty( *this, aPropertyName, aValue );
    }

    const Polymorph getProperty( String const& aPropertyName ) const
    {
        return _getPropertyInterface().getProperty( *this, aPropertyName );
    }

    void loadProperty( String const& aPropertyName, Polymorph const& aValue )
    {
        return _getPropertyInterface().loadProperty( *this, aPropertyName, aValue );
    }

    const Polymorph saveProperty( String const& aPropertyName ) const
    {
        return _getPropertyInterface().saveProperty( *this, aPropertyName );
    }

    const StringVector getPropertyList() const
    {
        return _getPropertyInterface().getPropertyList( *this );
    }

    PropertySlotProxy* createPropertySlotProxy( String const& aPropertyName )
    {
        return _getPropertyInterface().createPropertySlotProxy( *this, aPropertyName );
    }

    const PropertyAttributes
    getPropertyAttributes( String const& aPropertyName ) const
    {
        return _getPropertyInterface().getPropertyAttributes( *this, aPropertyName );
    }

    virtual PropertyInterfaceBase const& getPropertyInterface() const
    {
        return _getPropertyInterface();
    }

    PythonEntityBase( PythonDynamicModule< Tderived_ > const& aModule )
        : theModule( aModule ) {}

    static void addToRegistry()
    {
        py::objects::register_dynamic_id< Tderived_ >();
        py::objects::register_dynamic_id< Tbase_ >();
        py::objects::register_conversion< Tderived_, Tbase_ >( false );
        py::objects::register_conversion< Tbase_, Tderived_ >( true );
    }

protected:

    PythonDynamicModule< Tderived_ > const& theModule;
    mutable String thePrivPrefix;
};

class PythonProcess: public PythonEntityBase< PythonProcess, Process >
{
public:
    virtual ~PythonProcess() {}

    LIBECS_DM_INIT_PROP_INTERFACE()
    {
        INHERIT_PROPERTIES( Process );
    }

    virtual void initialize()
    {
        PyObject* aSelf( py::detail::wrapper_base_::owner( this ) );
        generic_getattr( py::object( py::borrowed( aSelf ) ), "initialize" )();
        theFireMethod = generic_getattr( py::object( py::borrowed( aSelf ) ), "fire" );
    }

    virtual void fire()
    {
        py::object retval( theFireMethod() );
        if ( retval )
        {
            setActivity( py::extract< Real >( retval ) );
        } 
    }

    virtual const bool isContinuous() const
    {
        PyObject* aSelf( py::detail::wrapper_base_::owner( this ) );
        py::handle<> anIsContinuousDescr( py::allow_null( PyObject_GenericGetAttr( reinterpret_cast< PyObject* >( aSelf->ob_type ), py::handle<>( PyString_InternFromString( "isContinuous" ) ).get() ) ) );
        if ( !anIsContinuousDescr )
        {
            PyErr_Clear();
            return Process::isContinuous();
        }

        descrgetfunc aDescrGetFunc( anIsContinuousDescr.get()->ob_type->tp_descr_get );
        if ( ( anIsContinuousDescr.get()->ob_type->tp_flags & Py_TPFLAGS_HAVE_CLASS ) && aDescrGetFunc )
        {
            return py::extract< bool >( py::handle<>( aDescrGetFunc( anIsContinuousDescr.get(), aSelf, reinterpret_cast< PyObject* >( aSelf->ob_type ) ) ).get() );
        }

        return py::extract< bool >( anIsContinuousDescr.get() );
    }

    PythonProcess( PythonDynamicModule< PythonProcess > const& aModule )
        : PythonEntityBase< PythonProcess, Process >( aModule ) {}

    py::object theFireMethod;
};


class PythonVariable: public PythonEntityBase< PythonVariable, Variable >
{
public:
    LIBECS_DM_INIT_PROP_INTERFACE()
    {
        INHERIT_PROPERTIES( Variable );
    }

    virtual void initialize()
    {
        Variable::initialize();
        PyObject* aSelf( py::detail::wrapper_base_::owner( this ) );
        py::getattr( py::object( py::borrowed( aSelf ) ), "initialize" )();
        theOnValueChangingMethod = py::handle<>( py::allow_null( PyObject_GenericGetAttr( aSelf, py::handle<>( PyString_InternFromString( const_cast< char* >( "onValueChanging" ) ) ).get() ) ) );
        if ( !theOnValueChangingMethod )
        {
            PyErr_Clear();
        }
    }

    virtual SET_METHOD( Real, Value )
    {
        if ( theOnValueChangingMethod )
        {
            if ( !PyCallable_Check( theOnValueChangingMethod.get() ) )
            {
                PyErr_SetString( PyExc_TypeError, "object is not callable" );
                py::throw_error_already_set();
            }

            py::handle<> aResult( PyObject_CallFunction( theOnValueChangingMethod.get(), "f", value ) );
            if ( !aResult )
            {
                py::throw_error_already_set();
            }
            else
            {
                if ( !PyObject_IsTrue( aResult.get() ) )
                {
                    return;
                }
            }
        }
        else
        {
            PyErr_Clear();
        }
        Variable::setValue( value );
    }

    PythonVariable( PythonDynamicModule< PythonVariable > const& aModule )
        : PythonEntityBase< PythonVariable, Variable >( aModule ) {}

    py::handle<> theOnValueChangingMethod;
};


class PythonSystem: public PythonEntityBase< PythonSystem, System >
{
public:
    LIBECS_DM_INIT_PROP_INTERFACE()
    {
        INHERIT_PROPERTIES( System );
    }

    virtual void initialize()
    {
        System::initialize();
        PyObject* aSelf( py::detail::wrapper_base_::owner( this ) );
        py::getattr( py::object( py::borrowed( aSelf ) ), "initialize" )();
    }

    PythonSystem( PythonDynamicModule< PythonSystem > const& aModule )
        : PythonEntityBase< PythonSystem, System >( aModule ) {}
};

template<typename T_>
struct DeduceEntityType
{
    static EntityType value;
};

template<>
EntityType DeduceEntityType< PythonProcess >::value( EntityType::PROCESS );

template<>
EntityType DeduceEntityType< PythonVariable >::value( EntityType::VARIABLE );

template<>
EntityType DeduceEntityType< PythonSystem >::value( EntityType::SYSTEM );

template< typename T_ >
struct PythonDynamicModule: public DynamicModule< EcsObject >
{
    typedef DynamicModule< EcsObject > Base;

    struct make_ptr_instance: public py::objects::make_instance_impl< T_, py::objects::pointer_holder< T_*, T_ >, make_ptr_instance > 
    {
        typedef py::objects::pointer_holder< T_*, T_ > holder_t;

        template <class Arg>
        static inline holder_t* construct(void* storage, PyObject* arg, Arg& x)
        {
            py::detail::initialize_wrapper( py::incref( arg ), boost::get_pointer(x) );
            return new (storage) holder_t(x);
        }

        template<typename Ptr>
        static inline PyTypeObject* get_class_object(Ptr const& x)
        {
            return static_cast< T_ const* >( boost::get_pointer(x) )->getModule().getPythonType();
        }
    };

    virtual EcsObject* createInstance() const;

    virtual const char* getFileName() const
    {
        const char* aRetval( 0 );
        try
        {
            py::handle<> aPythonModule( PyImport_Import( py::getattr( thePythonClass, "__module__" ).ptr() ) );
            if ( !aPythonModule )
            {
                py::throw_error_already_set();
            }
            aRetval = py::extract< const char * >( py::getattr( py::object( aPythonModule ), "__file__" ) );
        }
        catch ( py::error_already_set )
        {
            PyErr_Clear();
        }
        return aRetval;
    }

    virtual const char *getModuleName() const 
    {
        return reinterpret_cast< PyTypeObject* >( thePythonClass.ptr() )->tp_name;
    }

    virtual const DynamicModuleInfo* getInfo() const
    {
        return &thePropertyInterface;
    }

    PyTypeObject* getPythonType() const
    {
        return reinterpret_cast< PyTypeObject* >( thePythonClass.ptr() );
    }
 
    PythonDynamicModule( py::object aPythonClass )
        : Base( DM_TYPE_DYNAMIC ),
           thePythonClass( aPythonClass ),
           thePropertyInterface( getModuleName(),
                                 DeduceEntityType< T_ >::value.asString() )
    {
    }

private:
    py::object thePythonClass;
    PropertyInterface< T_ > thePropertyInterface;
};

template< typename Tderived_, typename Tbase_ >
inline PropertyInterface< Tbase_ > const&
PythonEntityBase< Tderived_, Tbase_ >::_getPropertyInterface() const
{
    return *reinterpret_cast< PropertyInterface< Tbase_ > const* >( theModule.getInfo() );
}

template< typename T_ >
EcsObject* PythonDynamicModule< T_ >::createInstance() const
{
    T_* retval( new T_( *this ) );
    py::handle<> aNewObject( make_ptr_instance::execute( retval ) );

    if ( !aNewObject )
    {
        delete retval;
        std::string anErrorStr( "Instantiation failure" );
        PyObject* aPyErrObj( PyErr_Occurred() );
        if ( aPyErrObj )
        {
            anErrorStr += "(";
            anErrorStr += aPyErrObj->ob_type->tp_name;
            anErrorStr += ": ";
            py::handle<> aPyErrStrRepr( PyObject_Str( aPyErrObj ) );
            BOOST_ASSERT( PyString_Check( aPyErrStrRepr.get() ) );
            anErrorStr.insert( anErrorStr.size(),
                PyString_AS_STRING( aPyErrStrRepr.get() ),
                PyString_GET_SIZE( aPyErrStrRepr.get() ) );
            anErrorStr += ")";
            PyErr_Clear();
        }
        throw std::runtime_error( anErrorStr );
    }

    return retval;
}

template< typename T_ >
inline PyObject* to_python_indirect_fun( T_* arg )
{
    return py::to_python_indirect< T_, py::detail::make_reference_holder >()( arg );
}

class Simulator
{
public:
    struct Model: public libecs::Model
    {
    public:
        Simulator& getSimulator() const
        {
            return theSimulator;
        }

        Model( Simulator& aSimulator, ModuleMaker< EcsObject >& aModuleMaker )
            : libecs::Model( aModuleMaker ), theSimulator( aSimulator ) {}

    protected:
        Simulator& theSimulator;
    };

private:
    struct CompositeModuleMaker: public ModuleMaker< EcsObject >,
                                 public SharedModuleMakerInterface
    {
        virtual ~CompositeModuleMaker() {}

        virtual void setSearchPath( const std::string& path )
        {
            SharedModuleMakerInterface* anInterface(
                dynamic_cast< SharedModuleMakerInterface* >( &theDefaultModuleMaker ) );
            if ( anInterface )
            {
                anInterface->setSearchPath( path );
            }
        }

        virtual const std::string getSearchPath() const
        {
            SharedModuleMakerInterface* anInterface(
                dynamic_cast< SharedModuleMakerInterface* >( &theDefaultModuleMaker ) );
            if ( anInterface )
            {
                return anInterface->getSearchPath();
            }
        }

        virtual const Module& getModule( const std::string& aClassName, bool forceReload = false )
        {
            ModuleMap::iterator i( theRealModuleMap.find( aClassName ) );
            if ( i == theRealModuleMap.end() )
            {
                const Module& retval( theDefaultModuleMaker.getModule( aClassName, forceReload ) );
                theRealModuleMap[ retval.getModuleName() ] = const_cast< Module* >( &retval );
                return retval;
            }
            return *(*i).second;
        }

        virtual void addClass( Module* dm )
        {
            assert( dm != NULL && dm->getModuleName() != NULL );
            this->theRealModuleMap[ dm->getModuleName() ] = dm;
            ModuleMaker< EcsObject >::addClass( dm );
        }

        virtual const ModuleMap& getModuleMap() const
        {
            return theRealModuleMap;
        }

        CompositeModuleMaker( ModuleMaker< EcsObject >& aDefaultModuleMaker )
            : theDefaultModuleMaker( aDefaultModuleMaker ) {}

    private:
        ModuleMaker< EcsObject >& theDefaultModuleMaker;
        ModuleMap theRealModuleMap;
    };

    struct DMTypeResolverHelper
    {
        EntityType operator()( py::object aClass )
        {
            if ( !PyType_Check( aClass.ptr() ) )
            {
                return EntityType( EntityType::NONE );
            }

            if (  thePyTypeProcess.get() == reinterpret_cast< PyTypeObject* >( aClass.ptr() ) )
            {
                return EntityType( EntityType::PROCESS );
            }
            else if ( thePyTypeVariable.get() == reinterpret_cast< PyTypeObject* >( aClass.ptr() ) )
            {
                return EntityType( EntityType::VARIABLE );
            }
            else if ( thePyTypeSystem.get() == reinterpret_cast< PyTypeObject* >( aClass.ptr() ) )
            {
                return EntityType( EntityType::SYSTEM );
            }

            py::handle<> aBasesList( PyObject_GetAttrString( aClass.ptr(), const_cast< char* >( "__bases__" ) ) );
            if ( !aBasesList )
            {
                PyErr_Clear();
                return EntityType( EntityType::NONE );
            }

            if ( !PyTuple_Check( aBasesList.get() ) )
            {
                return EntityType( EntityType::NONE );
            }


            for ( py::ssize_t i( 0 ), ie( PyTuple_GET_SIZE( aBasesList.get() ) ); i < ie; ++i )
            {
                py::object aBase( py::borrowed( PyTuple_GET_ITEM( aBasesList.get(), i ) ) );
                EntityType aResult( ( *this )( aBase ) );
                if ( aResult.getType() != EntityType::NONE )
                {
                    return aResult;
                }
            }

            return EntityType( EntityType::NONE );
        }

        DMTypeResolverHelper()
            : thePyTypeProcess( py::objects::registered_class_object( typeid( Process ) ) ),
              thePyTypeVariable( py::objects::registered_class_object( typeid( Variable ) ) ),
              thePyTypeSystem( py::objects::registered_class_object( typeid( System ) ) )
        {
        }

        py::type_handle thePyTypeProcess;
        py::type_handle thePyTypeVariable;
        py::type_handle thePyTypeSystem;
    };            

public:
    Simulator()
        : theRunningFlag( false ),
          theDirtyFlag( true ),
          theEventCheckInterval( 20 ),
          theEventHandler(),
          theDefaultPropertiedObjectMaker( createDefaultModuleMaker() ),
          thePropertiedObjectMaker( *theDefaultPropertiedObjectMaker ),
          theModel( *this, thePropertiedObjectMaker )
    {
    }

    ~Simulator()
    {
    }

    void initialize()
    {
        theModel.initialize();
    }

    Stepper* createStepper( String const& aClassname,
                            String const& anId )
    {
        if( theRunningFlag )
        {
            THROW_EXCEPTION( Exception, 
                             "cannot create a Stepper during simulation" );
        }

        return theModel.createStepper( aClassname, anId );
    }

    Stepper* getStepper( String const& anId )
    {
        return theModel.getStepper( anId );
    }

    void deleteStepper( String const& anID )
    {
        theModel.deleteStepper( anID );
    }

    const py::list getStepperList() const
    {
        Model::StepperMap const& aStepperMap( theModel.getStepperMap() );
        py::list retval;

        for( Model::StepperMap::const_iterator i( aStepperMap.begin() );
             i != aStepperMap.end(); ++i )
        {
            retval.append( py::object( (*i).first ) );
        }

        return retval;
    }

    const StringVector
    getStepperPropertyList( String const& aStepperID ) const
    {
        StepperPtr aStepperPtr( theModel.getStepper( aStepperID ) );
        
        return aStepperPtr->getPropertyList();
    }

    PropertyAttributes
    getStepperPropertyAttributes( String const& aStepperID, 
                                  String const& aPropertyName ) const
    {
        StepperPtr aStepperPtr( theModel.getStepper( aStepperID ) );
        return aStepperPtr->getPropertyAttributes( aPropertyName );
    }

    void setStepperProperty( String const& aStepperID,
                             String const& aPropertyName,
                             Polymorph const& aValue )
    {
        StepperPtr aStepperPtr( theModel.getStepper( aStepperID ) );
        
        aStepperPtr->setProperty( aPropertyName, aValue );
    }

    const Polymorph
    getStepperProperty( String const& aStepperID,
                        String const& aPropertyName ) const
    {
        Stepper const * aStepperPtr( theModel.getStepper( aStepperID ) );

        return aStepperPtr->getProperty( aPropertyName );
    }

    void loadStepperProperty( String const& aStepperID,
                              String const& aPropertyName,
                              Polymorph const& aValue )
    {
        StepperPtr aStepperPtr( theModel.getStepper( aStepperID ) );
        
        aStepperPtr->loadProperty( aPropertyName, aValue );
    }

    const Polymorph
    saveStepperProperty( String const& aStepperID,
                         String const& aPropertyName ) const
    {
        Stepper const * aStepperPtr( theModel.getStepper( aStepperID ) );

        return aStepperPtr->saveProperty( aPropertyName );
    }

    const String
    getStepperClassName( String const& aStepperID ) const
    {
        Stepper const * aStepperPtr( theModel.getStepper( aStepperID ) );

        return aStepperPtr->getPropertyInterface().getClassName();
    }

    const PolymorphMap getClassInfo( String const& aClassname ) const
    {
        libecs::PolymorphMap aBuiltInfoMap;
        for ( DynamicModuleInfo::EntryIterator* anInfo(
              theModel.getPropertyInterface( aClassname ).getInfoFields() );
              anInfo->next(); )
        {
            aBuiltInfoMap.insert( std::make_pair( anInfo->current().first,
                                  *reinterpret_cast< const libecs::Polymorph* >(
                                    anInfo->current().second ) ) );
        }
        return aBuiltInfoMap;
    }

    
    Entity* createEntity( String const& aClassname, 
                          String const& aFullIDString )
    {
        if( theRunningFlag )
        {
            THROW_EXCEPTION( Exception, 
                             "cannot create an Entity during simulation" );
        }

        return theModel.createEntity( aClassname, FullID( aFullIDString ) );
    }
    
    Variable* createVariable( String const& aClassname )
    {
        return theModel.createVariable( aClassname );
    }

    Process* createProcess( String const& aClassname )
    {
        return theModel.createProcess( aClassname );
    }

    System* createSystem( String const& aClassname )
    {
        return theModel.createSystem( aClassname );
    }

    Entity* getEntity( String const& aFullIDString )
    {
        return theModel.getEntity( FullID( aFullIDString ) );
    }


    void deleteEntity( String const& aFullIDString )
    {
        THROW_EXCEPTION( NotImplemented,
                         "deleteEntity() method is not supported yet" );
    }

    const Polymorph 
    getEntityList( String const& anEntityTypeString,
                   String const& aSystemPathString ) const
    {
        const EntityType anEntityType( anEntityTypeString );
        const SystemPath aSystemPath( aSystemPathString );

        if( aSystemPath.size() == 0 )
        {
            PolymorphVector aVector;
            if( anEntityType == EntityType::SYSTEM )
            {
                aVector.push_back( Polymorph( "/" ) );
            }
            return aVector;
        }

        System const* aSystemPtr( theModel.getSystem( aSystemPath ) );

        switch( anEntityType )
        {
        case EntityType::VARIABLE:
            return aSystemPtr->getVariableList();
        case EntityType::PROCESS:
            return aSystemPtr->getProcessList();
        case EntityType::SYSTEM:
            return aSystemPtr->getSystemList();
        default:
            break;
        }

        NEVER_GET_HERE;
    }

    const StringVector
    getEntityPropertyList( String const& aFullIDString ) const
    {
        Entity const * anEntityPtr( theModel.getEntity( FullID( aFullIDString ) ) );

        return anEntityPtr->getPropertyList();
    }

    const bool entityExists( String const& aFullIDString ) const
    {
        try
        {
            IGNORE_RETURN theModel.getEntity( FullID( aFullIDString ) );
        }
        catch( const NotFound& )
        {
            return false;
        }

        return true;
    }

    void setEntityProperty( String const& aFullPNString,
                                    Polymorph const& aValue )
    {
        FullPN aFullPN( aFullPNString );
        EntityPtr anEntityPtr( theModel.getEntity( aFullPN.getFullID() ) );

        anEntityPtr->setProperty( aFullPN.getPropertyName(), aValue );
    }

    const Polymorph
    getEntityProperty( String const& aFullPNString ) const
    {
        FullPN aFullPN( aFullPNString );
        Entity const * anEntityPtr( theModel.getEntity( aFullPN.getFullID() ) );
                
        return anEntityPtr->getProperty( aFullPN.getPropertyName() );
    }

    void loadEntityProperty( String const& aFullPNString,
                             Polymorph const& aValue )
    {
        FullPN aFullPN( aFullPNString );
        EntityPtr anEntityPtr( theModel.getEntity( aFullPN.getFullID() ) );

        anEntityPtr->loadProperty( aFullPN.getPropertyName(), aValue );
    }

    const Polymorph
    saveEntityProperty( String const& aFullPNString ) const
    {
        FullPN aFullPN( aFullPNString );
        Entity const * anEntityPtr( theModel.getEntity( aFullPN.getFullID() ) );

        return anEntityPtr->saveProperty( aFullPN.getPropertyName() );
    }

    PropertyAttributes
    getEntityPropertyAttributes( String const& aFullPNString ) const
    {
        FullPN aFullPN( aFullPNString );
        Entity const * anEntityPtr( theModel.getEntity( aFullPN.getFullID() ) );

        return anEntityPtr->getPropertyAttributes( aFullPN.getPropertyName() );
    }

    const String
    getEntityClassName( String const& aFullIDString ) const
    {
        FullID aFullID( aFullIDString );
        Entity const * anEntityPtr( theModel.getEntity( aFullID ) );

        return anEntityPtr->getPropertyInterface().getClassName();
    }

    Logger* createLogger( String const& aFullPNString )
    {
        return createLogger( aFullPNString, Logger::Policy() );
    }

    Logger* createLogger( String const& aFullPNString,
                          Logger::Policy const& aParamList )
    {
        if( theRunningFlag )
        {
            THROW_EXCEPTION( Exception, 
                             "cannot create a Logger during simulation" );
        }

        Logger* retval( theModel.getLoggerBroker().createLogger(
            FullPN( aFullPNString ), aParamList ) );

        return retval;
    }

    Logger* createLogger( String const& aFullPNString,
                          py::object aParamList )
    {
        if ( !PySequence_Check( aParamList.ptr() )
             || PySequence_Size( aParamList.ptr() ) != 4 )
        {
            THROW_EXCEPTION( Exception,
                             "second argument must be a tuple of 4 items");
        }

        return createLogger( aFullPNString,
                Logger::Policy(
                    PyInt_AsLong( static_cast< py::object >( aParamList[ 0 ] ).ptr() ),
                    PyFloat_AsDouble( static_cast< py::object >( aParamList[ 1 ] ).ptr() ),
                    PyInt_AsLong( static_cast< py::object >( aParamList[ 2 ] ).ptr() ),
                    PyInt_AsLong( static_cast< py::object >( aParamList[ 3 ] ).ptr() ) ) );
    }

    const py::list getLoggerList() const
    {
        py::list retval;

        LoggerBroker const& aLoggerBroker( theModel.getLoggerBroker() );

        for( LoggerBroker::const_iterator
                i( aLoggerBroker.begin() ), end( aLoggerBroker.end() );
             i != end; ++i )
        {
            retval.append( py::object( (*i).first.asString() ) );
        }

        return retval;
    }

    boost::shared_ptr< DataPointVector > 
    getLoggerData( String const& aFullPNString ) const
    {
        return getLogger( aFullPNString )->getData();
    }

    boost::shared_ptr< DataPointVector >
    getLoggerData( String const& aFullPNString, 
                   Real const& startTime, Real const& endTime ) const
    {
        return getLogger( aFullPNString )->getData( startTime, endTime );
    }

    boost::shared_ptr< DataPointVector >
    getLoggerData( String const& aFullPNString,
                   Real const& start, Real const& end, 
                   Real const& interval ) const
    {
        return getLogger( aFullPNString )->getData( start, end, interval );
    }

    const Real 
    getLoggerStartTime( String const& aFullPNString ) const
    {
        return getLogger( aFullPNString )->getStartTime();
    }

    const Real 
    getLoggerEndTime( String const& aFullPNString ) const
    {
        return getLogger( aFullPNString )->getEndTime();
    }


    void setLoggerPolicy( String const& aFullPNString, 
                          Logger::Policy const& pol )
    {
        typedef PolymorphValue::Tuple Tuple;
        getLogger( aFullPNString )->setLoggerPolicy( pol );
    }

    void setLoggerPolicy( String const& aFullPNString,
                          py::object aParamList )
    {
        if ( !PySequence_Check( aParamList.ptr() )
            || PySequence_Size( aParamList.ptr() ) != 4 )
        {
            THROW_EXCEPTION( Exception,
                             "second parameter must be a tuple of 4 items");
        }

        return setLoggerPolicy( aFullPNString,
                Logger::Policy(
                    PyInt_AsLong( static_cast< py::object >( aParamList[ 0 ] ).ptr() ),
                    PyFloat_AsDouble( static_cast< py::object >( aParamList[ 1 ] ).ptr() ),
                    PyInt_AsLong( static_cast< py::object >( aParamList[ 2 ] ).ptr() ),
                    PyInt_AsLong( static_cast< py::object >( aParamList[ 3 ] ).ptr() ) ) );
    }

    Logger::Policy
    getLoggerPolicy( String const& aFullPNString ) const
    {
        return getLogger( aFullPNString )->getLoggerPolicy();
    }

    const Logger::size_type 
    getLoggerSize( String const& aFullPNString ) const
    {
        return getLogger( aFullPNString )->getSize();
    }

    const std::pair< Real, String > getNextEvent() const
    {
        StepperEvent const& aNextEvent( theModel.getTopEvent() );

        return std::make_pair(
            static_cast< Real >( aNextEvent.getTime() ),
            aNextEvent.getStepper()->getID() );
    }

    void step( const Integer aNumSteps )
    {
        if( aNumSteps <= 0 )
        {
            THROW_EXCEPTION( Exception,
                             "step( n ): n must be 1 or greater ("
                             + stringCast( aNumSteps ) + " given)" );
        }

        start();

        Integer aCounter( aNumSteps );
        do
        {
            theModel.step();
            
            --aCounter;
            
            if( aCounter == 0 )
            {
                stop();
                break;
            }

            if( aCounter % theEventCheckInterval == 0 )
            {
                handleEvent();

                if( ! theRunningFlag )
                {
                    break;
                }
            }
        }
        while( 1 );

    }

    const Real getCurrentTime() const
    {
        return theModel.getCurrentTime();
    }

    void run()
    {
        start();

        do
        {
            unsigned int aCounter( theEventCheckInterval );
            do
            {
                theModel.step();
                --aCounter;
            }
            while( aCounter != 0 );
            
            handleEvent();

        }
        while( theRunningFlag );
    }

    void run( const Real aDuration )
    {
        if( aDuration <= 0.0 )
        {
            THROW_EXCEPTION( Exception,
                             "duration must be greater than 0 ("
                             + stringCast( aDuration ) + " given)" );
        }

        start();

        const Real aStopTime( theModel.getCurrentTime() + aDuration );

        // setup SystemStepper to step at aStopTime

        //FIXME: dirty, ugly!
        Stepper* aSystemStepper( theModel.getSystemStepper() );
        aSystemStepper->setCurrentTime( aStopTime );
        aSystemStepper->setStepInterval( 0.0 );

        theModel.getScheduler().updateEvent( 0, aStopTime );


        if ( theEventHandler )
        {
            while ( theRunningFlag )
            {
                unsigned int aCounter( theEventCheckInterval );
                do 
                {
                    if( theModel.getTopEvent().getStepper() == aSystemStepper )
                    {
                        theModel.step();
                        stop();
                        break;
                    }
                    
                    theModel.step();

                    --aCounter;
                }
                while( aCounter != 0 );

                handleEvent();
            }
        }
        else
        {
            while ( theRunningFlag )
            {
                if( theModel.getTopEvent().getStepper() == aSystemStepper )
                {
                    theModel.step();
                    stop();
                    break;
                }

                theModel.step();
            }
        }

    }

    void stop()
    {
        theRunningFlag = false;

        theModel.flushLoggers();
    }

    void setEventHandler( py::handle<> const& anEventHandler )
    {
        theEventHandler = anEventHandler;
    }

    const py::object getDMInfo() const
    {
        typedef ModuleMaker< EcsObject >::ModuleMap ModuleMap;
        const ModuleMap& modules( thePropertiedObjectMaker.getModuleMap() );
        py::list retval;

        for( ModuleMap::const_iterator i( modules.begin() );
                    i != modules.end(); ++i )
        {
            const PropertyInterfaceBase* info(
                reinterpret_cast< const PropertyInterfaceBase *>(
                    i->second->getInfo() ) );
            const char* aFilename( i->second->getFileName() );

            retval.append( py::make_tuple(
                py::object( info->getTypeName() ),
                py::object( i->second->getModuleName() ),
                py::object( aFilename ? aFilename: "" ) ) );
        }

        return retval;
    }

    PropertyInterfaceBase::PropertySlotMap const&
    getPropertyInfo( String const& aClassname ) const
    {
        return theModel.getPropertyInterface( aClassname ).getPropertySlotMap();
    }

    static const char getDMSearchPathSeparator()
    {
        return Model::PATH_SEPARATOR;
    }

    const std::string getDMSearchPath() const
    {
        return theModel.getDMSearchPath();
    }

    void setDMSearchPath( const std::string& aDMSearchPath )
    {
        theModel.setDMSearchPath( aDMSearchPath );
    }

    Logger* getLogger( String const& aFullPNString ) const
    {
        return theModel.getLoggerBroker().getLogger( aFullPNString );
    }

    void addPythonDM( py::object obj )
    {
        if ( !PyType_Check( obj.ptr() ) )
        {
            PyErr_SetString( PyExc_TypeError, "argument must be a type object" );
            py::throw_error_already_set();
        }

        EntityType aDMType( DMTypeResolverHelper()( obj ) );

        DynamicModule< EcsObject >* aModule( 0 );

        switch ( aDMType.getType() )
        {
        case EntityType::PROCESS:
            aModule = new PythonDynamicModule< PythonProcess >( obj );
            break;

        case EntityType::VARIABLE:
            aModule = new PythonDynamicModule< PythonVariable >( obj );
            break;

        case EntityType::SYSTEM:
            aModule = new PythonDynamicModule< PythonSystem >( obj );
            break;

        default:
            THROW_EXCEPTION( NotImplemented, "not implemented" );
        }
    
        thePropertiedObjectMaker.addClass( aModule );
    }

protected:

    ModelRef getModel() 
    { 
        return theModel; 
    }

    Model const& getModel() const 
    { 
        return theModel; 
    }

    inline void handleEvent()
    {
        for (;;)
        {
            if ( PyErr_CheckSignals() )
            {
                stop();
                break;
            }

            if ( PyErr_Occurred() )
            {
                stop();
                py::throw_error_already_set();
            }

            if ( !theEventHandler )
            {
                break;
            }

            if ( !PyObject_IsTrue( py::handle<>(
                    PyObject_CallFunction( theEventHandler.get(), NULL ) ).get() ) )
            {
                break;
            }
        }
    }

    void start()
    {
        theRunningFlag = true;
    }

private:

    bool                    theRunningFlag;

    mutable bool            theDirtyFlag;

    Integer         theEventCheckInterval;

    py::handle<>    theEventHandler;

    std::auto_ptr< ModuleMaker< EcsObject > > theDefaultPropertiedObjectMaker;
    CompositeModuleMaker thePropertiedObjectMaker;
    Model           theModel;
};

inline const char* typeCodeToString( enum PropertySlotBase::Type aTypeCode )
{
    switch ( aTypeCode )
    {
    case PropertySlotBase::POLYMORPH:
        return "polymorph";
    case PropertySlotBase::REAL:
        return "real";
    case PropertySlotBase::INTEGER:
        return "integer";
    case PropertySlotBase::STRING:
        return "string";
    }
    return "???";
}

static std::string PropertyAttributes___str__( PropertyAttributes const* self )
{
    std::string retval;
    retval += "{type=";
    retval += typeCodeToString( self->getType() );
    retval += ", ";
    retval += "settable="
              + stringCast( self->isSetable() )
              + ", ";
    retval += "gettable="
              + stringCast( self->isGetable() )
              + ", ";
    retval += "loadable="
              + stringCast( self->isLoadable() )
              + ", ";
    retval += "savable="
              + stringCast( self->isSavable() )
              + ", ";
    retval += "dynamic="
              + stringCast( self->isDynamic() )
              + "}";
    return retval;
}

static int PropertyAttributes___getitem__( PropertyAttributes const* self, int idx )
{
    switch ( idx )
    {
    case 0:
        return self->isSetable();
    case 1:
        return self->isGetable();
    case 2:
        return self->isLoadable();
    case 3:
        return self->isSavable();
    case 4:
        return self->isDynamic();
    case 5:
        return self->getType();
    }

    throw std::range_error("Index out of bounds");
}

static py::object LoggerPolicy_GetItem( Logger::Policy const* self, int idx )
{
    switch ( idx )
    {
    case 0:
        return py::object( self->getMinimumStep() );
    case 1:
        return py::object( self->getMinimumTimeInterval() );
    case 2:
        return py::object( self->doesContinueOnError() );
    case 3:
        return py::object( self->getMaxSpace() );
    }

    throw std::range_error("Index out of bounds");
}

static py::object Process_get_variableReferences( Process* self )
{
    return py::object( VariableReferences( self ) );
}

static Polymorph Entity___getattr__( Entity* self, std::string key )
{
    if ( key == "__members__" || key == "__methods__" )
    {
        PyErr_SetString( PyExc_KeyError, key.c_str() );
        py::throw_error_already_set();
    }

    if ( key.size() > 0 )
        key[ 0 ] = std::toupper( key[ 0 ] );
    return self->getProperty( key );
}

static void Entity___setattr__( py::object aSelf, py::object key, py::object value )
{
    py::handle<> aDescr( py::allow_null( PyObject_GetAttr( reinterpret_cast< PyObject* >( aSelf.ptr()->ob_type ), key.ptr() ) ) );
    if ( !aDescr || !( aDescr->ob_type->tp_flags & Py_TPFLAGS_HAVE_CLASS ) || !aDescr.get()->ob_type->tp_descr_set )
    {
        PyErr_Clear();
        Entity* self = py::extract< Entity* >( aSelf );
        std::string keyStr = py::extract< std::string >( key );
        if ( keyStr.size() > 0 )
            keyStr[ 0 ] = std::toupper( keyStr[ 0 ] );
        self->setProperty( keyStr, py::extract< Polymorph >( value ) );
    }
    else
    {
        aDescr.get()->ob_type->tp_descr_set( aDescr.get(), aSelf.ptr(), value.ptr() );
    }
}

static Simulator& Entity___get_simulator__( Entity* self )
{
    return static_cast< Simulator::Model const* >( self->getModel() )->getSimulator();
}

template< typename T_ >
static PyObject* writeOnly( T_* )
{
    PyErr_SetString( PyExc_AttributeError, "Write-only attributes." );
    return py::incref( Py_None );
}

static void setWarningHandler( py::handle<> const& handler )
{
    static PythonWarningHandler thehandler;
    thehandler = PythonWarningHandler( handler );
    libecs::setWarningHandler( &thehandler );
}

struct return_entity
    : public py::with_custodian_and_ward_postcall<
            0, 1, py::default_call_policies >
{
    struct result_converter
    {
        template< typename T_ >
        struct apply
        {
            struct type
            {
                PyObject* operator()( Entity* ptr ) const
                {
                    if ( ptr == 0 )
                        return py::detail::none();
                    else
                        return ( *this )( *ptr );
                }
                
                PyObject* operator()( Entity const& x ) const
                {
                    Entity* const ptr( const_cast< Entity* >( &x ) );
                    PyObject* aRetval( py::detail::wrapper_base_::owner( ptr ) );
                    if ( !aRetval )
                    {
                        if ( Process* tmp = dynamic_cast< Process* >( ptr ) )
                        {
                            aRetval = py::detail::make_reference_holder::execute( tmp );
                        }
                        else if ( Variable* tmp = dynamic_cast< Variable* >( ptr ) )
                        {
                            aRetval = py::detail::make_reference_holder::execute( tmp );
                        }
                        else if ( System* tmp = dynamic_cast< System* >( ptr ) )
                        {
                            aRetval = py::detail::make_reference_holder::execute( tmp );
                        }
                        else
                        {
                            aRetval = py::detail::make_reference_holder::execute( ptr );
                        }
                    }
                    return aRetval;
                }
            };
        };
    };
};

BOOST_PYTHON_MODULE( _ecs )
{
    DataPointVectorWrapper< DataPoint >::__class_init__();
    DataPointVectorWrapper< LongDataPoint >::__class_init__();
    STLIteratorWrapper< VariableReferenceVector::const_iterator >::__class_init__();

    // without this it crashes when Logger::getData() is called. why?
    import_array();

    registerTupleConverters< std::pair< Real, String > >();
    PolymorphToPythonConverter::addToRegistry();
    StringKeyedMapToPythonConverter< PolymorphMap >::addToRegistry();
    StringVectorToPythonConverter::addToRegistry();
    PropertySlotMapToPythonConverter::addToRegistry();
    DataPointVectorSharedPtrConverter::addToRegistry();
    FullIDToPythonConverter::addToRegistry();

    PolymorphRetriever::addToRegistry();

    // functions
    py::register_exception_translator< Exception >( &translateException );
    py::register_exception_translator< std::exception >( &translateException );
    py::register_exception_translator< std::range_error >( &translateRangeError );
    PythonVariable::addToRegistry();
    PythonProcess::addToRegistry();
    PythonSystem::addToRegistry();

    py::def( "getLibECSVersionInfo", &getLibECSVersionInfo );
    py::def( "getLibECSVersion",     &getVersion );
    py::def( "setWarningHandler",    &::setWarningHandler );

    typedef py::return_value_policy< py::reference_existing_object >
            return_existing_object;
    typedef py::return_value_policy< py::copy_const_reference >
            return_copy_const_reference;

    py::class_< PropertyAttributes >( "PropertyAttributes",
        py::init< enum PropertySlotBase::Type, bool, bool, bool, bool, bool >() )
        .add_property( "type", &PropertyAttributes::getType )
        .add_property( "setable", &PropertyAttributes::isSetable )
        .add_property( "getable", &PropertyAttributes::isGetable )
        .add_property( "loadable", &PropertyAttributes::isLoadable )
        .add_property( "savable", &PropertyAttributes::isSavable )
        .add_property( "dynamic", &PropertyAttributes::isDynamic )
        .def( "__str__", &PropertyAttributes___str__ )
        .def( "__getitem__", &PropertyAttributes___getitem__ )
        ;

    py::class_< Logger::Policy >( "LoggerPolicy", py::init<>() )
        .add_property( "minimumStep", &Logger::Policy::getMinimumStep,
                                      &Logger::Policy::setMinimumStep )
        .add_property( "minimumTimeInterval",
                       &Logger::Policy::getMinimumTimeInterval,
                       &Logger::Policy::setMinimumTimeInterval )
        .add_property( "continueOnError",
                       &Logger::Policy::doesContinueOnError,
                       &Logger::Policy::setContinueOnError )
        .add_property( "maxSpace",
                       &Logger::Policy::getMaxSpace,
                       &Logger::Policy::setMaxSpace )
        .def( "__getitem__", &LoggerPolicy_GetItem )
        ;

    py::class_< VariableReferences >( "VariableReferences", py::no_init )
        .add_property( "positiveReferences",
                       &VariableReferences::getPositivesReferences )
        .add_property( "zeroReferences",
                       &VariableReferences::getZeroReferences )
        .add_property( "negativeReferences",
                       &VariableReferences::getNegativeReferences )
        .def( "add",
              ( void ( VariableReferences::* )( String const&, String const&, Integer const&, bool ) )
              &VariableReferences::add )
        .def( "add",
              ( void ( VariableReferences::* )( String const&, String const&, Integer const& ) )
              &VariableReferences::add )
        .def( "remove", &VariableReferences::remove )
        .def( "__getitem__", &VariableReferences::__getitem__,
              return_copy_const_reference() )
        .def( "__len__", &VariableReferences::__len__ )
        .def( "__iter__", &VariableReferences::__iter__ )
        .def( "__repr__", &VariableReferences::__repr__ )
        .def( "__str__", &VariableReferences::__repr__ )
        ;

    py::class_< VariableReference >( "VariableReference", py::no_init )
        // properties
        .add_property( "superSystem",
            py::make_function(
                &VariableReference::getSuperSystem,
                py::return_internal_reference<> () ) )
        .add_property( "coefficient", &VariableReference::getCoefficient )
        .add_property( "name",        &VariableReference::getName )
        .add_property( "isAccessor",  &VariableReference::isAccessor )
        .add_property( "variable",
                py::make_function(
                    &VariableReference::getVariable,
                    return_existing_object() ) )
        .def( "__str__", &VariableReference___repr__ )
        .def( "__repr__", &VariableReference___repr__ )
        ;

    py::class_< Stepper, py::bases<>, Stepper, boost::noncopyable >
        ( "Stepper", py::no_init )
        .add_property( "id", &Stepper::getID, &Stepper::setID )
        .add_property( "priority",
                       &Stepper::getPriority,
                       &Stepper::setPriority )
        .add_property( "stepInterval",
                       &Stepper::getStepInterval, 
                       &Stepper::setStepInterval )
        .add_property( "maxStepInterval",
                       &Stepper::getMaxStepInterval,
                       &Stepper::setMaxStepInterval )
        .add_property( "minStepInterval",
                       &Stepper::getMinStepInterval,
                       &Stepper::setMinStepInterval )
        .add_property( "rngSeed", &writeOnly<Stepper>, &Stepper::setRngSeed )
        ;

    py::class_< Entity, py::bases<>, Entity, boost::noncopyable >
        ( "Entity", py::no_init )
        // properties
        .add_property( "simulator",
            py::make_function( &Entity___get_simulator__,
                return_existing_object() ) )
        .add_property( "superSystem",
            py::make_function( &Entity::getSuperSystem,
            return_existing_object() ) )
        .add_property( "id", &Entity::getID, &Entity::setID )
        .add_property( "fullID", &Entity::getFullID )
        .add_property( "name", &Entity::getName )
        .def( "__setattr__", &Entity___setattr__ )
        .def( "__getattr__", &Entity___getattr__ )
        ;

    py::class_< System, py::bases< Entity >, System, boost::noncopyable>
        ( "System", py::no_init )
        // properties
        .add_property( "size",        &System::getSize )
        .add_property( "sizeN_A",     &System::getSizeN_A )
        .add_property( "stepperID",   &System::getStepperID )
        ;

    py::class_< Process, py::bases< Entity >, Process, boost::noncopyable >
        ( "Process", py::no_init )
        .add_property( "activity",  &Process::getActivity,
                                    &Process::setActivity )
        .add_property( "isContinuous", &Process::isContinuous )
        .add_property( "priority",  &Process::getPriority )
        .add_property( "stepperID", &Process::getStepperID )
        .add_property( "variableReferences",
              py::make_function( &Process_get_variableReferences ) )
        ;

    py::class_< Variable, py::bases< Entity >, Variable, boost::noncopyable >
        ( "Variable", py::no_init )
        .add_property( "value",  &Variable::getValue,
                                 &Variable::setValue )
        .add_property( "molarConc",  &Variable::getMolarConc,
                                     &Variable::setMolarConc  )
        .add_property( "numberConc", &Variable::getNumberConc,
                                     &Variable::setNumberConc )
        ;

    py::class_< Logger, py::bases<>, Logger, boost::noncopyable >( "Logger", py::no_init )
        .add_property( "StartTime", &Logger::getStartTime )
        .add_property( "EndTime", &Logger::getEndTime )
        .add_property( "Size", &Logger::getSize )
        .add_property( "Policy",
            py::make_function(
                &Logger::getLoggerPolicy,
                return_copy_const_reference() ) )
        .def( "getData", 
              ( boost::shared_ptr< DataPointVector >( Logger::* )( void ) const )
              &Logger::getData )
        .def( "getData", 
              ( boost::shared_ptr< DataPointVector >( Logger::* )(
                RealParam, RealParam ) const )
              &Logger::getData )
        .def( "getData",
              ( boost::shared_ptr< DataPointVector >( Logger::* )(
                     RealParam, RealParam, RealParam ) const )
              &Logger::getData )
        ;

    // Simulator class
    py::class_< Simulator, py::bases<>, boost::shared_ptr< Simulator >, boost::noncopyable >( "Simulator" )
        .def( py::init<>() )
        .def( "getClassInfo",
              &Simulator::getClassInfo )
        // Stepper-related methods
        .def( "createStepper",
              &Simulator::createStepper,
              py::return_internal_reference<>() )
        .def( "getStepper",
              &Simulator::getStepper,
              py::return_internal_reference<>() )
        .def( "deleteStepper",
              &Simulator::deleteStepper )
        .def( "getStepperList",
              &Simulator::getStepperList )
        .def( "getStepperPropertyList",
              &Simulator::getStepperPropertyList )
        .def( "getStepperPropertyAttributes", 
              &Simulator::getStepperPropertyAttributes )
        .def( "setStepperProperty",
              &Simulator::setStepperProperty )
        .def( "getStepperProperty",
              &Simulator::getStepperProperty )
        .def( "loadStepperProperty",
              &Simulator::loadStepperProperty )
        .def( "saveStepperProperty",
              &Simulator::saveStepperProperty )
        .def( "getStepperClassName",
              &Simulator::getStepperClassName )

        // Entity-related methods
        .def( "createEntity",
              &Simulator::createEntity,
              return_entity() )
        .def( "createVariable",
              &Simulator::createVariable,
              py::return_internal_reference<>() )
        .def( "createProcess",
              &Simulator::createProcess,
              py::return_internal_reference<>() )
        .def( "createSystem",
              &Simulator::createSystem,
              py::return_internal_reference<>() )
        .def( "getEntity",
              &Simulator::getEntity,
              return_entity() )
        .def( "deleteEntity",
              &Simulator::deleteEntity )
        .def( "getEntityList",
              &Simulator::getEntityList )
        .def( "entityExists",
              &Simulator::entityExists )
        .def( "getEntityPropertyList",
              &Simulator::getEntityPropertyList )
        .def( "setEntityProperty",
              &Simulator::setEntityProperty )
        .def( "getEntityProperty",
              &Simulator::getEntityProperty )
        .def( "loadEntityProperty",
              &Simulator::loadEntityProperty )
        .def( "saveEntityProperty",
              &Simulator::saveEntityProperty )
        .def( "getEntityPropertyAttributes", 
              &Simulator::getEntityPropertyAttributes )
        .def( "getEntityClassName",
              &Simulator::getEntityClassName )

        // Logger-related methods
        .def( "getLoggerList",
                    &Simulator::getLoggerList )    
        .def( "createLogger",
              ( Logger* ( Simulator::* )( String const& ) )
                    &Simulator::createLogger,
              py::return_internal_reference<> () )
        .def( "createLogger",                                 
              ( Logger* ( Simulator::* )( String const&, Logger::Policy const& ) )
              &Simulator::createLogger,
              py::return_internal_reference<>() )
        .def( "createLogger",                                 
              ( Logger* ( Simulator::* )( String const&, py::object ) )
                    &Simulator::createLogger,
              py::return_internal_reference<> () )
        .def( "getLogger", &Simulator::getLogger,
              py::return_internal_reference<>() )
        .def( "getLoggerData", 
              ( boost::shared_ptr< DataPointVector >( Simulator::* )(
                    String const& ) const )
              &Simulator::getLoggerData )
        .def( "getLoggerData", 
              ( boost::shared_ptr< DataPointVector >( Simulator::* )(
                    String const&, Real const&, Real const& ) const )
              &Simulator::getLoggerData )
        .def( "getLoggerData",
              ( boost::shared_ptr< DataPointVector >( Simulator::* )(
                     String const&, Real const&, 
                     Real const&, Real const& ) const )
              &Simulator::getLoggerData )
        .def( "getLoggerStartTime",
              &Simulator::getLoggerStartTime )    
        .def( "getLoggerEndTime",
              &Simulator::getLoggerEndTime )        
        .def( "getLoggerPolicy",
              &Simulator::getLoggerPolicy )
        .def( "setLoggerPolicy",
              ( void (Simulator::*)(
                    String const&, Logger::Policy const& ) )
              &Simulator::setLoggerPolicy )
        .def( "setLoggerPolicy",
              ( void (Simulator::*)(
                    String const& aFullPNString,
                    py::object aParamList ) )
              &Simulator::setLoggerPolicy )
        .def( "getLoggerSize",
              &Simulator::getLoggerSize )

        // Simulation-related methods
        .def( "initialize",
              &Simulator::initialize )
        .def( "getCurrentTime",
              &Simulator::getCurrentTime )
        .def( "getNextEvent",
              &Simulator::getNextEvent )
        .def( "stop",
              &Simulator::stop )
        .def( "step",
              ( void ( Simulator::* )( const Integer ) )
              &Simulator::step )
        .def( "run",
              ( void ( Simulator::* )() )
              &Simulator::run )
        .def( "run",
              ( void ( Simulator::* )( const Real ) ) 
              &Simulator::run )
        .def( "getPropertyInfo",
              &Simulator::getPropertyInfo,
              return_copy_const_reference() )
        .def( "getDMInfo",
              &Simulator::getDMInfo )
        .def( "setEventHandler",
              &Simulator::setEventHandler )
        .add_static_property( "DM_SEARCH_PATH_SEPARATOR",
              &Simulator::getDMSearchPathSeparator )
        .def( "setDMSearchPath", &Simulator::setDMSearchPath )
        .def( "getDMSearchPath", &Simulator::getDMSearchPath )
        .def( "addPythonDM", &Simulator::addPythonDM )
        ;
}
