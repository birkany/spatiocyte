//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//        This file is part of E-CELL Simulation Environment package
//
//                Copyright (C) 2002 Keio University
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

#ifndef __CASHKARP45_HPP
#define __CASHKARP45_HPP


// #include <iostream>

#include "libecs/Stepper.hpp"

USE_LIBECS;

// DECLARE_VECTOR( Real, RealVector );

// DECLARE_CLASS( CashKarp45Stepper );

class CashKarp45Stepper 
  : 
  public AdaptiveDifferentialStepper
{

  LIBECS_DM_OBJECT( Stepper, CashKarp45Stepper );

public:

  CashKarp45Stepper( void );
  
  virtual ~CashKarp45Stepper( void );

  virtual void initialize();
  virtual bool calculate();

  virtual const Int getOrder() const { return 4; }

protected:

  //  RealVector theK1;
  RealVector theK2;
  RealVector theK3;
  RealVector theK4;
  RealVector theK5;
  RealVector theK6;

  RealVector theErrorEstimate;

};

#endif /* __CASHKARP45_HPP */