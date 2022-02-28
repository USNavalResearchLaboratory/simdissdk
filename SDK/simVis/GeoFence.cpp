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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simVis/GeoFence.h"

#undef LC
#define LC "[GeoFence] "

namespace simVis {

HorizonGeoFence::HorizonGeoFence(double earthRadius) :
GeoFence(),
ecef_(0, 0, 1),
pnorm_(0, 0, 1),
minDeviation_(1.0),
earthRadius_(earthRadius)
{
  //nop
}


void HorizonGeoFence::setLocation(const osg::Vec3d& ecef)
{
  ecef_ = ecef;
  pnorm_ = ecef;
  pnorm_.normalize();
  double angleRP = acos(earthRadius_/ecef.length());
  double horizonAngle = angleRP + osg::PI_2;
  minDeviation_ = -1.0 + 2.0*((osg::PI - horizonAngle)/osg::PI);
}


bool HorizonGeoFence::contains(const osg::Vec3d& p) const
{
  osg::Vec3d pt = (p - ecef_);
  pt.normalize();
  double dev = pt * pnorm_;
  return dev >= minDeviation_;
}

}
