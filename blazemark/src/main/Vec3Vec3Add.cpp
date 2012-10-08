//=================================================================================================
/*!
//  \file src/blaze/Vec3Vec3Add.cpp
//  \brief Source file for the 3-dimensional vector/vector addition benchmark
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
#include <blaze/math/Functions.h>
#include <blaze/math/Infinity.h>
#include <blaze/math/StaticVector.h>
#include <blaze/util/Random.h>
#include <blaze/util/Timing.h>
#include <blazemark/armadillo/Vec3Vec3Add.h>
#include <blazemark/blaze/init/StaticVector.h>
#include <blazemark/blaze/Vec3Vec3Add.h>
#include <blazemark/blitz/Vec3Vec3Add.h>
#include <blazemark/boost/Vec3Vec3Add.h>
#include <blazemark/eigen/Vec3Vec3Add.h>
#include <blazemark/flens/Vec3Vec3Add.h>
#include <blazemark/gmm/Vec3Vec3Add.h>
#include <blazemark/mtl/Vec3Vec3Add.h>
#include <blazemark/system/Armadillo.h>
#include <blazemark/system/Blitz.h>
#include <blazemark/system/Config.h>
#include <blazemark/system/Eigen.h>
#include <blazemark/system/FLENS.h>
#include <blazemark/system/GMM.h>
#include <blazemark/system/MTL.h>
#include <blazemark/system/Types.h>
#include <blazemark/util/Benchmarks.h>
#include <blazemark/util/Parser.h>
#include <blazemark/util/StaticDenseRun.h>


//*************************************************************************************************
// Using declarations
//*************************************************************************************************

using blazemark::Benchmarks;
using blazemark::Parser;
using blazemark::StaticDenseRun;




//=================================================================================================
//
//  TYPE DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Type of a benchmark run.
//
// This type definition specifies the type of a single benchmark run for the 3-dimensional
// vector/vector addition benchmark.
*/
typedef StaticDenseRun<3UL>  Run;
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

   const size_t N( run.getNumber() );

   std::vector< blaze::StaticVector<element_t,3UL,columnVector> > a( N ), b( N ), c( N );
   blaze::timing::WcTimer timer;
   double wct( 0.0 );
   size_t steps( 1UL );

   blazemark::blaze::init( a );
   blazemark::blaze::init( b );

   while( true ) {
      timer.start();
      for( size_t step=0UL, i=0UL; step<steps; ++step, ++i ) {
         if( i == N ) i = 0UL;
         c[i] = a[i] + b[i];
      }
      timer.end();
      wct = timer.last();
      if( wct >= 0.2 ) break;
      steps *= 2UL;
   }

   for( size_t i=0UL; i<N; ++i )
      if( c[i][0] < element_t(0) )
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
   run.setFlops( 3UL );
}
//*************************************************************************************************




//=================================================================================================
//
//  BENCHMARK FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief 3-dimensional vector/vector addition benchmark function.
//
// \param runs The specified benchmark runs.
// \param benchmarks The selection of benchmarks.
// \return void
*/
void vec3vec3add( std::vector<Run>& runs, Benchmarks benchmarks )
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
      std::cout << "   Blaze [MFlop/s]:\n";
      for( std::vector<Run>::iterator run=runs.begin(); run!=runs.end(); ++run ) {
         const size_t N    ( run->getNumber() );
         const size_t steps( run->getSteps()  );
         run->setBlazeResult( blazemark::blaze::vec3vec3add( N, steps ) );
         const double mflops( run->getFlops() * steps / run->getBlazeResult() / 1E6 );
         std::cout << "     " << std::setw(12) << N << mflops << std::endl;
      }
   }

   if( benchmarks.runBoost ) {
      std::cout << "   Boost uBLAS [MFlop/s]:\n";
      for( std::vector<Run>::iterator run=runs.begin(); run!=runs.end(); ++run ) {
         const size_t N    ( run->getNumber() );
         const size_t steps( run->getSteps()  );
         run->setBoostResult( blazemark::boost::vec3vec3add( N, steps ) );
         const double mflops( run->getFlops() * steps / run->getBoostResult() / 1E6 );
         std::cout << "     " << std::setw(12) << N << mflops << std::endl;
      }
   }

#if BLAZEMARK_BLITZ_MODE
   if( benchmarks.runBlitz ) {
      std::cout << "   Blitz++ [MFlop/s]:\n";
      for( std::vector<Run>::iterator run=runs.begin(); run!=runs.end(); ++run ) {
         const size_t N    ( run->getNumber() );
         const size_t steps( run->getSteps()  );
         run->setBlitzResult( blazemark::blitz::vec3vec3add( N, steps ) );
         const double mflops( run->getFlops() * steps / run->getBlitzResult() / 1E6 );
         std::cout << "     " << std::setw(12) << N << mflops << std::endl;
      }
   }
#endif

#if BLAZEMARK_GMM_MODE
   if( benchmarks.runGMM ) {
      std::cout << "   GMM++ [MFlop/s]:\n";
      for( std::vector<Run>::iterator run=runs.begin(); run!=runs.end(); ++run ) {
         const size_t N    ( run->getNumber() );
         const size_t steps( run->getSteps()  );
         run->setGMMResult( blazemark::gmm::vec3vec3add( N, steps ) );
         const double mflops( run->getFlops() * steps / run->getGMMResult() / 1E6 );
         std::cout << "     " << std::setw(12) << N << mflops << std::endl;
      }
   }
#endif

#if BLAZEMARK_ARMADILLO_MODE
   if( benchmarks.runArmadillo ) {
      std::cout << "   Armadillo [MFlop/s]:\n";
      for( std::vector<Run>::iterator run=runs.begin(); run!=runs.end(); ++run ) {
         const size_t N    ( run->getNumber() );
         const size_t steps( run->getSteps()  );
         run->setArmadilloResult( blazemark::armadillo::vec3vec3add( N, steps ) );
         const double mflops( run->getFlops() * steps / run->getArmadilloResult() / 1E6 );
         std::cout << "     " << std::setw(12) << N << mflops << std::endl;
      }
   }
#endif

#if BLAZEMARK_FLENS_MODE
   if( benchmarks.runFLENS ) {
      std::cout << "   FLENS [MFlop/s]:\n";
      for( std::vector<Run>::iterator run=runs.begin(); run!=runs.end(); ++run ) {
         const size_t N    ( run->getNumber() );
         const size_t steps( run->getSteps()  );
         run->setFLENSResult( blazemark::flens::vec3vec3add( N, steps ) );
         const double mflops( run->getFlops() * steps / run->getFLENSResult() / 1E6 );
         std::cout << "     " << std::setw(12) << N << mflops << std::endl;
      }
   }
#endif

#if BLAZEMARK_MTL_MODE
   if( benchmarks.runMTL ) {
      std::cout << "   MTL [MFlop/s]:\n";
      for( std::vector<Run>::iterator run=runs.begin(); run!=runs.end(); ++run ) {
         const size_t N    ( run->getNumber() );
         const size_t steps( run->getSteps()  );
         run->setMTLResult( blazemark::mtl::vec3vec3add( N, steps ) );
         const double mflops( run->getFlops() * steps / run->getMTLResult() / 1E6 );
         std::cout << "     " << std::setw(12) << N << mflops << std::endl;
      }
   }
#endif

#if BLAZEMARK_EIGEN_MODE
   if( benchmarks.runEigen ) {
      std::cout << "   Eigen [MFlop/s]:\n";
      for( std::vector<Run>::iterator run=runs.begin(); run!=runs.end(); ++run ) {
         const size_t N    ( run->getNumber() );
         const size_t steps( run->getSteps()  );
         run->setEigenResult( blazemark::eigen::vec3vec3add( N, steps ) );
         const double mflops( run->getFlops() * steps / run->getEigenResult() / 1E6 );
         std::cout << "     " << std::setw(12) << N << mflops << "\n";
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
/*!\brief The main function for the 3-dimensional vector/vector addition benchmark.
//
// \param argc The total number of command line arguments.
// \param argv The array of command line arguments.
// \return void
*/
int main( int argc, char** argv )
{
   std::cout << "\n 3-Dimensional Vector/Vector Addition:\n";

   Benchmarks benchmarks;

   try {
      parseCommandLineArguments( argc, argv, benchmarks );
   }
   catch( std::exception& ex ) {
      std::cerr << "   " << ex.what() << "\n";
      return EXIT_FAILURE;
   }

   const std::string installPath( INSTALL_PATH );
   const std::string parameterFile( installPath + "/params/vec3vec3add.prm" );
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
      vec3vec3add( runs, benchmarks );
   }
   catch( std::exception& ex ) {
      std::cerr << "   Error during benchmark execution: " << ex.what() << "\n";
      return EXIT_FAILURE;
   }
}
//*************************************************************************************************
