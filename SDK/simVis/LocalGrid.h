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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_LOCAL_GRID_H
#define SIMVIS_LOCAL_GRID_H

#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "simCore/Common/Common.h"
#include "simData/DataTypes.h"
#include "simVis/LocatorNode.h"

namespace osg { class Group; }
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
    LocalGridNode(Locator* hostLocator, const EntityNode* host = nullptr, int referenceYear = 1970);

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

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "LocalGridNode"; }

  public: // LocatorNode interface
    virtual void syncWithLocator(); //override

  protected: // methods

    /// osg::Referenced-derived
    virtual ~LocalGridNode();

  private: // methods
    /// recreate the geometry
    void rebuild_(const simData::LocalGridPrefs& prefs);

    /// update the locator settings
    void configureLocator_(const simData::LocalGridPrefs& prefs);

    /// create Cartesian grid display
    void createCartesian_(const simData::LocalGridPrefs& prefs, osg::Group* geomGroup, osg::Group* labelGroup) const;

    /// create polar ring or range ring display
    void createRangeRings_(const simData::LocalGridPrefs& prefs, osg::Group* geomGroup, osg::Group* labelGroup, bool includePolarRadials) const;

    /// create speed ring or speed line display
    void createSpeedRings_(const simData::LocalGridPrefs& prefs, osg::Group* geomGroup, osg::Group* labelGroup, bool drawSpeedLine) const;

    /// update the speed ring/line display for current data
    void updateSpeedRings_(const simData::LocalGridPrefs& prefs, double sizeM, double timeRadiusSeconds);

    /**
    * Determine if speed rings/speed line display can be displayed, and process speed-related variables for display
    * @param[in ] prefs Preferences that determine display characteristics
    * @param[out ] sizeM size in meters of outer ring/speed line
    * @param[out ] timeRadiusSeconds  size of outer ring/speed line expressed as a time in seconds
    * @return if < 0, display is not valid; if == 0, display is valid; if > 0, display needs to be updated.
    */
    int processSpeedParams_(const simData::LocalGridPrefs& prefs, double& sizeM, double& timeRadiusSeconds);

  private: // data
    osg::ref_ptr<osg::Group> graphicsGroup_;
    osg::ref_ptr<osg::Group> labelGroup_;

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
