//=================================================================================================
/*!
//  \file src/blaze/SVecSVecMult.cpp
//  \brief Source file for the sparse vector/scalar multiplication benchmark
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

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <blaze/math/CompressedVector.h>
#include <blaze/math/Functions.h>
#include <blaze/math/Infinity.h>
#include <blaze/util/Random.h>
#include <blaze/util/Timing.h>
#include <blazemark/blaze/init/CompressedVector.h>
#include <blazemark/blaze/SVecScalarMult.h>
#include <blazemark/boost/SVecScalarMult.h>
#include <blazemark/gmm/SVecScalarMult.h>
#include <blazemark/system/Config.h>
#include <blazemark/system/GMM.h>
#include <blazemark/system/Types.h>
#include <blazemark/util/Benchmarks.h>
#include <blazemark/util/DynamicSparseRun.h>
#include <blazemark/util/Indices.h>
#include <blazemark/util/Parser.h>


//*************************************************************************************************
// Using declarations
//*************************************************************************************************

using blazemark::Benchmarks;
using blazemark::DynamicSparseRun;
using blazemark::Parser;




//=================================================================================================
//
//  TYPE DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Type of a benchmark run.
//
// This type definition specifies the type of a single benchmark run for the sparse
// vector/scalar multiplication benchmark.
*/
typedef DynamicSparseRun  Run;
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Estimating the necessary number of steps for each benchmark.
//
// \param run The parameters for the benchmark run.
// \return void
//
// This function estimates the necessary number of steps for the given benchmark based on the
// performance of the Blaze library.
*/
void estimateSteps( Run& run )
{
   using blazemark::element_t;
   using blaze::columnVector;

   ::blaze::setSeed( ::blazemark::seed );

   const size_t N( run.getSize() );
   const size_t F( run.getNonZeros() );

   blaze::CompressedVector<element_t,columnVector> a( N, F ), b( N );
   blaze::timing::WcTimer timer;
   double wct( 0.0 );
   size_t steps( 1UL );

   blazemark::blaze::init( a, F );

   while( true ) {
      timer.start();
      for( size_t i=0UL; i<steps; ++i ) {
         b = a * element_t(3);
      }
      timer.end();
      wct = timer.last();
      if( wct >= 0.2 ) break;
      steps *= 2UL;
   }

   if( b.size() != N )
      std::cerr << " Line " << __LINE__ << ": ERROR detected!!!\n";

   run.setSteps( blaze::max( 1UL, ( blazemark::runtime * steps ) / timer.last() ) );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Estimating the necessary number of floating point operations.
//
// \param run The parameters for the benchmark run.
// \return void
//
// This function estimates the number of floating point operations required for a single
// computation of the (composite) arithmetic operation.
*/
void estimateFlops( Run& run )
{
   const size_t F( run.getNonZeros() );

   run.setFlops( F );
}
//*************************************************************************************************




//=================================================================================================
//
//  BENCHMARK FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Sparse vector/scalar multiplication benchmark function.
//
// \param runs The specified benchmark runs.
// \param benchmarks The selection of benchmarks.
// \return void
*/
void svecscalarmult( std::vector<Run>& runs, Benchmarks benchmarks )
{
   std::cout << std::left;

   std::sort( runs.begin(), runs.end() );

   size_t slowSize( blaze::inf );
   for( std::vector<Run>::iterator run=runs.begin(); run!=runs.end(); ++run )
   {
      estimateFlops( *run );

      if( run->getSteps() == 0UL ) {
         if( run->getSize() < slowSize ) {
            estimateSteps( *run );
            if( run->getSteps() == 1UL )
               slowSize = run->getSize();
         }
         else run->setSteps( 1UL );
      }
   }

   if( benchmarks.runBlaze ) {
      std::vector<Run>::iterator run=runs.begin();
      while( run != runs.end() ) {
         const float fill( run->getFillingDegree() );
         std::cout << "   Blaze (" << fill << "% filled) [MFlop/s]:\n";
         for( ; run!=runs.end(); ++run ) {
            if( run->getFillingDegree() != fill ) break;
            const size_t N    ( run->getSize()     );
            const size_t F    ( run->getNonZeros() );
            const size_t steps( run->getSteps()    );
            run->setBlazeResult( blazemark::blaze::svecscalarmult( N, F, steps ) );
            const double mflops( run->getFlops() * steps / run->getBlazeResult() / 1E6 );
            std::cout << "     " << std::setw(12) << N << mflops << std::endl;
         }
      }
   }

   if( benchmarks.runBoost ) {
      std::vector<Run>::iterator run=runs.begin();
      while( run != runs.end() ) {
         const float fill( run->getFillingDegree() );
         std::cout << "   Boost uBLAS (" << fill << "% filled) [MFlop/s]:\n";
         for( ; run!=runs.end(); ++run ) {
            if( run->getFillingDegree() != fill ) break;
            const size_t N    ( run->getSize()     );
            const size_t F    ( run->getNonZeros() );
            const size_t steps( run->getSteps()    );
            run->setBoostResult( blazemark::boost::svecscalarmult( N, F, steps ) );
            const double mflops( run->getFlops() * steps / run->getBoostResult() / 1E6 );
            std::cout << "     " << std::setw(12) << N << mflops << std::endl;
         }
      }
   }

#if BLAZEMARK_GMM_MODE
   if( benchmarks.runGMM ) {
      std::vector<Run>::iterator run=runs.begin();
      while( run != runs.end() ) {
         const float fill( run->getFillingDegree() );
         std::cout << "   GMM++ (" << fill << "% filled) [MFlop/s]:\n";
         for( ; run!=runs.end(); ++run ) {
            if( run->getFillingDegree() != fill ) break;
            const size_t N    ( run->getSize()     );
            const size_t F    ( run->getNonZeros() );
            const size_t steps( run->getSteps()    );
            run->setGMMResult( blazemark::gmm::svecscalarmult( N, F, steps ) );
            const double mflops( run->getFlops() * steps / run->getGMMResult() / 1E6 );
            std::cout << "     " << std::setw(12) << N << mflops << std::endl;
         }
      }
   }
#endif

   for( std::vector<Run>::iterator run=runs.begin(); run!=runs.end(); ++run ) {
      std::cout << *run;
   }
}
//*************************************************************************************************




//=================================================================================================
//
//  MAIN FUNCTION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The main function for the sparse vector/scalar multiplication benchmark.
//
// \param argc The total number of command line arguments.
// \param argv The array of command line arguments.
// \return void
*/
int main( int argc, char** argv )
{
   std::cout << "\n Sparse Vector/Scalar Multiplication:\n";

   Benchmarks benchmarks;

   try {
      parseCommandLineArguments( argc, argv, benchmarks );
   }
   catch( std::exception& ex ) {
      std::cerr << "   " << ex.what() << "\n";
      return EXIT_FAILURE;
   }

   const std::string installPath( INSTALL_PATH );
   const std::string parameterFile( installPath + "/params/svecscalarmult.prm" );
   Parser<Run> parser;
   std::vector<Run> runs;

   try {
      parser.parse( parameterFile.c_str(), runs );
   }
   catch( std::exception& ex ) {
      std::cerr << "   Error during parameter extraction: " << ex.what() << "\n";
      return EXIT_FAILURE;
   }

   try {
      svecscalarmult( runs, benchmarks );
   }
   catch( std::exception& ex ) {
      std::cerr << "   Error during benchmark execution: " << ex.what() << "\n";
      return EXIT_FAILURE;
   }
}
//*************************************************************************************************
