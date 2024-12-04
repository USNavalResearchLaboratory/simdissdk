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
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_CALC_DCM_H
#define SIMCORE_CALC_DCM_H

#include <array>
#include <vector>
#include "simCore/Calc/SquareMatrix.h"

namespace simCore
{
/**
 * Class for a 3x3 square matrix that supports direction cosine matrix (DCM) calculations.
 */
class SDKCORE_EXPORT Dcm : public SquareMatrix
{
public:
  /**
   * Default Constructor
   * Configures for a 3 by 3 DCM of all zeros
   */
  Dcm();
  virtual ~Dcm();

  /**
  * Calculate the determinant of the (square) matrix, which should be 1 for a proper rotation matrix
  * @return determinant of the matrix
  */
  double determinant() const;

  /**
  * Determine if matrix is a valid rotation matrix
  * @param t Tolerance of the comparisons
  * @return true if matrix is a valid rotation matrix
  */
  bool isValid(double t = 1.0e-6) const;

  /**
  * Convert the direction cosine matrix to Euler angles using a NED frame
  * @return euler angles as a Vec3
  */
  Vec3 toEuler() const;

  /**
  * Convert Euler angles to a direction cosine matrix using a NED frame
  * @param ea  Euler angles
  */
  void fromEuler(const Vec3& ea);

  /**
  * Convert the direction cosine matrix to quaternion
  * @param forcePositiveScalar  ensure that quaternion scalar is positive by negating components when necessary
  * @return  quaternion as a std:array, where scale ("scalar part", "real part") is the [0] element
  */
  std::array<double, 4> toQ() const;

  /**
  * Converts a quaternion vector to direction cosine matrix
  * @param q  4 element double c++ std::array quaternion
  * @pre quaternion valid, where scale ("scalar part", "real part") is the [0] element of the quaternion array
  * @return 0 on success, non-zero if quaternion array was invalid or not normalized
  */
  int fromDQ(const double q[4]);

  /**
  * Converts a quaternion array to direction cosine matrix
  * @param q  4 element double std::array quaternion
  * @pre quaternion non-empty, where scale ("scalar part", "real part") is the [0] element of the quaternion array
  * @return 0 on success, non-zero if quaternion array was invalid or not normalized
  */
  int fromQ(const std::array<double, 4>& q);
};

}

#endif
