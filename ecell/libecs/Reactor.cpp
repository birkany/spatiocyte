//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//        This file is part of E-CELL Simulation Environment package
//
//                Copyright (C) 1996-2000 Keio University
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

#include "Util.hpp"
#include "Reactant.hpp"
#include "RootSystem.hpp"
#include "Stepper.hpp"
#include "FQPI.hpp"
#include "Substance.hpp"

#include "Reactor.hpp"

Reactor::Condition Reactor::theGlobalCondition;// = Reactor::Condition::Good;

void Reactor::makeSlots()
{
  //FIXME: get methods
  MessageSlot( "AppendSubstrate",Reactor,*this,&Reactor::setAppendSubstrate,
	       NULL );
  MessageSlot( "AppendProduct",Reactor,*this,&Reactor::setAppendProduct,
	       NULL );
  MessageSlot( "AppendCatalyst",Reactor,*this,&Reactor::setAppendCatalyst,
	       NULL );
  MessageSlot( "AppendEffector",Reactor,*this,&Reactor::setAppendEffector,
	       NULL );

  MessageSlot( "SubstrateList",Reactor,*this,&Reactor::setSubstrateList,
	       &Reactor::getSubstrateList);
  MessageSlot( "ProductList",Reactor,*this,&Reactor::setProductList,
	       &Reactor::getProductList);
  MessageSlot( "CatalystList",Reactor,*this,&Reactor::setCatalystList,
	       &Reactor::getCatalystList);
  MessageSlot( "EffectorList",Reactor,*this,&Reactor::setEffectorList,
	       &Reactor::getEffectorList);

  MessageSlot( "InitialActivity",Reactor,*this,&Reactor::setInitialActivity,
	       &Reactor::getInitialActivity );
}

void Reactor::setAppendSubstrate( MessageCref message )
{
  //FIXME: range check
  appendSubstrate( message[0].asString(), message[1].asInt() );
}

void Reactor::setAppendProduct( MessageCref message )
{
  //FIXME: range check
  appendSubstrate( message[0].asString(), message[1].asInt() );
}

void Reactor::setAppendCatalyst( MessageCref message )
{
  //FIXME: range check
  appendSubstrate( message[0].asString(), message[1].asInt() );
}

void Reactor::setAppendEffector( MessageCref message )
{
  //FIXME: range check
  appendSubstrate( message[0].asString(), message[1].asInt() );
}

void Reactor::setSubstrateList( MessageCref message )
{
  cerr << "not implemented yet." << endl;
}

void Reactor::setProductList( MessageCref message )
{
  cerr << "not implemented yet." << endl;
}

void Reactor::setEffectorList( MessageCref message )
{
  cerr << "not implemented yet." << endl;
}

void Reactor::setCatalystList( MessageCref message )
{
  cerr << "not implemented yet." << endl;
}

const Message Reactor::getSubstrateList( StringCref keyword )
{
  UniversalVariableVector aList;
  
  for( ReactantVectorConstIterator i = theSubstrateList.begin() ;
       i != theSubstrateList.end() ; ++i )
    {
      aList.push_back( (*i)->getSubstance().getFqid() );
    }

  return Message( keyword, aList );
}

const Message Reactor::getProductList( StringCref keyword )
{
  UniversalVariableVector aList;
  
  for( ReactantVectorConstIterator i = theProductList.begin() ;
       i != theProductList.end() ; ++i )
    {
      aList.push_back( (*i)->getSubstance().getFqid() );
    }

  return Message( keyword, aList );
}

const Message Reactor::getEffectorList( StringCref keyword )
{
  UniversalVariableVector aList;
  
  for( ReactantVectorConstIterator i = theEffectorList.begin() ;
       i != theEffectorList.end() ; ++i )
    {
      aList.push_back( (*i)->getSubstance().getFqid() );
    }

  return Message( keyword, aList );
}

const Message Reactor::getCatalystList( StringCref keyword )
{
  UniversalVariableVector aList;
  
  for( ReactantVectorConstIterator i = theCatalystList.begin() ;
       i != theCatalystList.end() ; ++i )
    {
      aList.push_back( (*i)->getSubstance().getFqid() );
    }

  return Message( keyword, aList );
}

void Reactor::setInitialActivity( MessageCref message )
{
  setInitialActivity( message[0].asFloat() );
}

const Message Reactor::getInitialActivity( StringCref keyword )
{
  return Message( keyword, UniversalVariable( theInitialActivity ) );
}

void Reactor::appendSubstrate( FQIDCref fqid, int coefficient )
{
  SubstancePtr aSubstance( getSuperSystem()->getRootSystem()->
			   getSubstance( fqid ) );

  appendSubstrate( *aSubstance, coefficient );
}

void Reactor::appendProduct( FQIDCref fqid, int coefficient )
{
  SubstancePtr aSubstance( getSuperSystem()->getRootSystem()->
			   getSubstance( fqid ) );
  
  appendProduct( *aSubstance, coefficient );
}

void Reactor::appendCatalyst( FQIDCref fqid,int coefficient)
{
  SubstancePtr aSubstance( getSuperSystem()->getRootSystem()->
			   getSubstance( fqid ) );
  
  appendCatalyst( *aSubstance, coefficient );
}

void Reactor::appendEffector( FQIDCref fqid, int coefficient )
{
  SubstancePtr aSubstance( getSuperSystem()->getRootSystem()->
			   getSubstance( fqid ) );
  
  appendEffector( *aSubstance, coefficient );
}

void Reactor::setInitialActivity( Float activity )
{
  theInitialActivity = activity;
  // FIXME: take delta T from supersystem
  theActivity= activity * 
    getSuperSystem()->getStepper()->getDeltaT();
}

Reactor::Reactor() 
  :
  theInitialActivity( 0 ),
  theActivityBuffer( 0 ),
  theCondition( Premature ),
  theActivity( 0 )
{
  makeSlots();
}

const String Reactor::getFqpi() const
{
  return PrimitiveTypeStringOf( *this ) + ":" + getFqid();
}

void Reactor::appendSubstrate( SubstanceRef substrate, int coefficient )
{
  ReactantPtr reactant = new Reactant( substrate, coefficient );
  theSubstrateList.insert( theSubstrateList.end(), reactant );
}

void Reactor::appendProduct( SubstanceRef product, int coefficient )
{
  ReactantPtr reactant = new Reactant( product, coefficient );
  theProductList.insert( theProductList.end(), reactant );
}

void Reactor::appendCatalyst( SubstanceRef catalyst, int coefficient )
{
  ReactantPtr reactant = new Reactant( catalyst, coefficient );
  theCatalystList.insert( theCatalystList.end(), reactant );
}

void Reactor::appendEffector( SubstanceRef effector, int coefficient )
{
  ReactantPtr reactant = new Reactant( effector, coefficient );
  theEffectorList.insert( theEffectorList.end(), reactant );
}

Reactor::Condition Reactor::condition( Condition condition )
{
  theCondition = static_cast<Condition>( theCondition | condition );
  if( theCondition  != Good )
    return theGlobalCondition = Bad;
  return Good;
}

void Reactor::warning( StringCref message )
{
//FIXME:   *theMessageWindow << className() << " [" << fqen() << "]";
//FIXME:   *theMessageWindow << ":\n\t" << message << "\n";
}

void Reactor::initialize()
{
  if( getNumberOfSubstrates() > getMaximumNumberOfSubstrates() )
    warning("too many substrates.");
  else if( getNumberOfSubstrates() < getMinimumNumberOfSubstrates() )
    warning("too few substrates.");
  if( getNumberOfProducts() > getMaximumNumberOfProducts() )
    warning("too many products.");
  else if( getNumberOfProducts() < getMinimumNumberOfProducts() )
    warning("too few products.");
  if( getNumberOfCatalysts() > getMaximumNumberOfCatalysts() )
    warning("too many catalysts.");
  else if( getNumberOfCatalysts() < getMinimumNumberOfCatalysts() )
    warning("too few catalysts.");
  if( getNumberOfEffectors() > getMaximumNumberOfEffectors() )
    warning("too many effectors.");
  else if( getNumberOfEffectors() < getMinimumNumberOfEffectors() )
    warning("too few effectors.");
}


Float Reactor::getActivity() 
{
  return theActivity;
}


/*
  Do not modify
  $Author$
  $Revision$
  $Date$
  $Locker$
*/
