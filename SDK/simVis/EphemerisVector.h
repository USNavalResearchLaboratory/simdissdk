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
#ifndef SIMVIS_EPHEMERISVECTOR_H
#define SIMVIS_EPHEMERISVECTOR_H

#include "osgEarthUtil/Ephemeris"
#include "simCore/Common/Common.h"
#include "simCore/Time/TimeClass.h"
#include "simData/DataTypes.h"
#include "simVis/Types.h"

namespace simCore { class CoordinateConverter; }

namespace simVis
{

class PlatformModelNode;

/// Attachment node for a platform's ephemeris vector graphics.
class SDKVIS_EXPORT EphemerisVector : public osg::Group
{
public:
  /**
   * Construct a new ephemeris vector graphic.
   * @param moonColor color for moon vector
   * @param sunColor color for sun vector
   * @param lineWidth width of axis vector lines, in pixels
   */
  EphemerisVector(const simVis::Color& moonColor, const simVis::Color& sunColor, float lineWidth = 2.0);

  /**
   * Sets new preferences for this object.
   * @param prefs Preferences to apply
   */
  void setPrefs(const simData::PlatformPrefs& prefs);

  /** Updates geometry to match given platform update */
  void update(const simData::PlatformUpdate& platformUpdate);

  /** Sets the model node to use for scaling purposes */
  void setModelNode(const PlatformModelNode* hostPlatformModel);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "EphemerisVector"; }

protected:
  /// osg::Referenced-derived
  virtual ~EphemerisVector();

private:
  /// Callback that will call rebuild_() when too much time-of-day has elapsed since last rebuild
  class RebuildOnTimer;

  /// recreate the geometry
  void rebuild_(const simData::PlatformPrefs& prefs);
  /// recreates the vertices for a single ephemeris position
  void rebuildLine_(const osg::Vec3& ephemerisPosition, osg::Vec3Array& vertices, float lineLength) const;
  /// creates a standard geode for an ephemeris axis line, returning the geode and setting the array
  osg::Geode* createGeode_(osg::observer_ptr<osg::Vec3Array>& vertices, const simVis::Color& color) const;

  simData::PlatformPrefs               lastPrefs_;          ///< last prefs update
  simData::PlatformUpdate              lastUpdate_;         ///< Current platform location
  simCore::CoordinateConverter*        coordConvert_;       ///< Converts ephemeris ECEF to TP for vector calc

  osg::observer_ptr<osg::Geode> moonGeode_;
  osg::observer_ptr<osg::Vec3Array> moonVertices_;
  osg::observer_ptr<osg::Geode> sunGeode_;
  osg::observer_ptr<osg::Vec3Array> sunVertices_;
  osg::observer_ptr<const PlatformModelNode> modelNode_;
  osg::ref_ptr<osgEarth::Util::Ephemeris> ephemeris_;

  /// Last clock time when we rebuilt the line; detect time drift to rebuild line for entites that don't move
  simCore::TimeStamp lastUpdateTime_;
  bool hasLastPrefs_; ///< Whether lastPrefs_ has been set by prefs we received
};

} // namespace simVis

#endif /* SIMVIS_EPHEMERISVECTOR_H */
