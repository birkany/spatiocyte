//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//        This file is part of E-Cell Simulation Environment package
//
//                Copyright (C) 1996-2002 Keio University
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//
// E-Cell is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// E-Cell is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with E-Cell -- see the file COPYING.
// If not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// 
//END_HEADER
//
// written by Kouichi Takahashi <shafi@e-cell.org>,
// E-Cell Project, Institute for Advanced Biosciences, Keio University.
//

#include <cmath>

#include "Stepper.hpp"

#include "Model.hpp"

namespace libecs
{

  Scheduler::Scheduler()
    :
    theCurrentTime( 0.0 )
  {
    ; // do nothing
  }

  Scheduler::~Scheduler()
  {
    ; // do nothing
  }

 
  void Scheduler::registerStepper( StepperPtr aStepper )
  {
    const int anIndex( registerEvent( SchedulerEvent( aStepper->
						      getCurrentTime(),
						      aStepper ) ) );

    aStepper->setSchedulerIndex( anIndex );
  }

  void Scheduler::reset()
  {
    theScheduleQueue.clear();
    theCurrentTime = 0.0;
  }


  void Scheduler::step()
  {
    SchedulerEventCref aTopEvent( theScheduleQueue.top() );
    const Time aCurrentTime( aTopEvent.getTime() );
    StepperPtr const aStepperPtr( aTopEvent.getStepper() );

    setCurrentTime( aCurrentTime );

    // this check is necessary, but check the impact on performance
    // before commenting out.
    //    if( ! std::isfinite( aCurrentTime ) )
    //      {
    //	THROW_EXCEPTION( SimulationError, "Current time has reached [" 
    //			 + stringCast( aCurrentTime ) + "].");
    //      }

    aStepperPtr->integrate( aCurrentTime );
    aStepperPtr->step();

    // Use higher precision for this procedure:
    const Time aStepInterval( aStepperPtr->getStepInterval() );
    const Time aScheduledTime( aCurrentTime + aStepInterval );

    // schedule a new event
    theScheduleQueue.changeTopKey( SchedulerEvent( aScheduledTime, 
						   aStepperPtr ) );
    aStepperPtr->dispatchInterruptions();
    aStepperPtr->log();


    /*
    if( aCurrentTime == aScheduledTime && aStepInterval > 0.0 )
      {
	THROW_EXCEPTION( SimulationError, 
			 "Too small step interval given by Stepper [" +
			 aStepperPtr->getID() + "]." );
      }
    */

   }


  void Scheduler::reschedule( StepperPtr const aStepperPtr )
  {
    // Use higher precision for this addition.
    const Time 
      aScheduledTime( static_cast<Time>( aStepperPtr->getCurrentTime() ) + 
		      static_cast<Time>( aStepperPtr->getStepInterval() ) );

    DEBUG_EXCEPTION( aScheduledTime + std::numeric_limits<Real>::epsilon()
		     >= getCurrentTime(),
     		     SimulationError,
     		     "Attempt to go past by Stepper [" +
		     aStepperPtr->getID() + "].  (Scheduled time = " +
		     stringCast( aScheduledTime ) + ", current time = " 
		     + stringCast( getCurrentTime() ) + ".)" );

    theScheduleQueue.changeOneKey( aStepperPtr->getSchedulerIndex(),
				   SchedulerEvent( aScheduledTime, 
						   aStepperPtr ) );
  }


} // namespace libecs





/*
  Do not modify
  $Author$
  $Revision$
  $Date$
  $Locker$
*/
