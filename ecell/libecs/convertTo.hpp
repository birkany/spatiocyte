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

#ifndef __CONVERTTO_HPP
#define __CONVERTTO_HPP

#include "libecs.hpp"
#include "Util.hpp"

//#include "Polymorph.hpp"

namespace libecs
{


  /** @addtogroup property
      
  @ingroup libecs
  @{
  */

  template< typename ToType, typename FromType >
  inline const ToType convertTo( const FromType& aValue )
  {
    return convertTo( aValue, Type2Type<ToType>() );
  }

  template< typename ToType, typename FromType >
  const ToType convertTo( const FromType& aValue, Type2Type<ToType> )
  {
    return static_cast<ToType>( aValue );
    // DefaultSpecializationProhibited();
  }


  // specializations


  // to String

  // identity
  template<>
  inline const String
  convertTo( StringCref aValue,
	     Type2Type< String > )
  {
    return aValue;
  }

  template<>
  inline const String convertTo( RealCref aValue,
				 Type2Type< String > )
  {
    return toString( aValue );
  }

  template<>
  inline const String convertTo( IntegerCref aValue,
				 Type2Type< String > )
  {
    return toString( aValue );
  }


  // to Real


  // identity
  template<>
  inline const Real
  convertTo( RealCref aValue,
	     Type2Type< Real > )
  {
    return aValue;
  }

  template<>
  inline const Real convertTo( StringCref aValue,
			       Type2Type< Real > )
  {
    return stringTo< Real >( aValue );
  }

  template<>
  inline const Real convertTo( IntegerCref aValue,
			       Type2Type< Real > )
  {
    return static_cast< const Real >( aValue );
  }



  // to Integer


  // identity
  template<>
  inline const Integer
  convertTo( IntegerCref aValue,
	     Type2Type< Integer > )
  {
    return aValue;
  }

  template<>
  inline const Integer convertTo( RealCref aValue,
			      Type2Type< Integer > )
  {
    return static_cast< const Integer >( aValue );
  }

  template<>
  inline const Integer convertTo( StringCref aValue,
				  Type2Type< Integer > )
  {
    return stringTo< Integer >( aValue );
  }


}


#endif /* __CONVERTTO_HPP */
