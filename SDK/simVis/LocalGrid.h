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
#ifndef SIMVIS_LOCAL_GRID_H
#define SIMVIS_LOCAL_GRID_H

#include "osg/Group"
#include "osg/observer_ptr"
#include "osgEarth/ShaderGenerator"
#include "simCore/Common/Common.h"
#include "simData/DataTypes.h"
#include "simVis/Locator.h"

namespace osgText { class Text; }

namespace simVis
{
  class EntityNode;

  /// Attachment node for a local coordinate grid display.
  class SDKVIS_EXPORT LocalGridNode : public simVis::LocatorNode
  {
  public:
    /**
    * Construct a new local grid node.
    * @param[in ] hostLocator Locator of the host platform or entity
    *             A local grid is always attached to another object.
    * @param[in ] host Host entity node
    * @param[in ] referenceYear The calculations for the Speed Rings Fixed Time preference needs the scenario reference year
    */
    LocalGridNode(Locator* hostLocator, const EntityNode* host = NULL, int referenceYear = 1970);

    /**
    * Checks new preferences for this object, messages to console if there are issues
    * @param[in ] prefs Preferences to validate
    */
    void validatePrefs(const simData::LocalGridPrefs& prefs);

    /**
     * Sets new preferences for this object.
     * @param[in ] prefs Preferences to apply
     * @param[in ] force Apply them even if the current settings already match
     */
    void setPrefs(const simData::LocalGridPrefs& prefs, bool force = false);

    /**
     * The last known prefs.
     * @return Preferences
     */
    const simData::LocalGridPrefs& getPrefs() const { return lastPrefs_; }

    /**
     * Notifies grid display when host platform locator changes
     */
    void notifyHostLocatorChange();

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "LocalGridNode"; }

  protected: // methods

    /// osg::Referenced-derived
    virtual ~LocalGridNode() {}

  private: // methods
    /// recreate the geometry
    void rebuild_(const simData::LocalGridPrefs& prefs);

    /// update the locator settings
    void configureLocator_(const simData::LocalGridPrefs& prefs);

    /// calculate Speed rings parameters based on host platform data
    bool calcSpeedParams_(const simData::LocalGridPrefs& prefs);

    /// convenience function to prepare text labeling
    osgText::Text* createTextPrototype_(const simData::LocalGridPrefs& prefs, double value, const std::string& units="", int precision=1) const;

    /// convenience function to prepare text labeling
    osgText::Text* createTextPrototype_(const simData::LocalGridPrefs& prefs, const std::string& str) const;

    /// create Cartesian grid display
    void createCartesian_(const simData::LocalGridPrefs& prefs, osg::Geode* geode) const;

    /// create polar ring or range ring display
    void createRangeRings_(const simData::LocalGridPrefs& prefs, osg::Geode* geode, bool includePolarRadials) const;

    /// create speed ring or speed line display
    void createSpeedRings_(const simData::LocalGridPrefs& prefs, osg::Geode* geode, bool drawSpeedLine) const;

    /// Draws a straight line between two points, subdividing it an arbitrary number of times
    void addLineStrip_(osg::Geometry& geom, osg::Vec3Array& vertices, int& primitiveSetStart,
      const osg::Vec3& start, const osg::Vec3& end, int numPointsPerLine) const;

    simData::LocalGridPrefs lastPrefs_;
    bool                    forceRebuild_;
    double                  hostSpeedMS_;
    double                  hostTimeS_;
    int                     referenceYear_;
    double                  fixedTime_;
    osg::observer_ptr<const EntityNode> host_;
  };

} // namespace simVis

#endif // SIMVIS_LOCAL_GRID_H
