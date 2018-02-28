/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_CALC_SQUARE_MATRIX_H
#define SIMCORE_CALC_SQUARE_MATRIX_H

#include <vector>
#include "simCore/Common/Common.h"

namespace simCore
{
/**
 * Class for a square matrix.
 */
class SDKCORE_EXPORT SquareMatrix
{
public:
  /**
   * Constructor
   * @param dimension The dimension of the matrix must be 2 or greater
   */
  explicit SquareMatrix(unsigned int dimension);
  /**
   * Default Constructor
   * Configures for a 3 by 3 matrix of all zeros
   */
  SquareMatrix();
  /**
   * Copy Constructor
   */
  SquareMatrix(const SquareMatrix& rhs);
  virtual ~SquareMatrix();

  /**
   * Returns the dimension of the matrix
   * @return the dimension of the matrix
   */
  unsigned int dimension() const;

  /// Assignment operator
  SquareMatrix& operator= (const SquareMatrix &m);

  /**
   * Set the matrix to the identity matrix
   * @return 0 on success, 1 on failure
   */
  int makeIdentity();

  /**
   * Set the matrix to all zeros
   * @return 0 on success, 1 on failure
   */
  int makeZero();

  /**
   * Transpose the matrix
   * @return 0 on success, 1 on failure
   */
  int transpose();

  /**
   * Set an individual location in the matrix
   * @param row The row of the location to set
   * @param col The column of the location to set
   * @param value The value for the given location
   * @return 0 on success, 1 on failure
   */
  int set(unsigned int row, unsigned int col, double value);

  /**
   * Get an individual location in the matrix
   * @param row The row of the location to get
   * @param col The column of the location to get
   * @param value The value for the given location
   * @return 0 on success, 1 on failure
   */
  int get(unsigned int row, unsigned int col, double& value) const;

  /**
   * Multiple the matrix by a scalar
   * @param scaleValue The value for scaling the matrix
   * @return 0 on success, 1 on failure
   */
  int scale(double scaleValue);

  /**
   * Add a matrix to the current matrix
   * @param m The matrix to add to the current matrix
   * @return 0 on success, 1 on failure
   */
  int add(const SquareMatrix& m);

  /**
   * Multiply the given matrix to the right of the current matrix
   * @param m The matrix to multiply to the current matrix
   * @return 0 on success, 1 on failure
   */
  int preMultiply(const SquareMatrix& m);

  /**
   * Multiply the given matrix to the left of the current matrix
   * @param m The matrix to multiply to the current matrix
   * @return 0 on success, 1 on failure
   */
  int postMultiply(const SquareMatrix& m);

private:
  /**
   * Get an individual location in the matrix
   * @param row The row of the location to get
   * @param col The column of the location to get
   * @return The value for the given location
   */
  double get_(unsigned int row, unsigned int col) const;

  unsigned int dimension_;
  std::vector<double> matrix_;
};

/**
 * Return true if the two matrices are equal within the given tolerance
 * @param m1 First matrix to compare
 * @param m2 Second matrix to compare
 * @param t Tolerance of the comparison
 * @return True if the matrices are equal within the given tolerance
 */
SDKCORE_EXPORT bool areEqual(const SquareMatrix& m1, const SquareMatrix& m2, double t = 1.0e-6);

}

#endif /* SIMCORE_CALC_SQUARE_MATRIX_H */
