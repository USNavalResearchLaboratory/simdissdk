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
  * Converts a quaternion vector to direction cosine matrix using a NED frame
  * @param q  4 element double c++ array quaternion
  * @pre quaternion valid
  * @return 0 on success, non-zero if quaternion array was invalid or not normalized
  */
  int fromDQ(const double q[4]);

  /**
  * Converts a quaternion array to direction cosine matrix using a NED frame
  * @param q  4 element double std::array quaternion
  * @pre quaternion non-empty
  * @return 0 on success, non-zero if quaternion array was invalid or not normalized
  */
  int fromQ(const std::array<double, 4>& q);
};

}

#endif
