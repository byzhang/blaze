//=================================================================================================
/*!
//  \file blaze/math/expressions/TSVecTDMatMultExpr.h
//  \brief Header file for the transpose sparse vector/transpose dense matrix multiplication expression
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

#ifndef _BLAZE_MATH_EXPRESSIONS_TSVECTDMATMULTEXPR_H_
#define _BLAZE_MATH_EXPRESSIONS_TSVECTDMATMULTEXPR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <stdexcept>
#include <boost/type_traits/remove_reference.hpp>
#include <blaze/math/constraints/DenseMatrix.h>
#include <blaze/math/constraints/DenseVector.h>
#include <blaze/math/constraints/SparseVector.h>
#include <blaze/math/constraints/StorageOrder.h>
#include <blaze/math/constraints/TransposeFlag.h>
#include <blaze/math/expressions/DenseVector.h>
#include <blaze/math/expressions/Expression.h>
#include <blaze/math/expressions/Forward.h>
#include <blaze/math/shims/Reset.h>
#include <blaze/math/traits/MultTrait.h>
#include <blaze/math/typetraits/CanAlias.h>
#include <blaze/math/typetraits/IsExpression.h>
#include <blaze/math/typetraits/IsMatMatMultExpr.h>
#include <blaze/util/Assert.h>
#include <blaze/util/constraints/Reference.h>
#include <blaze/util/DisableIf.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/SelectType.h>
#include <blaze/util/Types.h>


namespace blaze {

//=================================================================================================
//
//  CLASS TSVECDMATMULTEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for transpose sparse vector-transpose dense matrix multiplications.
// \ingroup dense_vector_expression
//
// The TSVecTDMatMultExpr class represents the compile time expression for multiplications
// between transpose sparse vectors and column-major dense matrices.
*/
template< typename VT    // Type of the left-hand side sparse vector
        , typename MT >  // Type of the right-hand side dense matrix
class TSVecTDMatMultExpr : public DenseVector< TSVecTDMatMultExpr<VT,MT>, true >
                         , private Expression
{
 private:
   //**Type definitions****************************************************************************
   typedef typename VT::ResultType     VRT;  //!< Result type of the left-hand side sparse vector expression.
   typedef typename MT::ResultType     MRT;  //!< Result type of the right-hand side dense matrix expression.
   typedef typename VT::CompositeType  VCT;  //!< Composite type of the left-hand side sparse vector expression.
   typedef typename MT::CompositeType  MCT;  //!< Composite type of the right-hand side dense matrix expression.
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the evaluation strategy of the multiplication expression.
   /*! The \a useAssign compile time constant expression represents a compilation switch for
       the evaluation strategy of the multiplication expression. In case the dense matrix
       expression requires an intermediate evaluation or the sparse vector expression is a
       compound expression, \a useAssign will be set to \a true and the addition expression
       will be evaluated via the \a assign function family. Otherwise \a useAssign will be
       set to \a false and the expression will be evaluated via the subscript operator. */
   enum { useAssign = ( IsExpression<VT>::value || !IsReference<MCT>::value ) };
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   template< typename VT2 >
   struct UseAssign {
      enum { value = useAssign };
   };
   /*! \endcond */
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   typedef TSVecTDMatMultExpr<VT,MT>           This;           //!< Type of this TSVecTDMatMultExpr instance.
   typedef typename MultTrait<VRT,MRT>::Type   ResultType;     //!< Result type for expression template evaluations.
   typedef typename ResultType::TransposeType  TransposeType;  //!< Transpose type for expression template evaluations.
   typedef typename ResultType::ElementType    ElementType;    //!< Resulting element type.
   typedef const ElementType                   ReturnType;     //!< Return type for expression template evaluations.

   //! Data type for composite expression templates.
   typedef typename SelectType< useAssign, const ResultType, const TSVecTDMatMultExpr& >::Type  CompositeType;

   //! Composite type of the left-hand side sparse vector expression.
   typedef typename SelectType< IsExpression<VT>::value, const VT, const VT& >::Type  LeftOperand;

   //! Composite type of the right-hand side sparse matrix expression.
   typedef typename SelectType< IsExpression<MT>::value, const MT, const MT& >::Type  RightOperand;

   //! Type for the assignment of the left-hand side sparse vector operand.
   typedef typename SelectType< IsExpression<VT>::value, const VRT, VCT >::Type  LT;

   //! Type for the assignment of the left-hand side dense matrix operand.
   typedef MCT  RT;
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template evaluation strategy.
   enum { vectorizable = 0 };

   //! Compilation flag for the detection of aliasing effects.
   enum { canAlias = IsReference<MCT>::value && ( !IsExpression<MT>::value || CanAlias<MT>::value ) };
   //**********************************************************************************************

   //**Constructor*********************************************************************************
   /*!\brief Constructor for the TSVecTDMatMultExpr class.
   //
   // \param vec The left-hand side sparse vector operand of the multiplication expression.
   // \param mat The right-hand side dense matrix operand of the multiplication expression.
   */
   explicit inline TSVecTDMatMultExpr( const VT& vec, const MT& mat )
      : vec_( vec )  // Left-hand side sparse vector of the multiplication expression
      , mat_( mat )  // Right-hand side dense matrix of the multiplication expression
   {
      BLAZE_INTERNAL_ASSERT( vec_.size() == mat_.rows(), "Invalid vector and matrix sizes" );
   }
   //**********************************************************************************************

   //**Subscript operator**************************************************************************
   /*!\brief Subscript operator for the direct access to the vector elements.
   //
   // \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
   // \return The resulting value.
   */
   inline ReturnType operator[]( size_t index ) const {
      BLAZE_INTERNAL_ASSERT( index < mat_.columns(), "Invalid vector access index" );

      typedef typename boost::remove_reference<VCT>::type::ConstIterator  ConstIterator;

      VCT x( vec_ );  // Evaluation of the left-hand side sparse vector operand

      BLAZE_INTERNAL_ASSERT( x.size() == vec_.size(), "Invalid vector size" );

      const ConstIterator end( x.end() );
      ConstIterator element( x.begin() );
      ElementType res;

      if( element != end ) {
         res = element->value() * mat_( element->index(), index );
         ++element;
         for( ; element!=end; ++element )
            res += element->value() * mat_( element->index(), index );
      }
      else {
         reset( res );
      }

      return res;
   }
   //**********************************************************************************************

   //**Size function*******************************************************************************
   /*!\brief Returns the current size/dimension of the vector.
   //
   // \return The size of the vector.
   */
   inline size_t size() const {
      return mat_.columns();
   }
   //**********************************************************************************************

   //**Left operand access*************************************************************************
   /*!\brief Returns the left-hand side sparse vector operand.
   //
   // \return The left-hand side sparse vector operand.
   */
   inline LeftOperand leftOperand() const {
      return vec_;
   }
   //**********************************************************************************************

   //**Right operand access************************************************************************
   /*!\brief Returns the right-hand side transpose dense matrix operand.
   //
   // \return The right-hand side transpose dense matrix operand.
   */
   inline RightOperand rightOperand() const {
      return mat_;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression is aliased with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case the given alias is contained in this expression, \a false if not.
   */
   template< typename T >
   inline bool isAliased( const T* alias ) const {
      return ( IsReference<MCT>::value && mat_.isAliased( alias ) );
   }
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   LeftOperand  vec_;  //!< Left-hand side sparse vector of the multiplication expression.
   RightOperand mat_;  //!< Right-hand side dense matrix of the multiplication expression.
   //**********************************************************************************************

   //**Assignment to dense vectors*****************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a transpose sparse vector-transpose dense matrix multiplication to
   //        a dense vector.
   // \ingroup dense_vector
   //
   // \param lhs The target left-hand side dense vector.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a transpose sparse vector-
   // transpose dense matrix multiplication expression to a dense vector. Due to the explicit
   // application of the SFINAE principle, this operator can only be selected by the compiler
   // in case either the left-hand side matrix operand requires an intermediate evaluation or
   // the right-hand side vector operand is a compound expression.
   */
   template< typename VT2 >  // Type of the target dense vector
   friend inline typename EnableIf< UseAssign<VT2> >::Type
      assign( DenseVector<VT2,true>& lhs, const TSVecTDMatMultExpr& rhs )
   {
      BLAZE_INTERNAL_ASSERT( (~lhs).size() == rhs.size(), "Invalid vector sizes" );

      typedef typename boost::remove_reference<LT>::type::ConstIterator  ConstIterator;

      // Evaluation of the left-hand side sparse vector operand
      LT x( rhs.vec_ );
      if( x.nonZeros() == 0UL ) {
         reset( ~lhs );
         return;
      }

      // Evaluation of the right-hand side dense matrix operand
      RT A( rhs.mat_ );

      // Checking the evaluated operands
      BLAZE_INTERNAL_ASSERT( x.size()    == rhs.vec_.size()   , "Invalid vector size"       );
      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.mat_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.mat_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.columns() == (~lhs).size()     , "Invalid vector size"       );

      // Performing the sparse vector-dense matrix multiplication
      const ConstIterator end( x.end() );

      for( size_t i=0UL; i<(~lhs).size(); ++i )
      {
         ConstIterator element( x.begin() );

         (~lhs)[i] = element->value() * A(element->index(),i);
         ++element;
         for( ; element!=end; ++element ) {
            (~lhs)[i] += element->value() * A(element->index(),i);
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Assignment to sparse vectors****************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a transpose sparse vector-transpose dense matrix multiplication to
   //        a sparse vector.
   // \ingroup sparse_vector
   //
   // \param lhs The target left-hand side sparse vector.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a transpose sparse vector-
   // transpose dense matrix multiplication expression to a sparse vector.
   */
   template< typename VT2 >  // Type of the target sparse vector
   friend inline void assign( SparseVector<VT2,true>& lhs, const TSVecTDMatMultExpr& rhs )
   {
      BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_REFERENCE_TYPE( typename ResultType::CompositeType );

      BLAZE_INTERNAL_ASSERT( (~lhs).size() == rhs.size(), "Invalid vector sizes" );

      const ResultType tmp( rhs );
      assign( ~lhs, tmp );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to dense vectors********************************************************
   /*!\brief Addition assignment of a transpose sparse vector-transpose dense matrix multiplication
   //        to a dense vector.
   //
   // \param lhs The target left-hand side dense vector.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized addition assignment of a transpose sparse
   // vector-transpose dense matrix multiplication expression to a dense vector.
   */
   template< typename VT2 >  // Type of the target dense vector
   friend inline void addAssign( DenseVector<VT2,true>& lhs, const TSVecTDMatMultExpr& rhs )
   {
      BLAZE_INTERNAL_ASSERT( (~lhs).size() == rhs.size(), "Invalid vector sizes" );

      typedef typename boost::remove_reference<LT>::type::ConstIterator  ConstIterator;

      // Evaluation of the left-hand side sparse vector operand
      LT x( rhs.vec_ );
      if( x.nonZeros() == 0UL ) return;

      // Evaluation of the right-hand side dense matrix operand
      RT A( rhs.mat_ );

      // Checking the evaluated operands
      BLAZE_INTERNAL_ASSERT( x.size()    == rhs.vec_.size()   , "Invalid vector size"       );
      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.mat_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.mat_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.columns() == (~lhs).size()     , "Invalid vector size"       );

      // Performing the sparse vector-dense matrix multiplication
      const ConstIterator end( x.end() );
      ConstIterator element( x.begin() );

      for( ; element!=end; ++element ) {
         for( size_t i=0UL; i<A.columns(); ++i ) {
            (~lhs)[i] += element->value() * A(element->index(),i);
         }
      }
   }
   //**********************************************************************************************

   //**Addition assignment to sparse vectors*******************************************************
   // No special implementation for the addition assignment to sparse vectors.
   //**********************************************************************************************

   //**Subtraction assignment to dense vectors*****************************************************
   /*!\brief Subtraction assignment of a transpose sparse vector-transpose dense matrix
   //        multiplication to a dense vector.
   //
   // \param lhs The target left-hand side dense vector.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized subtraction assignment of a transpose
   // sparse vector-transpose dense matrix multiplication expression to a dense vector.
   */
   template< typename VT2 >  // Type of the target dense vector
   friend inline void subAssign( DenseVector<VT2,true>& lhs, const TSVecTDMatMultExpr& rhs )
   {
      BLAZE_INTERNAL_ASSERT( (~lhs).size() == rhs.size(), "Invalid vector sizes" );

      typedef typename boost::remove_reference<LT>::type::ConstIterator  ConstIterator;

      // Evaluation of the left-hand side sparse vector operand
      LT x( rhs.vec_ );
      if( x.nonZeros() == 0UL ) return;

      // Evaluation of the right-hand side dense matrix operand
      RT A( rhs.mat_ );

      // Checking the evaluated operands
      BLAZE_INTERNAL_ASSERT( x.size()    == rhs.vec_.size()   , "Invalid vector size"       );
      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.mat_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.mat_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.columns() == (~lhs).size()     , "Invalid vector size"       );

      // Performing the sparse vector-dense matrix multiplication
      const ConstIterator end( x.end() );
      ConstIterator element( x.begin() );

      for( ; element!=end; ++element ) {
         for( size_t i=0UL; i<A.columns(); ++i ) {
            (~lhs)[i] -= element->value() * A(element->index(),i);
         }
      }
   }
   //**********************************************************************************************

   //**Subtraction assignment to sparse vectors****************************************************
   // No special implementation for the subtraction assignment to sparse vectors.
   //**********************************************************************************************

   //**Multiplication assignment to dense vectors**************************************************
   /*!\brief Multiplication assignment of a transpose sparse vector-transpose dense matrix
   //        multiplication to a dense vector.
   //
   // \param lhs The target left-hand side dense vector.
   // \param rhs The right-hand side multiplication expression to be multiplied.
   // \return void
   //
   // This function implements the performance optimized multiplication assignment of a transpose
   // sparse vector-transpose dense matrix multiplication expression to a dense vector.
   */
   template< typename VT2 >  // Type of the target dense vector
   friend inline void multAssign( DenseVector<VT2,true>& lhs, const TSVecTDMatMultExpr& rhs )
   {
      BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_REFERENCE_TYPE( typename ResultType::CompositeType );

      BLAZE_INTERNAL_ASSERT( (~lhs).size() == rhs.size(), "Invalid vector sizes" );

      const ResultType tmp( rhs );
      multAssign( ~lhs, tmp );
   }
   //**********************************************************************************************

   //**Multiplication assignment to sparse vectors*************************************************
   // No special implementation for the multiplication assignment to sparse vectors.
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE( VT );
   BLAZE_CONSTRAINT_MUST_BE_TRANSPOSE_VECTOR_TYPE( VT );
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT );
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Multiplication operator for the multiplication of a transpose sparse vector and a
//        column-major dense matrix (\f$ \vec{y}^T=\vec{x}^T*A \f$).
// \ingroup dense_matrix
//
// \param vec The left-hand side transpose sparse vector for the multiplication.
// \param mat The right-hand side column-major dense matrix for the multiplication.
// \return The resulting transpose vector.
// \exception std::invalid_argument Vector and matrix sizes do not match.
//
// This operator represents the multiplication between a transpose sparse vector and a column-major
// dense matrix:

   \code
   using blaze::rowVector;
   using blaze::columnMajor;

   blaze::CompressedVector<double,rowVector> x, y;
   blaze::DynamicMatrix<double,columnMajor> A;
   // ... Resizing and initialization
   y = x * A;
   \endcode

// The operator returns an expression representing a transpose sparse vector of the higher-order
// element type of the two involved element types \a T1::ElementType and \a T2::ElementType.
// Both the dense matrix type \a T1 and the dense vector type \a T2 as well as the two element
// types \a T1::ElementType and \a T2::ElementType have to be supported by the MultTrait class
// template.\n
// In case the current size of the vector \a vec doesn't match the current number of rows of
// the matrix \a mat, a \a std::invalid_argument is thrown.
*/
template< typename T1, typename T2 >
inline const typename DisableIf< IsMatMatMultExpr<T2>, TSVecTDMatMultExpr<T1,T2> >::Type
   operator*( const SparseVector<T1,true>& vec, const DenseMatrix<T2,true>& mat )
{
   if( (~vec).size() != (~mat).rows() )
      throw std::invalid_argument( "Vector and matrix sizes do not match" );

   return TSVecTDMatMultExpr<T1,T2>( ~vec, ~mat );
}
//*************************************************************************************************

} // namespace blaze

#endif
