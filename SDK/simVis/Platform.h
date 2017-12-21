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
#ifndef SIMVIS_PLATFORM_NODE_H
#define SIMVIS_PLATFORM_NODE_H

#include "osg/ref_ptr"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/EM/RadarCrossSection.h"
#include "simData/DataTypes.h"
#include "simVis/Constants.h"
#include "simVis/Entity.h"

namespace simData { class DataStore; }
namespace simVis
{
  class AreaHighlightNode;
  class AxisVector;
  class EphemerisVector;
  class LabelContentCallback;
  class LocalGridNode;
  class PlatformInertialTransform;
  class PlatformModelNode;
  class PlatformTspiFilterManager;
  class RadialLOSNode;
  class TrackHistoryNode;
  class VelocityVector;

  /**
  * Interface for an object that will create LOS nodes as they're needed.
  * Allows all platforms to have the option to show LOS without wasting resources on nodes for ones that don't
  */
  class LosCreator
  {
  public:
    virtual ~LosCreator() {}
    /// Creates a new RadialLOSNode which is owned by the caller
    virtual RadialLOSNode* newLosNode() = 0;
  };

  /**
  * Node that represents the platform model and all its attachments.
  * Note! The PlatformNode itself doesn't have any actual geometry (and no
  * transform either - this is so attachments can decide whether they want
  * to be locator-relative, like a model, or absolute, like a track history).
  */
  class SDKVIS_EXPORT PlatformNode : public EntityNode
  {
  public:

    /**
    * Constructs a new platform node.
    * @param props   Initialization properties
    * @param dataStore Reference to the datastore that contains platform data
    * @param manager Filters platform TSPI points
    * @param trackParent Parent node for the track history, since it cannot be a child of the platform node
    * @param locator Locator that will position this platform
    * @param referenceYear The calculations for the Speed Rings Fixed Time preference needs the scenario reference year
    */
    PlatformNode(const simData::PlatformProperties& props,
                 const simData::DataStore& dataStore,
                 PlatformTspiFilterManager& manager,
                 osg::Group* trackParent,
                 Locator* locator = NULL,
                 int referenceYear = 1970);

    /**
    * Access to the node that renders the 3D model/icon
    * @return Platform model node
    */
    PlatformModelNode* getModel();

    /**
    * The track history trail node
    * @return track history node, or NULL if it doesn't exists yet
    */
    TrackHistoryNode* getTrackHistory();

    /**
    * Returns the Radar Cross Section of the platform.  Can be NULL.
    * @return the Radar Cross Section of the platform.  Can be NULL.
    */
    simCore::RadarCrossSectionPtr getRcs() const;

    /**
    * The last properties set on this object
    * @return Platform properties
    */
    const simData::PlatformProperties& getProperties() const { return lastProps_; }

    /**
    * Applies a new set of properties to this object
    * @param props New properties to apply
    */
    void setProperties(const simData::PlatformProperties& props);

    /**
    * The last set of preferences applied to this node
    * @return Platform preferences
    */
    const simData::PlatformPrefs& getPrefs() const { return lastPrefs_; }

    /**
    * Applies a new set of preferences to this node
    * @param prefs New preferences to apply
    */
    void setPrefs(const simData::PlatformPrefs& prefs);

    /**
    * Sets a custom callback that will be used to generate the string that goes in the label.
    * @param callback Callback that will generate content; if NULL will only display platform name/alias
    */
    void setLabelContentCallback(LabelContentCallback* callback);

    /// Returns current content callback
    LabelContentCallback* labelContentCallback() const;

    /// Returns the pop up text based on the label content callback, update and preference
    std::string popupText() const;

    /// Set the creator for the LOS nodes
    void setLosCreator(LosCreator* losCreator);

  public: // EntityNode interface

    /**
    * Whether the entity is active within the scenario at the current time.
    * The entity is considered active if it has a valid position update for the
    * current scenario time, and has not received a command to turn off
    * @return true if active; false if not
    */
    virtual bool isActive() const;

    /** Gets the unique ID of the platform driving this node. */
    virtual simData::ObjectId getId() const;

    /** Platform has no host. */
    virtual bool getHostId(simData::ObjectId& out_hostId) const { return false; }

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
    * @param updateSlice  Data store update slice (could be NULL
    * @param force true to force the update to be applied; false only apply if logic dictates
    * @return true if update applied, false if not
    */
    virtual bool updateFromDataStore(const simData::DataSliceBase* updateSlice, bool force=false);

    /**
    * Notifies the platform of a clock mode update.
    * override from EntityNode.
    */
    virtual void updateClockMode(const simCore::Clock* clock);

    /**
    * Flushes all the entity's data point visualization
    */
    virtual void flush();

    /**
    * Returns a range value (meters) used for visualization.  Will return zero for platforms and projectors.
    */
    virtual double range() const;

    /** Retrieve the object index tag for platforms. */
    virtual unsigned int objectIndexTag() const;

    /**
    * Gets the traversal mask for this node type
    */
    static unsigned int getMask() { return simVis::DISPLAY_MASK_PLATFORM; }

    /**
    * Gets the actual bounds of the platform model
    * @return A bounding box in object space
    */
    const osg::BoundingBox& getActualSize() const { return unscaledModelBounds_; }

    /**
    * Gets the bounds of the platform model as displayed (possibly scaled)
    * @return A bounding box in object space
    */
    const osg::BoundingBox& getVisualSize() const { return scaledModelBounds_; }

    /**
    * Gets offset to the front of the scaled platform model
    * along the X axis, in model units (typically meters).  Used
    * primarily to implement the Missile Offset in beam.
    * @return Offset along X axis to front of model for missile offset.
    */
    double getFrontOffset() const { return frontOffset_; }

    /**
    * Returns the last update for the platform
    * @return the last update for the platform, or NULL if platform has no valid update
    */
    const simData::PlatformUpdate* update() const;

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "PlatformNode"; }

  protected:
    /// osg::Referenced-derived
    virtual ~PlatformNode();

  private:
    /** Copy constructor, not implemented or available. */
    PlatformNode(const PlatformNode&);

    /**
    * Indicates if the platform is currently active in the scenario, which is determined by if it has current data and if dataDrarw is set to on
    * @param prefs current pref values to determine the dataDraw state
    * @return true if platform has data at current scenario time and dataDraw is true
    */
    bool isActive_(const simData::PlatformPrefs& prefs) const;

    /**
    * Indicates if the track history should exist in the scene, based on the expireMode, if the platform is active, non-static, and track history draw state is valid
    * @param prefs pref values to interrogate
    * @return true if the track history should exist in the scene
    */
    bool showTrack_(const simData::PlatformPrefs& prefs) const;

    /**
    * Mark the platform as not valid; receipt of a valid datastore update will make the platform valid again
    */
    void setInvalid_();

    /**
    * Indicates if the track history should show based on the expireMode flag.
    * @param prefs current pref values to determine the expireMode
    * @return true if expireMode is set to true and scenario time is > last platform history point
    */
    bool showExpiredTrackHistory_(const simData::PlatformPrefs& prefs) const;
    void setRcsPrefs_(const simData::PlatformPrefs& prefs);
    void updateLocator_(const simData::PlatformUpdate& u);
    void updateHostBounds_(double scale);
    void updateLabel_(const simData::PlatformPrefs& prefs);
    bool createTrackHistoryNode_(const simData::PlatformPrefs& prefs);
    void updateOrRemoveBodyAxis_(bool prefsDraw, const simData::PlatformPrefs& prefs);
    void updateOrRemoveInertialAxis_(bool prefsDraw, const simData::PlatformPrefs& prefs);
    void updateOrRemoveVelocityVector_(bool prefsDraw, const simData::PlatformPrefs& prefs);
    void updateOrRemoveEphemerisVector_(bool prefsDraw, const simData::PlatformPrefs& prefs);
    void updateOrRemoveCircleHighlight_(bool prefsDraw, const simData::PlatformPrefs& prefs);
    void updateOrRemoveHorizons_(const simData::PlatformPrefs& prefs);
    void updateOrRemoveHorizon_(simCore::HorizonCalculations horizonType , const simData::PlatformPrefs& prefs);

    const simData::DataStore&       ds_;
    PlatformTspiFilterManager&      platformTspiFilterManager_;
    simCore::RadarCrossSectionPtr   rcs_;
    simData::PlatformProperties     lastProps_;
    simData::PlatformPrefs          lastPrefs_;
    simData::PlatformUpdate         lastUpdate_;
    /// the last time a data store update came in
    double                          lastUpdateTime_;
    /// the time of the earliest history point that still exists in the data slice
    double                          firstHistoryTime_;
    /// parent of the track history points, which must be different from the platform node to support expiremode
    osg::observer_ptr<osg::Group>   trackParent_;
    /// track history points
    osg::ref_ptr<TrackHistoryNode>  track_;
    osg::ref_ptr<LocalGridNode>     localGrid_;
    osg::ref_ptr<AreaHighlightNode> areaHighlight_;
    osg::ref_ptr<AxisVector>        bodyAxisVector_;
    osg::ref_ptr<AxisVector>        inertialAxisVector_;
    osg::ref_ptr<PlatformInertialTransform> scaledInertialTransform_;
    osg::ref_ptr<VelocityVector>    velocityAxisVector_;
    osg::ref_ptr<EphemerisVector>   ephemerisVector_;
    osg::ref_ptr<PlatformModelNode> model_;
    osg::ref_ptr<LabelContentCallback> contentCallback_;
    LosCreator*                     losCreator_; // Not owned
    RadialLOSNode*                  opticalLosNode_;
    RadialLOSNode*                  radioLosNode_;
    osg::BoundingBox                unscaledModelBounds_;
    osg::BoundingBox                scaledModelBounds_;
    double                          frontOffset_;
    /// flag indicating if the platform node has valid data for the current scenario time
    bool                            valid_;
    /// flag indicating that the lastPrefs_ field is valid
    bool                            lastPrefsValid_;
    /// force next update from data store to be processed, even if !slice->hasChanged()
    bool                            forceUpdateFromDataStore_;
    /// queue up the invalidate to apply on the next data store update
    bool                            queuedInvalidate_;
  };

} // namespace simVis


#endif // SIMVIS_PLATFORM_NODE_H
