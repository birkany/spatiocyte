#define GSL_RANGE_CHECK_OFF

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

#include <map>

#include "libecs.hpp"
#include "Process.hpp"
#include "Util.hpp"
#include "FullID.hpp"
#include "PropertyInterface.hpp"

#include "System.hpp"
#include "Stepper.hpp"
#include "Variable.hpp"
#include "VariableProxy.hpp"

#include "QuasiDynamicFluxProcess.hpp"

USE_LIBECS;

LIBECS_DM_CLASS( FluxDistributionProcess, Process )
{

 public:

  LIBECS_DM_OBJECT( FluxDistributionProcess, Process )
    {
      INHERIT_PROPERTIES( Process );

      PROPERTYSLOT_SET_GET( Polymorph, KnownProcessList );
      PROPERTYSLOT_SET_GET( Polymorph, UnknownProcessList );
      PROPERTYSLOT_SET_GET( Real, Epsilon );
    }

  FluxDistributionProcess();
  ~FluxDistributionProcess();

  SIMPLE_SET_GET_METHOD( Polymorph, KnownProcessList );
  SIMPLE_SET_GET_METHOD( Polymorph, UnknownProcessList );
  SIMPLE_SET_GET_METHOD( Real, Epsilon );

  void initialize();  
  void process();
  
 protected:

  gsl_matrix* generateInverse( gsl_matrix *m_unknown, 
			       Int matrix_size );

 protected:

  Polymorph KnownProcessList;
  Polymorph UnknownProcessList;

  gsl_matrix* theKnownMatrix;
  gsl_matrix* theUnknownMatrix;
  gsl_matrix* theInverseMatrix;
  gsl_matrix* theSolutionMatrix;

  gsl_matrix* theTmpInverseMatrix;
  gsl_matrix* theTmpUnknownMatrix;


  gsl_vector* theKnownVelocityVector;
  gsl_vector* theSolutionVector;
  gsl_vector* theFluxVector;

  ProcessVector theKnownProcessPtrVector;
  std::vector< QuasiDynamicFluxProcessPtr > theUnknownProcessPtrVector;

  bool   theIrreversibleFlag;
  Int    theMatrixSize;
  Real   Epsilon;

};

LIBECS_DM_INIT( FluxDistributionProcess, Process );

using namespace libecs;

FluxDistributionProcess::FluxDistributionProcess()
  :
  theMatrixSize( 0 ),
  theIrreversibleFlag( false ),
  theKnownMatrix( NULLPTR ),
  theUnknownMatrix( NULLPTR ),
  theInverseMatrix( NULLPTR ),
  theSolutionMatrix( NULLPTR ),
  theTmpInverseMatrix( NULLPTR ),
  theTmpUnknownMatrix( NULLPTR ),
  theKnownVelocityVector( NULLPTR ),
  theSolutionVector( NULLPTR ),
  theFluxVector( NULLPTR ),
  Epsilon( 1e-6 )
{
  ; // do nothing
}

FluxDistributionProcess::~FluxDistributionProcess()
{
  gsl_matrix_free( theUnknownMatrix );
  gsl_matrix_free( theKnownMatrix );
  gsl_matrix_free( theInverseMatrix );
  gsl_matrix_free( theSolutionMatrix );
  gsl_matrix_free( theTmpInverseMatrix );
  gsl_matrix_free( theTmpUnknownMatrix );
  gsl_vector_free( theKnownVelocityVector );
  gsl_vector_free( theSolutionVector );
  gsl_vector_free( theFluxVector );
}

gsl_matrix* FluxDistributionProcess::generateInverse(gsl_matrix *m_unknown, 
						     Int matrix_size)
{      
  gsl_matrix *m_tmp1, *m_tmp2, *m_tmp3;
  gsl_matrix *inv;
  gsl_matrix *V; 
  gsl_vector *S, *work;
  
  m_tmp1 = gsl_matrix_calloc(matrix_size,matrix_size);
  m_tmp2 = gsl_matrix_calloc(matrix_size,matrix_size);
  m_tmp3 = gsl_matrix_calloc(matrix_size,matrix_size);
  inv = gsl_matrix_calloc(matrix_size,matrix_size);
  
  V = gsl_matrix_calloc (matrix_size,matrix_size); 
  S = gsl_vector_calloc (matrix_size);             
  work = gsl_vector_calloc (matrix_size);          
      
  gsl_matrix_memcpy(m_tmp1,m_unknown);
  gsl_linalg_SV_decomp(m_tmp1,V,S,work);
      
  //generate Singular Value Matrix
  
  for(int i=0;i<matrix_size;i++){
    double singular_value = gsl_vector_get(S,i);
    if(singular_value > Epsilon){
      gsl_matrix_set(m_tmp2,i,i,1/singular_value);
    }
    else
      {
	gsl_matrix_set(m_tmp2,i,i,0.0);
      }
  }
  
  gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,m_tmp2,m_tmp1,0.0,m_tmp3);
  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,V,m_tmp3,0.0,inv);
  
  gsl_matrix_free(m_tmp1);
  gsl_matrix_free(m_tmp2);
  gsl_matrix_free(m_tmp3);
  gsl_matrix_free(V);
  gsl_vector_free(S);
  gsl_vector_free(work);
  
  return inv;  
}

void FluxDistributionProcess::initialize()
{
  
  Process::initialize();
  
  //
  // generate Variable Map
  //
  
  std::map< VariablePtr, Int > aVariableMap;
  VariableReferenceVector aVariableVector( getVariableReferenceVector() );
  for(Int i( 0 ); i < aVariableVector.size(); i++ )
    {
      aVariableMap.insert( std::pair< VariablePtr, Int >( aVariableVector[i].getVariable(), i  ) );
    }
  
  //
  // gsl Matrix & Vector allocation
  //
  
  if( ( UnknownProcessList.asPolymorphVector() ).size() > aVariableVector.size() )
    {
      theMatrixSize = ( UnknownProcessList.asPolymorphVector() ).size();
    }else{
      theMatrixSize = aVariableVector.size(); 
    }

  if( theUnknownMatrix != NULLPTR )
    { 
      gsl_matrix_free( theUnknownMatrix );
      gsl_matrix_free( theKnownMatrix );
      gsl_matrix_free( theTmpInverseMatrix );
      gsl_matrix_free( theSolutionMatrix );
      gsl_matrix_free( theTmpUnknownMatrix );
      
      gsl_vector_free( theKnownVelocityVector );
      gsl_vector_free( theSolutionVector );
      gsl_vector_free( theFluxVector );
    }
  
  theUnknownMatrix = gsl_matrix_calloc( theMatrixSize, theMatrixSize );
  theKnownMatrix = gsl_matrix_calloc( theMatrixSize, KnownProcessList.asPolymorphVector().size() );

  if( KnownProcessList.asPolymorphVector().size() < theMatrixSize )
    {
      theSolutionMatrix = gsl_matrix_calloc( theMatrixSize, KnownProcessList.asPolymorphVector().size() );    
    }
  else
    {
      theSolutionMatrix = gsl_matrix_calloc( theMatrixSize, theMatrixSize );
    }

  theTmpInverseMatrix = gsl_matrix_calloc( theMatrixSize, theMatrixSize );
  theTmpUnknownMatrix = gsl_matrix_calloc( theMatrixSize, theMatrixSize );
  theKnownVelocityVector = gsl_vector_calloc( ( KnownProcessList.asPolymorphVector() ).size() );
  theSolutionVector = gsl_vector_calloc( theMatrixSize );
  theFluxVector = gsl_vector_calloc( theMatrixSize );
  
  //
  // generate theUnknownMatrix
  //
  
  theUnknownProcessPtrVector.clear();
  
  for(Int i(0); i < ( UnknownProcessList.asPolymorphVector() ).size(); i++ ) 
    {
      const FullID aFullID( ( UnknownProcessList.asPolymorphVector() )[i].asString() );
      
      SystemPtr aSystem( getSuperSystem()->
			 getSystem( aFullID.getSystemPath() ) );

      QuasiDynamicFluxProcessPtr aProcess( dynamic_cast<QuasiDynamicFluxProcessPtr>(aSystem->getProcess( aFullID.getID() ) ) );

      theUnknownProcessPtrVector.push_back( aProcess );
      VariableReferenceVector aVector( aProcess->getVariableReferenceVector());
      for( Int j(0); j < aVector.size(); j++ )
	{
	  gsl_matrix_set( theUnknownMatrix, aVariableMap.find( aVector[j].getVariable() )->second, i, aVector[j].getCoefficient() );
	}
    }
  
  //
  // check Irreversible
  //

  theIrreversibleFlag = false;
  for( Int i( 0 ); i < ( UnknownProcessList.asPolymorphVector() ).size(); i++ )
    {
      if( theUnknownProcessPtrVector[i]->getIrreversible() )
	{
	  theIrreversibleFlag = true;
	}
    }
  
  //
  // generate KnownProcess Matrix
  //
  
  theKnownProcessPtrVector.clear();
  
  for( Int i( 0 ); i < ( KnownProcessList.asPolymorphVector() ).size(); i++ ) 
    {
      const FullID aFullID( ( KnownProcessList.asPolymorphVector() )[i].asString() );
      SystemPtr aSystem( getSuperSystem()->
			 getSystem( aFullID.getSystemPath() ) );
      
      ProcessPtr aProcess( aSystem->getProcess( aFullID.getID() ) );
      theKnownProcessPtrVector.push_back( aProcess );
      VariableReferenceVector aVector( aProcess->getVariableReferenceVector() );
      for( Int j(0); j < aVector.size(); j++ )
	{
	  gsl_matrix_set( theKnownMatrix, aVariableMap.find( aVector[j].getVariable() )->second, i, aVector[j].getCoefficient());
	}
    }
  
  //
  // generate inverse matrix
  //
  if( theInverseMatrix != NULLPTR )
    { 
      gsl_matrix_free( theInverseMatrix );
    }

  theInverseMatrix = generateInverse( theUnknownMatrix , theMatrixSize );
  
  gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, -1.0, theInverseMatrix, 
		  theKnownMatrix, 0.0, theSolutionMatrix );

}  

void FluxDistributionProcess::process()
{
  
  for( Int i( 0 ); i < ( KnownProcessList.asPolymorphVector() ).size(); i++ )
    {
      gsl_vector_set( theKnownVelocityVector, i, theKnownProcessPtrVector[i]->getActivity() );
    }
  
  gsl_blas_dgemv( CblasNoTrans, 1.0, theSolutionMatrix, 
		  theKnownVelocityVector, 0.0, theSolutionVector );

  if ( theIrreversibleFlag )
    {
      bool aFlag( false );
      std::vector<Int> aVector;
      for( Int i( 0 ); i < ( UnknownProcessList.asPolymorphVector() ).size();
	   ++i )
	{
	  if( ( theUnknownProcessPtrVector[i]->getIrreversible() ) &&
	      ( gsl_vector_get(theSolutionVector, i) < 0 ) )
	    {
	      aFlag = true;
	      aVector.push_back(i);
	    }
	}		    
      
      if( aFlag )
	{
	  gsl_matrix_memcpy( theTmpUnknownMatrix, theUnknownMatrix );
	  for( Int i( 0 ); i < aVector.size(); i++ )
	    {
	      for( Int j( 0 ); j < theMatrixSize; j++ )
		{
		  gsl_matrix_set( theTmpUnknownMatrix, j, aVector[i], 0 );
		}
	    }
	  
	  theTmpInverseMatrix = generateInverse( theTmpUnknownMatrix,
						 theMatrixSize );    
	  gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, -1.0, 
			  theTmpInverseMatrix, 
			  theKnownMatrix, 0.0, theSolutionMatrix );
	  gsl_blas_dgemv( CblasNoTrans, 1.0, theSolutionMatrix, 
			  theKnownVelocityVector, 0.0, theSolutionVector );
	}	
    }

  for( Int i( 0 ); i < ( UnknownProcessList.asPolymorphVector() ).size(); i++ )
    {
      theUnknownProcessPtrVector[i]->setFlux( gsl_vector_get( theSolutionVector, i ) ) ;
    }
  
}
