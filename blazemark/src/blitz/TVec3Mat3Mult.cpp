//=================================================================================================
/*!
//  \file src/blitz/TVec3Mat3Mult.cpp
//  \brief Source file for the Blitz++ 3D transpose vector/matrix multiplication kernel
//
//  Copyright (C) 2011 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. This library is free software; you can redistribute
//  it and/or modify it under the terms of the GNU General Public License as published by the
//  Free Software Foundation; either version 3, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with a special
//  exception for linking and compiling against the Blaze library, the so-called "runtime
//  exception"; see the file COPYING. If not, see http://www.gnu.org/licenses/.
*/
//=================================================================================================


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <iostream>
#include <vector>
#include <blitz/array.h>
#include <blaze/util/Timing.h>
#include <blazemark/blitz/init/Array.h>
#include <blazemark/blitz/TVec3Mat3Mult.h>
#include <blazemark/system/Config.h>


namespace blazemark {

namespace blitz {

//=================================================================================================
//
//  KERNEL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Blitz++ 3-dimensional transpose vector/matrix multiplication kernel.
//
// \param N The number of 3D vectors to be computed.
// \param steps The number of iteration steps to perform.
// \return Minimum runtime of the kernel function.
//
// This kernel function implements the 3-dimensional transpose vector/matrix multiplication by
// means of the Blitz++ functionality.
*/
double tvec3mat3mult( size_t N, size_t steps )
{
   using ::blazemark::element_t;

   ::blaze::setSeed( seed );

   ::std::vector< ::blitz::Array<element_t,1> > a( N ), b( N );
   ::std::vector< ::blitz::Array<element_t,2> > A( N );
   ::blitz::firstIndex i;
   ::blitz::secondIndex j;
   ::blaze::timing::WcTimer timer;

   for( size_t l=0UL; l<N; ++l ) {
      a[l].resize( 3 );
      b[l].resize( 3 );
      A[l].resize( 3, 3 );
      init( a[l] );
      initRowMajorMatrix( A[l] );
   }

   for( size_t l=0UL; l<N; ++l ) {
      b[l] = sum( a[l](j) * A[l](j,i), j );
   }

   for( size_t rep=0UL; rep<reps; ++rep )
   {
      timer.start();
      for( size_t step=0UL, l=0UL; step<steps; ++step, ++l ) {
         if( l == N ) l = 0UL;
         b[l] = sum( a[l](j) * A[l](j,i), j );
      }
      timer.end();

      for( size_t l=0UL; l<N; ++l )
         if( b[l](0) < element_t(0) )
            std::cerr << " Line " << __LINE__ << ": ERROR detected!!!\n";

      if( timer.last() > maxtime )
         break;
   }

   const double minTime( timer.min()     );
   const double avgTime( timer.average() );

   if( minTime * ( 1.0 + deviation*0.01 ) < avgTime )
      std::cerr << " Blitz++ kernel 'tvec3mat3mult': Time deviation too large!!!\n";

   return minTime;
}
//*************************************************************************************************

} // namespace blitz

} // namespace blazemark
