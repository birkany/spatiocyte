//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//        This file is part of E-CELL Simulation Environment package
//
//                Copyright (C) 1996-2002 Keio University
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//
// E-CELL is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// E-CELL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with E-CELL -- see the file COPYING.
// If not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// 
//END_HEADER
//
// written by Kouichi Takahashi <shafi@e-cell.org> at
// E-CELL Project, Lab. for Bioinformatics, Keio University.
//
// modified by Masayuki Okayama <smash@e-cell.org> at
// E-CELL Project, Lab. for Bioinformatics, Keio University.
//

#include "PropertySlotMaker.hpp"

#include "PropertyInterface.hpp"

namespace libecs
{


  ///////////////////////////// PropertyInterface

  PropertySlotMakerPtr PropertyInterface::getPropertySlotMaker()
  {
    static PropertySlotMaker aPropertySlotMaker;

    return &aPropertySlotMaker;
  }


  void PropertyInterface::makeSlots()
  {

    // to be deprecated
    registerSlot( getPropertySlotMaker()->
		  createPropertySlot( "ClassName", *this, 
				      Type2Type<String>(),
				      NULLPTR,
				      &PropertyInterface::
				      getClassNameString ) );
    

    // to be deprecated
    registerSlot( getPropertySlotMaker()->
		  createPropertySlot( "PropertyList",*this,
				      Type2Type<Polymorph>(),
				      NULLPTR,
				      &PropertyInterface::getPropertyListWithAttributes ) );
    

  }

  const Polymorph PropertyInterface::getPropertyListWithAttributes() const
  {
    PolymorphVector aVector;
    aVector.reserve( thePropertySlotMap.size() );

    for( PropertySlotMapConstIterator i( thePropertySlotMap.begin() ); 
	 i != thePropertySlotMap.end() ; ++i )
      {
	PolymorphVector anInnerVector;
	PropertySlotPtr aPropertySlotPtr( i->second );

	// the name of the property slot
	anInnerVector.push_back( aPropertySlotPtr->getName() );

	// is setable?
	anInnerVector.push_back( static_cast<Int>
				 ( aPropertySlotPtr->isSetable() ) );

	// is getable?
	anInnerVector.push_back( static_cast<Int>
				 ( aPropertySlotPtr->isGetable() ) );

	aVector.push_back( anInnerVector );
      }

    return aVector;
  }

  const Polymorph PropertyInterface::getPropertyList() const
  {
    PolymorphVector aVector;
    aVector.reserve( thePropertySlotMap.size() );

    for( PropertySlotMapConstIterator i( thePropertySlotMap.begin() ); 
	 i != thePropertySlotMap.end() ; ++i )
      {
	PropertySlotPtr aPropertySlotPtr( i->second );
	aVector.push_back( aPropertySlotPtr->getName() );
      }

    return aVector;
  }

  const Polymorph PropertyInterface::
  getPropertyAttributes( StringCref aPropertyName ) const  
  {
    PropertySlotPtr aPropertySlotPtr( getPropertySlot( aPropertyName ) );

    PolymorphVector aVector;

    // is setable?
    aVector.push_back( static_cast<Int>
		       ( aPropertySlotPtr->isSetable() ) );

    // is getable?
    aVector.push_back( static_cast<Int>
		       ( aPropertySlotPtr->isGetable() ) );

    return aVector;
  }


  PropertyInterface::PropertyInterface()
  {
    makeSlots();
  }

  PropertyInterface::~PropertyInterface()
  {
    for( PropertySlotMapIterator i( thePropertySlotMap.begin() ); 
	 i != thePropertySlotMap.end() ; ++i )
      {
	delete i->second;
      }
  }

  void PropertyInterface::registerSlot( PropertySlotPtr slot )
  {
    String keyword = slot->getName();
    if( thePropertySlotMap.find( keyword ) != thePropertySlotMap.end() )
      {
	// it already exists. take the latter one.
	delete thePropertySlotMap[ keyword ];
	thePropertySlotMap.erase( keyword );
      }

    thePropertySlotMap[ keyword ] = slot;
  }

  void PropertyInterface::removeSlot( StringCref keyword )
  {
    if( thePropertySlotMap.find( keyword ) == thePropertySlotMap.end() )
      {
	THROW_EXCEPTION( NoSlot,
			 getClassName() + String( ":no slot for keyword [" ) +
			 keyword + String( "] found.\n" ) );
      }

    delete thePropertySlotMap[ keyword ];
    thePropertySlotMap.erase( keyword );
  }

  void PropertyInterface::setProperty( StringCref aPropertyName, 
				       PolymorphCref aValue )
  {
    PropertySlotMapConstIterator 
      aPropertySlotMapIterator( thePropertySlotMap.find( aPropertyName ) );

    if( aPropertySlotMapIterator == thePropertySlotMap.end() )
      {
	THROW_EXCEPTION( NoSlot,
			 getClassName() + 
			 String(": No property slot found with name [")
			 + aPropertyName + "].  Set property failed." );
      }

    aPropertySlotMapIterator->second->setPolymorph( aValue );
  }

  const Polymorph
  PropertyInterface::getProperty( StringCref aPropertyName ) const
  {
    PropertySlotMapConstIterator 
      aPropertySlotMapIterator( thePropertySlotMap.find( aPropertyName ) );

    if( aPropertySlotMapIterator == thePropertySlotMap.end() )
      {
	THROW_EXCEPTION( NoSlot, 
			 getClassName() + 
			 String(": No property slot found with name [")
			 + aPropertyName + "].  Get property failed." );
      }

    return aPropertySlotMapIterator->second->getPolymorph();
  }


} // namespace libecs


/*
  Do not modify
  $Author$
  $Revision$
  $Date$
  $Locker$
*/
