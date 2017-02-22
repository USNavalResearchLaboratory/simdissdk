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
#ifndef SIMVIS_TYPES_H
#define SIMVIS_TYPES_H

#include "simCore/Common/Common.h"
#include "osgEarth/Units"
#include "osgEarthSymbology/Color"

namespace simVis
{
  /**
   * @file simVis/Types.h
   * A couple helpful aliases
   */

  /** Bring osgEarth::Units into the simVis namespace */
  typedef osgEarth::Units            Units;
  /** Bring osgEarth::Angle into the simVis namespace */
  typedef osgEarth::Angle            Angle;
  /** Bring osgEarth::Distance into the simVis namespace */
  typedef osgEarth::Distance         Distance;
  /** Bring osgEarth::Symbology::Color into the simVis namespace */
  typedef osgEarth::Symbology::Color Color;

  /** State attribute type for simVis::LightAmbient -- should not conflict with other Type values */
  static const osg::StateAttribute::Type LIGHT_AMBIENT = static_cast<osg::StateAttribute::Type>(1001);
  /** State attribute type for simVis::LightDiffuse -- should not conflict with other Type values */
  static const osg::StateAttribute::Type LIGHT_DIFFUSE = static_cast<osg::StateAttribute::Type>(1002);

} // namespace simVis


#endif // SIMVIS_TYPES_H
