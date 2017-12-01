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
#ifndef SIMVIS_GATE_H
#define SIMVIS_GATE_H

#include "osg/observer_ptr"
#include "simData/DataTypes.h"
#include "simVis/Constants.h"
#include "simVis/Entity.h"
#include "simVis/LocatorNode.h"

namespace osg { class Depth; }
namespace simVis
{
  class EntityLabelNode;
  class LabelContentCallback;
  class LocalGridNode;
  class Locator;

  /// Scene graph node representing the Gate volume
  class SDKVIS_EXPORT GateVolume : public simVis::LocatorNode
  {
  public:
    /** Constructor */
    GateVolume(simVis::Locator* locator, const simData::GatePrefs* prefs, const simData::GateUpdate* update);

    /** Perform an in-place update to an existing volume */
    void performInPlaceUpdates(const simData::GateUpdate* a,
                                const simData::GateUpdate* b);

    /** Perform an in-place update to an existing volume */
    void performInPlacePrefChanges(const simData::GatePrefs* a,
                                    const simData::GatePrefs* b);

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }
    /** Return the class name */
    virtual const char* className() const { return "GateVolume"; }

  protected:
    /** Protected destructor due to deriving from osg::Referenced. */
    virtual ~GateVolume();

  private:
    // Not implemented
    GateVolume(const GateVolume&);

    osg::MatrixTransform* createNode_(const simData::GatePrefs* prefs, const simData::GateUpdate* update);

    osg::ref_ptr<osg::MatrixTransform> gateSV_;
  };

  /// Scene graph node representing a Gate centroid
  class SDKVIS_EXPORT GateCentroid : public simVis::LocatorNode
  {
  public:
    /** Constructor */
    explicit GateCentroid(simVis::Locator* locator);

    /** Perform an in-place update to an existing centroid */
    void update(const simData::GateUpdate& update);
    /** Clears the geometry from the centroid; use this instead of setting node mask, to avoid center-on-entity issues. */
    void setVisible(bool visible);

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }
    /** Return the class name */
    virtual const char* className() const { return "GateCentroid"; }

  protected:
    /** Protected destructor due to deriving from osg::Referenced. */
    virtual ~GateCentroid();

  private:
    // Not implemented
    GateCentroid(const GateCentroid&);

    /// calculate centroid verts from update
    void updateCentroid_(osg::Vec3Array* verts, const simData::GateUpdate& update);

    /// Holds the vertices for geometry
    osg::ref_ptr<osg::Geometry> geom_;
  };

  /// Scene graph node representing a Gate
  class SDKVIS_EXPORT GateNode : public EntityNode
  {
  public:
    /**
    * Construct a new Gate node.
    * @param props   Initial properties to apply to gate
    * @param locator Parent locator from which this gate should inherit its location
    * @param host This gate's host platform
    * @param referenceYear The calculation for the Speed Rings Fixed Time preference needs the scenario reference year
    */
    explicit GateNode(
      const simData::GateProperties& props,
      Locator*                       locator = NULL,
      const simVis::EntityNode*      host = NULL,
      int                            referenceYear = 1970);

    /**
    * Get latest properties applied to this gate.
    * @return Gate properties.
    */
    const simData::GateProperties& getProperties() const { return lastProps_; }

    /**
    * Get latest preferences applied to this gate.
    * @return Gate preferences.
    */
    const simData::GatePrefs& getPrefs() const { return lastPrefsFromDS_; }

    /**
    * Apply a new set of preferences to this gate node.
    * @param prefs New preferences to apply
    */
    void setPrefs(const simData::GatePrefs& prefs);

    /**
    * Adds a Prefs whose values will override any values coming from a "real"
    * prefs application.
    * @param[in ] id    Unique ID of the override to add
    * @param[in ] prefs Prefs that should override data store prefs
    */
    void setPrefsOverride(const std::string& id, const simData::GatePrefs& prefs);

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
    void setUpdateOverride(const std::string& id, const simData::GateUpdate& update);

    /**
    * Removes an Update override.
    * @param[in ] id unique identifier of the override to remove
    */
    void removeUpdateOverride(const std::string& id);

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
    virtual const char* className() const { return "GateNode"; }

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
    * Get the ID of the gate being rendered.
    * @return A unique ID
    */
    virtual simData::ObjectId getId() const;

    /** Get the gate's host's ID */
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
    * Flushes all the entity's data point visualization.
    */
    virtual void flush();

    /**
    * Returns a range value (meters) used for visualization.
    */
    virtual double range() const;

    /** Retrieve the object index tag for gates. */
    virtual unsigned int objectIndexTag() const;

    /**
    * Get the traversal mask for this node type.
    * @return a traversal mask
    */
    static unsigned int getMask() { return simVis::DISPLAY_MASK_GATE; }

    /**
     * Gets a pointer to the last data store update, or NULL if
     * none have been applied.
     */
    const simData::GateUpdate* getLastUpdateFromDS() const;

  protected:
    /// osg::Referenced-derived; destructor body needs to be in the .cpp
    virtual ~GateNode();

  private:
    // Not implemented
    GateNode(const GateNode&);

    /// determine if new update/new prefs can be handled with in-place-update (without complete rebuild)
    bool changeRequiresRebuild_(
      const simData::GateUpdate* newUpdate,
      const simData::GatePrefs* newPrefs) const;

    /// update the gate based on the specified update and prefs
    void apply_(
      const simData::GateUpdate* update,
      const simData::GatePrefs*  prefs,
      bool                       force = false);

    void updateLocator_(
      const simData::GateUpdate* update,
      const simData::GatePrefs*  prefs,
      bool                       force);

    /// apply the specified prefs, adding any overrides
    void applyPrefs_(
      const simData::GatePrefs& prefs,
      bool                      force = false);

    /**
    * Apply the specified DS update to the gate
    * this wraps target gate processing, so that calculated target gate updates can be treated as normal DS updates everywhere else in this code
    * @param update gate update to populate with calculated RAE
    * @param force tell the apply code to regenerate the visual
    */
    void applyDataStoreUpdate_(const simData::GateUpdate& update, bool force = false);

    /**
    * Apply overrides (if any) to the internally cached DS update, then apply result to gate
    * @param force tell the apply code to regenerate the visual
    */
    void applyUpdateOverrides_(bool force = false);

    /**
    * Creates a new update for the target gate based on the input update and the host beam's calculated RAE
    * @param update Datastore update for the gate
    * @param targetGateUpdate calculated target gate update
    * @return 0 if successful, non-zero if calculation could not be completed
    */
    int calculateTargetGate_(const simData::GateUpdate& update, simData::GateUpdate& targetGateUpdate);

  private:
    simData::GateProperties lastProps_;

    simData::GatePrefs      lastPrefsFromDS_;   // last Prefs set from the data store
    simData::GatePrefs      lastPrefsApplied_;  // last one applied (with accumulated overrides)

    simData::GateUpdate     lastUpdateFromDS_;  // last Update received from the data store
    simData::GateUpdate     lastUpdateApplied_; // last Update applied (with accumulated overrides)

    bool                    hasLastUpdate_;
    bool                    hasLastPrefs_;

    /**
     * Locator that represents the "origin" of the gate, typically a position on the host platform, stripped of orientation
     * this supports the drawing of the gate volume, with special processing for the Coverage draw type
     */
    osg::ref_ptr<Locator>  gateVolumeLocator_;

    /**
     * Locator that represents the centroid of the gate
     * this supports drawing the centroid and the localgrid
     */
    osg::ref_ptr<Locator>  centroidLocator_;

    bool                    visible_;

    /// container for the gate volume geometry
    osg::ref_ptr<GateVolume>   gateVolume_;

    /// container for the centroid geometry
    osg::ref_ptr<GateCentroid>   centroid_;

    osg::observer_ptr<const EntityNode> host_;
    osg::ref_ptr<LocalGridNode> localGrid_;
    osg::Depth*             depthAttr_;

    /**
     * Locator that represents the "origin" of the gate, typically a position on the host platform, stripped of orientation
     * this locator exists to apply specific offsets to its parent, to be inherited by the centroidPositionOffsetLocator_
     */
    osg::ref_ptr<Locator>   baseLocator_;

    /**
     * Locator that establishes a coordinate system at the gate centroid of the gate, stripped of orientation
     * this locator exists to apply specific offsets to its parent, to be inherited by the centroidLocator in centroidLocatorNode_
     */
    osg::ref_ptr<Locator>   centroidPositionOffsetLocator_;

    std::map<std::string, simData::GatePrefs> prefsOverrides_;

    std::map<std::string, simData::GateUpdate> updateOverrides_;

    void updateLabel_(const simData::GatePrefs& prefs);
    osg::ref_ptr<EntityLabelNode> label_;
    osg::ref_ptr<LabelContentCallback> contentCallback_;

    unsigned int objectIndexTag_;
  };

} // namespace simVis

#endif // SIMVIS_GATE_H

