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


#include "Variable.hpp"

#include "Steppers.hpp"


namespace libecs
{


  ////////////////////////// FixedEuler1Stepper


  FixedEuler1Stepper::FixedEuler1Stepper()
  {
    ; // do nothing
  }

  void FixedEuler1Stepper::step()
  {
    const UnsignedInt aSize( theWriteVariableVector.size() );

    integrate();
    slave();

    processNegative();
    log();

    clear();
    processNormal();

    setStepInterval( getNextStepInterval() );

    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );

	theVelocityBuffer[ c ] = aVariable->getVelocity();

	// avoid negative value
	while( aVariable->checkRange( getStepInterval() ) == false )
	  {
	    // don't use setStepInterval()
	    loadStepInterval( getStepInterval() * 0.5 );
	  }
      }

    if( getStepInterval() < getTolerantStepInterval() )
      {
  	setNextStepInterval( getStepInterval() * 2.0 );
      }
    else 
      {
	setNextStepInterval( getStepInterval() );
      }
  }


  ////////////////////////// FixedRungeKutta4Stepper


  FixedRungeKutta4Stepper::FixedRungeKutta4Stepper()
  {
    ; // do nothing
  }

  void FixedRungeKutta4Stepper::step()
  {
    // integrate phase first
    integrate();

    slave();

    processNegative();

    log();

    // clear
    clear();

    // ========= 1 ===========
    processNormal();

    const UnsignedInt aSize( theWriteVariableVector.size() );
    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );

	// get k1
	Real aVelocity( aVariable->getVelocity() );

	// restore k1 / 2 + x
	aVariable->loadValue( aVelocity * .5 * getStepInterval()
			      + theValueBuffer[ c ] );

	theVelocityBuffer[ c ] = aVelocity;

	// clear velocity
	aVariable->setVelocity( 0 );
      }

    // ========= 2 ===========
    processNormal();

    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	const Real aVelocity( aVariable->getVelocity() );
	theVelocityBuffer[ c ] += aVelocity + aVelocity;

	// restore k2 / 2 + x
	aVariable->loadValue( aVelocity * .5 * getStepInterval()
			      + theValueBuffer[ c ] );


	// clear velocity
	aVariable->setVelocity( 0 );
      }

    // ========= 3 ===========
    processNormal();
    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	const Real aVelocity( aVariable->getVelocity() );
	theVelocityBuffer[ c ] += aVelocity + aVelocity;

	// restore k3 + x
	aVariable->loadValue( aVelocity * getStepInterval()
			      + theValueBuffer[ c ] );

	// clear velocity
	aVariable->setVelocity( 0 );
      }

    // ========= 4 ===========
    processNormal();

    // restore theValueBuffer
    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	const Real aVelocity( aVariable->getVelocity() );

	// restore x (original value)
	aVariable->loadValue( theValueBuffer[ c ] );

	//// x(n+1) = x(n) + 1/6 * (k1 + k4 + 2 * (k2 + k3)) + O(h^5)

	theVelocityBuffer[ c ] += aVelocity;
	theVelocityBuffer[ c ] *= ( 1.0 / 6.0 );
	aVariable->setVelocity( theVelocityBuffer[ c ] );
      }
  }


  ////////////////////////// Euler1Stepper


  Euler1Stepper::Euler1Stepper()
  {
    ; // do nothing
  }

  void Euler1Stepper::initialize()
  {
    DifferentialStepper::initialize();
  }

  void Euler1Stepper::step()
  {
    integrate();
    slave();

    processNegative();

    log();
    clear();

    setStepInterval( getNextStepInterval() );

    while( !calculate() )
      {
	; // do nothing
      }
  }

  bool Euler1Stepper::calculate()
  {
    const UnsignedInt aSize( theWriteVariableVector.size() );

    // don't expect too much from euler
    const Real eps_abs( getTorelance() );
    const Real eps_rel( getRelativeTorelance() * eps_abs );
    const Real a_y( getStateScalingFactor() );
    const Real a_dydt( getDerivativeScalingFactor() );

    // ========= 1 ===========
    processNormal();

    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	
	const Real aVelocity( aVariable->getVelocity() );

	// get k1
	theVelocityBuffer[ c ] = aVelocity;

	aVariable->loadValue( aVelocity * .5 * getStepInterval()
			      + theValueBuffer[ c ] );

	// clear velocity
	aVariable->setVelocity( 0 );
      }

    // ========= 2 ===========
    processNormal();

    Real maxError( 0.0 );
	
    // restore theValueBuffer
    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );

	// get k2 = f(x+h/2, y+k1*h/2)
	const Real aVelocity( aVariable->getVelocity() );

	const Real anErrorEstimate( theVelocityBuffer[ c ] );

	// ( k1 + k2 ) / 2 for ~Yn+1
	theVelocityBuffer[ c ] = 
	  ( theVelocityBuffer[ c ] + aVelocity ) * 0.5;

	const Real 
	  aTolerance( eps_rel * 
		      ( a_y * fabs( theValueBuffer[ c ] ) 
			+ a_dydt * fabs( theVelocityBuffer[ c ] ) )
		      + eps_abs );

	const Real
	  anError( fabs( ( theVelocityBuffer[ c ] 
			   - anErrorEstimate ) / aTolerance ) );

	if( anError > maxError )
	  {
	    maxError = anError;
	    
	    if( maxError > 1.1 )
	      {
		// shrink it if the error exceeds 110%
		//		    setStepInterval( getStepInterval() 
		//				     * pow(maxError, -1.0)
		//				     *  safety );
		
		setStepInterval( getStepInterval() * 0.5 );
		
		//		    std::cerr << "s " << getCurrentTime() 
		//			      << ' ' << getStepInterval() 
		//			      << std::endl;
		
		reset();

		return 0;
	      }
	  }
	
	// restore x (original value)
	aVariable->loadValue( theValueBuffer[ c ] );
	
	/// x(n+1) = x(n) + k2 * aStepInterval + O(h^3)
	aVariable->setVelocity( theVelocityBuffer[ c ] );
      }
    
    if( maxError < 0.5 )
      {
	// grow it if error is 50% less than desired
	//	    Real aNewStepInterval( getStepInterval() * 2.0 );
	
	Real aNewStepInterval( getStepInterval() 
			       * pow(maxError, -0.5) * safety );
	
	if( aNewStepInterval >= getUserMaxInterval() )
	  {
	    aNewStepInterval = getStepInterval();
	  }
	
	//	    	    std::cerr << "g " << getCurrentTime() << ' ' 
	//	    		      << aStepInterval << std::endl;
	
	setNextStepInterval( aNewStepInterval );
      }
    else
      {
	setNextStepInterval( getStepInterval() );
      }

    return 1;
  }


  ////////////////////////// Midpoint2Stepper


  Midpoint2Stepper::Midpoint2Stepper()
  {
    ; // do nothing
  }

  void Midpoint2Stepper::initialize()
  {
    DifferentialStepper::initialize();

    const UnsignedInt aSize( theWriteVariableVector.size() );

    theK1.resize( aSize );
  }

  void Midpoint2Stepper::step()
  {
    // integrate phase first
    integrate();

    slave();

    processNegative();

    log();

    // clear
    clear();

    setStepInterval( getNextStepInterval() );

    while( !calculate() )
      {
	; // do nothing
      }
  }

  bool Midpoint2Stepper::calculate()
  {
    const UnsignedInt aSize( theWriteVariableVector.size() );

    const Real eps_abs( getTorelance() );
    const Real eps_rel( getRelativeTorelance() * eps_abs );
    const Real a_y( getStateScalingFactor() );
    const Real a_dydt( getDerivativeScalingFactor() );

    // ========= 1 ===========
    processNormal();

    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	    
	// get k1
	const Real aVelocity( aVariable->getVelocity() );
	theK1[ c ] = aVelocity;
	
	// restore k1
	aVariable->loadValue( aVelocity * getStepInterval() 
			      + theValueBuffer[ c ] );

	// clear velocity
	aVariable->setVelocity( 0.0 );
      }

    // ========= 2 ===========
    processNormal();

    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	    
	// get k2
	const Real aVelocity( aVariable->getVelocity() );
	theVelocityBuffer[ c ] = aVelocity;
	    
	// restore (k1 + k2) / 2
	//	    aVariable->loadValue( ( aVelocity + aVelocity 
	//				    - theK1[ c ] ) * getStepInterval()
	//				  + theValueBuffer[ c ] );
	    
	aVariable->loadValue( ( aVelocity + theK1[ c ] ) * 0.25 
			      * getStepInterval() 
			      + theValueBuffer[ c ] );
	    
	// clear velocity
	aVariable->setVelocity( 0.0 );
      }
	
    // ========= 3 ===========
    processNormal();
	
    Real maxError( 0.0 );

    // restore theValueBuffer
    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	
	const Real aVelocity( aVariable->getVelocity() );
	
	// ( k1 + k2 + k3 * 4 ) / 6 for ~Yn+1
	const Real 
	  anErrorEstimate( ( theK1[ c ] 
			     + theVelocityBuffer[ c ] 
			     + aVelocity * 4.0 ) * ( 1.0 / 6.0 ) );

	const Real 
	  aTolerance( eps_rel *
		      ( a_y * fabs( theValueBuffer[ c ] ) 
			+  a_dydt * fabs( theVelocityBuffer[ c ] ) )
		      + eps_abs );

	const Real
	  anError( fabs( ( theVelocityBuffer[ c ] 
			   - anErrorEstimate ) / aTolerance ) );
	    
	if( anError > maxError )
	  {
	    maxError = anError;
	    
	    if( maxError > 1.1 )
	      {
		// shrink it if the error exceeds 110%
		//		    setStepInterval( getStepInterval() 
		//				     * pow(maxError, -0.5) 
		//				     * safety );

		setStepInterval( getStepInterval() * 0.5 );

		//		    std::cerr << "s " << getCurrentTime() 
		//			      << ' ' << getStepInterval()
		//			      << std::endl;

		reset();
		
		return 0;
	      }
	  }

	// restore x (original value)
	aVariable->loadValue( theValueBuffer[ c ] );

	//// x(n+1) = x(n) + k2 * aStepInterval + O(h^3)
	aVariable->setVelocity( theVelocityBuffer[ c ] );
      }

    // grow it if error is 50% less than desired
    if ( maxError < 0.5 )
      {
	Real aNewStepInterval( getStepInterval()
			       * pow(maxError , -0.5)
			       * safety );
	//  	    Real aNewStepInterval( getStepInterval() * 2.0 );

	if( aNewStepInterval >= getUserMaxInterval() )
	  {
	    aNewStepInterval = getStepInterval();
	  }

	//	    std::cerr << "g " << getCurrentTime() << ' ' 
	//		      << getStepInterval() << std::endl;
	setNextStepInterval( aNewStepInterval );
      }
    else 
      {
	setNextStepInterval( getStepInterval() );
      }
    
    return 1;
  }


  ////////////////////////// CashKarp4Stepper


  CashKarp4Stepper::CashKarp4Stepper()
  {
    ; // do nothing
  }

  void CashKarp4Stepper::initialize()
  {
    DifferentialStepper::initialize();

    const UnsignedInt aSize( theWriteVariableVector.size() );

    theK1.resize( aSize );
    theK2.resize( aSize );
    theK3.resize( aSize );
    theK4.resize( aSize );
    theK5.resize( aSize );
    theK6.resize( aSize );

    theErrorEstimate.resize( aSize );
  }
  
  void CashKarp4Stepper::step()
  {
    // integrate phase first
    integrate();

    slave();

    processNegative();

    log();

    // clear
    clear();

    setStepInterval( getNextStepInterval() );

    while( !calculate() )
      {
	; // do nothing
      }
  }

  bool CashKarp4Stepper::calculate()
  {
    const UnsignedInt aSize( theWriteVariableVector.size() );

    const Real eps_abs( getTorelance() );
    const Real eps_rel( getRelativeTorelance() * eps_abs );
    const Real a_y( getStateScalingFactor() );
    const Real a_dydt( getDerivativeScalingFactor() );

    // ========= 1 ===========
    processNormal();

    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	    
	// get k1
	theK1[ c ] = aVariable->getVelocity();

	// restore k1 / 5 + x
	aVariable->
	  loadValue( theK1[ c ] * .2  * getStepInterval()
		     + theValueBuffer[ c ] );

	// k1 * 37/378 for Yn+1
	theVelocityBuffer[ c ] = theK1[ c ] * ( 37.0 / 378.0 );
	// k1 * 2825/27648 for ~Yn+1
	theErrorEstimate[ c ] = theK1[ c ] * ( 2825.0 / 27648.0 );

	// clear velocity
	aVariable->setVelocity( 0 );
      }

    // ========= 2 ===========
    processNormal();

    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	
	theK2[ c ] = aVariable->getVelocity();
	    
	// restore k1 * 3/40+ k2 * 9/40 + x
	aVariable->
	  loadValue( theK1[ c ] * ( 3.0 / 40.0 ) * getStepInterval()
		     + theK2[ c ] * ( 9.0 / 40.0 ) * getStepInterval()
		     + theValueBuffer[ c ] );
	    
	// k2 * 0 for Yn+1 (do nothing)
	//	    theVelocityBuffer[ c ] = theK2[ c ] * 0;
	// k2 * 0 for ~Yn+1 (do nothing)
	//	    theErrorEstimate[ c ] = theK2[ c ] * 0;
	    
	// clear velocity
	aVariable->setVelocity( 0 );
      }
	
    // ========= 3 ===========
    processNormal();
	
    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	
	theK3[ c ] = aVariable->getVelocity();
	
	// restore k1 * 3/10 - k2 * 9/10 + k3 * 6/5 + x
	aVariable->
	  loadValue( theK1[ c ] * ( 3.0 / 10.0 ) * getStepInterval()
		     - theK2[ c ] * ( 9.0 / 10.0 ) * getStepInterval()
		     + theK3[ c ] * ( 6.0 / 5.0 ) * getStepInterval()
		     + theValueBuffer[ c ] );
	
	// k3 * 250/621 for Yn+1
	theVelocityBuffer[ c ] += theK3[ c ] * ( 250.0 / 621.0 );
	// k3 * 18575/48384 for ~Yn+1
	theErrorEstimate[ c ] += theK3[ c ] * ( 18575.0 / 48384.0 );
	
	// clear velocity
	aVariable->setVelocity( 0 );
      }
    
    // ========= 4 ===========
    processNormal();
    
    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	
	theK4[ c ] = aVariable->getVelocity();
	
	// restore k2 * 5/2 - k1 * 11/54 - k3 * 70/27 + k4 * 35/27 + x
	aVariable->
	  loadValue( theK2[ c ] * ( 5.0 / 2.0 ) * getStepInterval()
		     - theK1[ c ] * ( 11.0 / 54.0 ) * getStepInterval()
		     - theK3[ c ] * ( 70.0 / 27.0 ) * getStepInterval()
		     + theK4[ c ] * ( 35.0 / 27.0 ) * getStepInterval()
		     + theValueBuffer[ c ] );
	
	// k4 * 125/594 for Yn+1
	theVelocityBuffer[ c ] += theK4[ c ] * ( 125.0 / 594.0 );
	// k4 * 13525/55296 for ~Yn+1
	theErrorEstimate[ c ] += theK4[ c ] * ( 13525.0 / 55296.0 );
	    
	// clear velocity
	aVariable->setVelocity( 0 );
      }
		
    // ========= 5 ===========
    processNormal();
	
    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	
	theK5[ c ] = aVariable->getVelocity();
	    
	// restore k1 * 1631/55296 
	//         + k2 * 175/512 
	//         + k3 * 575/13824
	//         + k4 * 44275/110592
	//         + k5 * 253/4096
	aVariable->
	  loadValue( ( theK1[ c ] * ( 1631.0 / 55296.0 )
		       + theK2[ c ] * ( 175.0 / 512.0 )
		       + theK3[ c ] * ( 575.0 / 13824.0 )
		       + theK4[ c ] * ( 44275.0 / 110592.0 )
		       + theK5[ c ] * ( 253.0 / 4096.0 ) )
		     * getStepInterval() + theValueBuffer[ c ] );
	    
	// k5 * 0 for Yn+1(do nothing)
	//	    theVelocityBuffer[ c ] += theK5[ c ] * 0;
	// k5 * 277/14336 for ~Yn+1
	theErrorEstimate[ c ] += theK5[ c ] * ( 277.0 / 14336.0 );
	    
	// clear velocity
	aVariable->setVelocity( 0 );
      }

    // ========= 6 ===========
    processNormal();
	
    Real maxError( 0.0 );
    
    // restore theValueBuffer
    for( UnsignedInt c( 0 ); c < aSize; ++c )
      {
	VariablePtr const aVariable( theWriteVariableVector[ c ] );
	
	theK6[ c ] = aVariable->getVelocity();

	// k6 * 512/1771 for Yn+1
	theVelocityBuffer[ c ] += theK6[ c ] * ( 512.0 / 1771.0 );
	// k6 * 1/4 for ~Yn+1
	theErrorEstimate[ c ] += theK6[ c ] * .25;

	const Real 
	  aTolerance( eps_rel * 
		      ( a_y * fabs( theValueBuffer[ c ] ) 
			+ a_dydt * fabs( theVelocityBuffer[ c ] ) )
		      + eps_abs );
	    
	const Real 
	  anError( fabs( ( theVelocityBuffer[ c ] 
			   - theErrorEstimate[ c ] ) / aTolerance ) );
	    
	if( anError > maxError )
	  {
	    maxError = anError;
	    
	    if( maxError > 1.1 )
	      {
		// shrink it if the error exceeds 110%
		//		    setStepInterval( getStepInterval() 
		//				     * pow(maxError, -0.20)
		//				     *  safety );
		setStepInterval( getStepInterval() * 0.5 );

		reset();

		return 0;
	      }
	  }

	// restore x (original value)
	aVariable->loadValue( theValueBuffer[ c ] );

	//// k1 * 37/378 + k3 * 250/621 + k4 * 125/594 + k6 * 512/1771)
	aVariable->setVelocity( theVelocityBuffer[ c ] );
      }

    // grow it if error is 50% less than desired
    if (maxError <= 0.5)
      {
	Real aNewStepInterval( getStepInterval() 
			       * pow(maxError , -0.25) * safety );
	    
	if( aNewStepInterval >= getUserMaxInterval() )
	  {
	    aNewStepInterval = getStepInterval();
	  }
	    	    
	setNextStepInterval( aNewStepInterval );
      }
    else 
      {
	setNextStepInterval( getStepInterval() );
      }

    return 1;
  }


} // namespace libecs


/*
  Do not modify
  $Author$
  $Revision$
  $Date$
  $Locker$
*/
