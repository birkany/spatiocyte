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

#include <limits>

#include "Util.hpp"
#include "Variable.hpp"
#include "VariableProxy.hpp"
#include "Process.hpp"
#include "Model.hpp"
#include "PropertySlotMaker.hpp"

#include "DifferentialStepper.hpp"


namespace libecs
{


  DifferentialStepper::DifferentialStepper()
    :
    theTolerantStepInterval( 0.001 ),
    theNextStepInterval( 0.001 )
  {
    CREATE_PROPERTYSLOT( Real, StepInterval, 
			 &DifferentialStepper::initializeStepInterval,
			 &DifferentialStepper::getStepInterval );

    CREATE_PROPERTYSLOT_GET( Real, NextStepInterval, DifferentialStepper );
  }

  void DifferentialStepper::initialize()
  {
    Stepper::initialize();

    // size of the velocity buffer == the number of *write* variables.
    theVelocityBuffer.resize( getReadOnlyVariableOffset() );

    // should create another method for property slot ?
    //    setNextStepInterval( getStepInterval() );
  }


  void DifferentialStepper::reset()
  {
    // clear velocity buffer
    theVelocityBuffer.assign( theVelocityBuffer.size(), 0.0 );

    Stepper::reset();
  }



  #if 0
  const bool DifferentialStepper::checkExternalError() const
  {
    const Real aSize( theValueVector.size() );
    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {

	// no!! theValueVector and theVariableVector holds completely
	// different sets of Variables
	const Real anOriginalValue( theValueVector[ c ] + 
				    numeric_limit<Real>::epsilon() );

	const Real aCurrentValue( theVariableVector[ c ]->getValue() );

	const Real anRelativeError( fabs( aCurrentValue - anOriginalValue ) 
				    / anOriginalValue );
	if( anRelativeError <= aTolerance )
	  {
	    continue;
	  }

	return false;
      }

    return true;
  }
#endif /* 0 */

  void DifferentialStepper::interrupt( StepperPtr const aCaller )
  {
    const Real aCallerTimeScale( aCaller->getTimeScale() );
    const Real aStepInterval   ( getStepInterval() );

    // If the step size of this is less than caller's timescale,
    // ignore this interruption.
    if( aCallerTimeScale >= aStepInterval )
      {
	return;
      }

    // if all Variables didn't change its value more than 10%,
    // ignore this interruption.
    /*  !!! currently this is disabled
    if( checkExternalError() )
      {
	return;
      }
    */
	
    const Real aCurrentTime      ( getCurrentTime() );
    const Real aCallerCurrentTime( aCaller->getCurrentTime() );

    // aCallerTimeScale == 0 implies need for immediate reset
    if( aCallerTimeScale != 0.0 )
      {
	// Shrink the next step size to that of caller's
	setNextStepInterval( aCallerTimeScale );

	const Real aNextStep      ( aCurrentTime + aStepInterval );
	const Real aCallerNextStep( aCallerCurrentTime + aCallerTimeScale );

	// If the next step of this occurs *before* the next step 
	// of the caller, just shrink step size of this Stepper.
	if( aNextStep <= aCallerNextStep )
	  {
	    //std::cerr << aCurrentTime << " return" << std::endl;

	    return;
	  }

	//	std::cerr << aCurrentTime << " noreset" << std::endl;

	
	// If the next step of this will occur *after* the caller,
	// reschedule this Stepper, as well as shrinking the next step size.
	//    setStepInterval( aCallerCurrentTime + ( aCallerTimeScale * 0.5 ) 
	//		     - aCurrentTime );
      }
    else
      {
	// reset step interval to the default
	//	std::cerr << aCurrentTime << " reset" << std::endl;

	setNextStepInterval( 0.001 );
      }
      
    setStepInterval( aCallerCurrentTime - aCurrentTime );
    getModel()->reschedule( this );
  }


  ////////////////////////// AdaptiveDifferentialStepper

  AdaptiveDifferentialStepper::AdaptiveDifferentialStepper()
    :
    theTolerance( 1.0e-6 ),
    theAbsoluteToleranceFactor( 1.0 ),
    theStateToleranceFactor( 1.0 ),
    theDerivativeToleranceFactor( 1.0 ),
    theAbsoluteEpsilon( 0.1 ),
    theRelativeEpsilon( 0.1 ),
    safety( 0.9 ),
    theMaxErrorRatio( 1.0 )
  {
    CREATE_PROPERTYSLOT_SET_GET( Real, Tolerance, 
				 AdaptiveDifferentialStepper );
    CREATE_PROPERTYSLOT_SET_GET( Real, AbsoluteToleranceFactor, 
				 AdaptiveDifferentialStepper );
    CREATE_PROPERTYSLOT_SET_GET( Real, StateToleranceFactor, 
				 AdaptiveDifferentialStepper );
    CREATE_PROPERTYSLOT_SET_GET( Real, DerivativeToleranceFactor, 
				 AdaptiveDifferentialStepper );
    CREATE_PROPERTYSLOT_SET_GET( Real, AbsoluteEpsilon, 
				 AdaptiveDifferentialStepper );
    CREATE_PROPERTYSLOT_SET_GET( Real, RelativeEpsilon, 
				 AdaptiveDifferentialStepper );
    CREATE_PROPERTYSLOT_GET    ( Real, MaxErrorRatio, 
				 AdaptiveDifferentialStepper );
    CREATE_PROPERTYSLOT_GET    ( Int,  Order, AdaptiveDifferentialStepper );

  }

  void AdaptiveDifferentialStepper::initialize()
  {
    DifferentialStepper::initialize();
  }

  void AdaptiveDifferentialStepper::step()
  {
    // clear
    clear();

    setStepInterval( getNextStepInterval() );

    while( !calculate() )
      {
	const Real
	  anExpectedStepInterval( getStepInterval() 
				  * pow( getMaxErrorRatio(), 
					 -1.0 / getOrder() ) 
				  * safety );

	if ( anExpectedStepInterval > getMinStepInterval() )
	  {
	    // shrink it if the error exceeds 110%
	    setStepInterval( anExpectedStepInterval );

	    //	setStepInterval( getStepInterval() * 0.5 );

	    //	std::cerr << "s " << getCurrentTime() 
	    //		  << ' ' << getStepInterval()
	    //		  << std::endl;
	  }
	else
	  {
	    setStepInterval( getMinStepInterval() );
	    
	    // this must return false,
	    // so theOriginalStepInterval does NOT LIMIT the error.
	    calculate();
	    setOriginalStepInterval( getMinStepInterval() );
	    break;
	  }
      }

    if ( getOriginalStepInterval() < getMinStepInterval() )
      {
	THROW_EXCEPTION( SimulationError,
			 "The error-limit step interval of Stepper [" + 
			 getID() + "] is too small." );
      }

    const Real anAdaptedStepInterval( getStepInterval() );
    const UnsignedInt aSize( getReadOnlyVariableOffset() );

    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theVariableVector[ c ] );

	const Real aTolerance( fabs( aVariable->getValue() ) 
			       * theRelativeEpsilon
			       + theAbsoluteEpsilon );

	const Real aVelocity( fabs( theVelocityBuffer[ c ] ) );

	if ( aTolerance < aVelocity * getStepInterval() )
	  {
	    setStepInterval( aTolerance / aVelocity );
	  }
      }

    const Real maxError( getMaxErrorRatio() );

    // grow it if error is 50% less than desired
    if ( maxError < 0.5 )
      {
	Real aNewStepInterval( getStepInterval() 
			       * pow(maxError , -1.0 / ( getOrder() + 1 ) )
			       * safety );

	//	Real aNewStepInterval( getStepInterval() * 2.0 );

	//	std::cerr << "g " << getCurrentTime() << ' ' 
	//		  << getStepInterval() << std::endl;

	setNextStepInterval( aNewStepInterval );
      }
    else 
      {
	setNextStepInterval( getStepInterval() );
      }
  }


} // namespace libecs


/*
  Do not modify
  $Author$
  $Revision$
  $Date$
  $Locker$
*/
