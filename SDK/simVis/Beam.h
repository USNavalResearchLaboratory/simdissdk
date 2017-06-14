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
#ifndef SIMVIS_BEAM_H
#define SIMVIS_BEAM_H

#include "osg/Geometry"
#include "osg/Depth"
#include "osg/ref_ptr"
#include "osg/observer_ptr"
#include "osgEarthAnnotation/LabelNode"

#include "simVis/Antenna.h"
#include "simVis/Constants.h"
#include "simVis/Entity.h"
#include "simVis/EntityLabel.h"
#include "simVis/LocalGrid.h"
#include "simVis/LabelContentManager.h"
#include "simVis/BeamPulse.h"

namespace simVis
{
  class ScenarioManager;

  /**
   * Renders a beam.
   */
  class SDKVIS_EXPORT BeamNode : public EntityNode
  {
  public:
    /**
    * Construct a new node that displays a Beam.
    * @param scenario ScenarioManager that is managing this beam
    * @param props Initial beam properties
    * @param locator Parent locator from which this beam's locator should inherit
    * @param host This beam's host entity
    * @param referenceYear The calculation for the Speed Rings Fixed Time preference needs the scenario reference year
    */
    BeamNode(
      const ScenarioManager* scenario,
      const simData::BeamProperties& props,
      Locator*                       locator = NULL,
      const simVis::EntityNode*      host = NULL,
      int                            referenceYear = 1970);

    /**
    * The installshaderProgram is required prior to using this class.
    * This will initialize shader once in the scenario
    * @param intoStateSet State set of the scenario
    */
    static void installShaderProgram(osg::StateSet* intoStateSet);

    /**
    * Access the properties object currently representing this beam.
    *
    * @return Current properties
    */
    const simData::BeamProperties& getProperties() const { return lastProps_; }

    /**
    * Access to last known preferences.
    *
    * @return Current preferences
    */
    const simData::BeamPrefs& getPrefs() const { return lastPrefsFromDS_; }

    /**
    * Apply new preferences, replacing any existing prefs.
    * @param prefs New preferences to apply
    */
    void setPrefs(const simData::BeamPrefs& prefs);

    /**
    * Sets offset to the front of the scaled host platform model
    * along the X axis, in model units (typically meters).  Used
    * primarily to implement the missile offset for the beam.
    * @param hostMissileOffset offset along X axis to front of host model
    */
    void setHostMissileOffset(double hostMissileOffset);

    /**
    * Adds a Prefs whose values will override any values coming from a "real"
    * prefs application.
    * @param[in ] id    Unique ID of the override to add
    * @param[in ] prefs Prefs that should override data store prefs
    */
    void setPrefsOverride(const std::string& id, const simData::BeamPrefs& prefs);

    /**
    * Removes a Prefs override.
    * @param[in ] id unique identifier of the override to remove
    */
    void removePrefsOverride(const std::string& id);

    /**
    * Adds an Update whose values will override any values coming from a "real"
    * @param[in ] id     Unique ID of the override to add
    * @param[in ] update Update that should override data store updates
    * scenario update
    */
    void setUpdateOverride(const std::string& id, const simData::BeamUpdate& update);

    /**
    * Removes an Update override.
    * @param[in ] id unique identifier of the override to remove
    */
    void removeUpdateOverride(const std::string& id);

    /**
     * Gets a pointer to the last data store update, or NULL if
     * none have been applied.
     */
    const simData::BeamUpdate* getLastUpdateFromDS() const;

    /**
    * Returns the distance and position on the beam closest to the given LLA
    * @param[in ] toLla  The point for the distance calculation
    * @param[out] closestLLa The position on the beam closest to toLla
    * @return The distance in meters between the beam and toLla
    */
    double getClosestPoint(const simCore::Vec3& toLla, simCore::Vec3& closestLLa) const;

    /**
    * Returns the antenna gain
    * @param[in] az The azimuth, in radians
    * @param[in] el The elevation, in radians
    * @return The gain of the antenna
    */
    float gain(float az, float el) const;

    /**
    * Returns the antenna polarity
    * @return The polarity of the antenna
    */
    simCore::PolarityType polarity() const;

    /**
    * Sets a custom callback that will be used to generate the string that goes in the label.
    * @param callback Callback that will generate content; if NULL will only display platform name/alias
    */
    void setLabelContentCallback(LabelContentCallback* callback);

    /// Returns current content callback
    LabelContentCallback* labelContentCallback() const;

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "BeamNode"; }

  public: // EntityNode interface
    /**
    * Whether the entity is active within the scenario at the current time.
    * The entity is considered active if it has a valid position update for the
    * current scenario time, and has not received a command to turn off
    * @return true if active; false if not
    */
    virtual bool isActive() const;

    /**
    * Whether this entity is visible.
    */
    virtual bool isVisible() const;

    /**
    * Get the object ID of the beam rendered by this node
    * @return Beam's object ID
    */
    virtual simData::ObjectId getId() const;

    /** Get the beam's host's ID */
    virtual bool getHostId(simData::ObjectId& out_hostId) const;

    /**
    * Returns the entity name. Can be used to get the actual name always or the
    * actual/alias depending on the commonprefs.usealias flag.
    * @param nameType  enum option to always return real/alias name or name based on
    *            the commonprefs usealias flag.
    * @param allowBlankAlias If true DISPLAY_NAME will return blank if usealias is true and alias is blank
    * @return    actual/alias entity name string
    */
    virtual const std::string getEntityName(EntityNode::NameType nameType, bool allowBlankAlias = false) const;

    /// Returns the hook text based on the label content callback, update and preference
    virtual std::string hookText() const;

    /// Returns the legend text based on the label content callback, update and preference
    virtual std::string legendText() const;

    /**
    * Updates the entity based on the bound data store.
    * @param updateSlice  Data store update slice (could be NULL)
    * @param force true to force the update to be applied; false allows entity to use its own internal logic to decide whether the update should be applied
    * @return true if update applied, false if not
    */
    virtual bool updateFromDataStore(const simData::DataSliceBase* updateSlice, bool force=false);

    /**
    * Flushes all the entity's data point visualization.  No meaning for Beams
    */
    virtual void flush() {}

    /**
    * Returns a range value (meters) used for visualization.  Will return zero for platforms and projectors.
    */
    virtual double range() const;

    /**
    * Get the traversal mask for this node type
    * @return a traversal mask
    */
    static unsigned int getMask() { return simVis::DISPLAY_MASK_BEAM; }

  protected:
    /// osg::Referenced-derived
    virtual ~BeamNode() {}

    /**
    * Apply the specified DS update
    * this wraps target beam processing, so that calculated target beam updates can be treated as normal DS updates everywhere else in this code
    * @param update beam update to populate with calculated RAE
    * @param force tell the apply code to regenerate the visual
    */
    void applyDataStoreUpdate_(const simData::BeamUpdate& update, bool force=false);

  private: // methods

    /// update the geometry based on changes in update or preferences.
    void apply_(
      const simData::BeamUpdate*     update,
      const simData::BeamPrefs*      prefs,
      bool                           force =false);

    void updateLocator_(
      const simData::BeamUpdate*     update,
      const simData::BeamPrefs*      prefs,
      bool                           force);

    /// update things that don't require a geometry rebuild
    void performInPlacePrefChanges_(
      const simData::BeamPrefs* a,
      const simData::BeamPrefs* b,
      osg::MatrixTransform*     node);

    void performInPlaceUpdates_(
      const simData::BeamUpdate* a,
      const simData::BeamUpdate* b,
      osg::MatrixTransform*      node);

    /// apply the beam scale pref to the specified node
    void setBeamScale_(osg::MatrixTransform* node, double beamScale);

    /**
    * Adjusts the passed in position vector with offsets to make the origin of the beam at
    * the front of the host platform.
    * @param position Vector that is to be offset
    */
    void applyPlatformIconOffset_(simCore::Vec3& position) const;

    /**
    * Apply overrides (if any) to the internally cached DS update, then apply result to beam
    * @param force tell the apply code to regenerate the visual
    */
    void applyUpdateOverrides_(bool force = false);

    /**
    * Calculates the RAE to target for beam
    * @param targetBeamUpdate beam update to populate with calculated RAE
    * @return 0 if successful, non-zero on failure
    */
    int calculateTargetBeam_(simData::BeamUpdate& targetBeamUpdate);

  private: // data
    simData::BeamProperties lastProps_;

    simData::BeamPrefs      lastPrefsFromDS_;
    simData::BeamPrefs      lastPrefsApplied_;

    simData::BeamUpdate     lastUpdateFromDS_;
    simData::BeamUpdate     lastUpdateApplied_;
    bool                    hasLastUpdate_;
    bool                    hasLastPrefs_;
    osg::ref_ptr<LocatorNode>    locatorNode_;
    bool                    visible_;
    osg::ref_ptr<osg::MatrixTransform>   node_;
    osg::observer_ptr<const EntityNode> host_;
    osg::observer_ptr<const EntityNode> target_;
    osg::ref_ptr<LocalGridNode> localGrid_;
    osg::ref_ptr<AntennaNode> antenna_;

    double hostMissileOffset_;
    // extra locator used only for non-BeamType_BODY_RELATIVE beams
    osg::ref_ptr<Locator>   positionOffsetLocator_;

    void applyPrefs(const simData::BeamPrefs& prefs, bool force =false);

    typedef std::map<std::string, simData::BeamPrefs> PrefsOverrides;
    PrefsOverrides prefsOverrides_;

    typedef std::map<std::string, simData::BeamUpdate> UpdateOverrides;
    UpdateOverrides updateOverrides_;

    osg::Depth*             depthAttr_;

    void updateLabel_(const simData::BeamPrefs& prefs);
    osg::ref_ptr<EntityLabelNode> label_;
    osg::ref_ptr<LabelContentCallback> contentCallback_;
    osg::observer_ptr<const ScenarioManager> scenario_;
    osg::ref_ptr<BeamPulse> beamPulse_;
  };

} //namespace simVis

#endif // SIMVIS_BEAM_H

