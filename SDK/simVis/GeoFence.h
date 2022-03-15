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
#ifndef SIMVIS_GEOFENCE_H
#define SIMVIS_GEOFENCE_H

#include "simCore/Common/Common.h"
#include "simNotify/Notify.h"
#include "osg/Referenced"
#include "osg/Vec3d"
#include "osg/Math"

namespace simVis
{
/**
  * A GeoFence is a monitored geospatial region. It fires an event
  * whenever something enters or exits the region.
  * (header only)
  */
class GeoFence : public osg::Referenced
{
public:
  /** Returns true if the given ECEF point is inside the fence */
  virtual bool contains(const osg::Vec3d& p) const =0;

protected:
  /** Protected Constructor */
  GeoFence() { }
  /** Protected Destructor */
  virtual ~GeoFence() { }
};


/** GeoFence that uses a hemisphere as the region predicate. */
class SDKVIS_EXPORT HorizonGeoFence : public GeoFence
{
public:
  /** Constructor */
  explicit HorizonGeoFence(double earthRadius =6371000.0);

  /** Sets the location of the eye, used to calculate the horizon */
  void setLocation(const osg::Vec3d& ecef);

  // GeoFence interface
  /** Returns true if the ECEF point p is visible relative to horizon */
  virtual bool contains(const osg::Vec3d& p) const;

protected:
  virtual ~HorizonGeoFence() { }

  /** ECEF position of the reference point in meters */
  osg::Vec3d ecef_;
  /** Normalized ECEF point */
  osg::Vec3d pnorm_;
  /** Minimum deviation, used to calculate contains() */
  double     minDeviation_;
  /** Earth radius in meters */
  double     earthRadius_;
};

} // namespace simVis

#endif // SIMVIS_GEOFENCE_H
