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
#ifndef SIMVIS_CUSTOM_RENDERING_H
#define SIMVIS_CUSTOM_RENDERING_H

#include "osg/observer_ptr"
#include "simData/DataTypes.h"
#include "simVis/Constants.h"
#include "simVis/Entity.h"
#include "simVis/LocatorNode.h"

namespace simVis
{
  class CustomLabelContentCallback;
  class EntityLabelNode;
  class LabelContentCallback;
  class LocalGridNode;
  class Locator;
  class OverrideColor;
  class ScenarioManager;

  /**
   * Node for Custom Rendering
   */
  class SDKVIS_EXPORT CustomRenderingNode : public EntityNode
  {
  public:
    /**
    * Construct a new node that displays a Custom.
    * @param scenario ScenarioManager that is managing this custom
    * @param props Initial custom properties
    * @param host This custom's host entity
    * @param referenceYear The calculation for the Speed Rings Fixed Time preference needs the scenario reference year
    */
    CustomRenderingNode(const ScenarioManager* scenario, const simData::CustomRenderingProperties& props,
      const simVis::EntityNode* host = NULL, int referenceYear = 1970);

    /**
    * Access the properties object currently representing this custom.
    *
    * @return Current properties
    */
    const simData::CustomRenderingProperties& getProperties() const { return lastProps_; }

    /**
    * Access to last known preferences.
    *
    * @return Current preferences
    */
    const simData::CustomRenderingPrefs& getPrefs() const { return lastPrefs_; }

    /**
    * Apply new preferences, replacing any existing prefs.
    * @param prefs New preferences to apply
    */
    void setPrefs(const simData::CustomRenderingPrefs& prefs);

    /**
    * Sets a custom callback that will be used to generate the string that goes in the label.
    * @param callback Callback that will generate content; if NULL will only display platform name/alias
    */
    void setLabelContentCallback(LabelContentCallback* callback);

    /// Returns current content callback
    LabelContentCallback* labelContentCallback() const;

    /**
     * This callback allows the external code to determine if the entity should be displayed.
     * If update() returns true the entity continues to be processed for displaying
     */
    class UpdateCallback : public osg::Referenced
    {
    public:
      /**
       * This callback allows the external code to determine if the entity should be displayed
       * @param updateSlice Currently not used
       * @param force true to force the update to be applied; false allows entity to use its own internal logic to decide whether the update should be applied.
       *              If a force update results in no graphics than false is still returned.
       * @return true if the entity should be displayed.
       */
      virtual bool update(const simData::DataSliceBase* updateSlice, bool force = false) = 0;

    protected:
      virtual ~UpdateCallback() {}
    };

    /** set the update callback */
    void setUpdateCallback(UpdateCallback* callback);

    /** Returns the update callback */
    UpdateCallback* updateCallback() const;

    /**
    * Returns a range value (meters) used for visualization.  Will return zero for platforms and projectors.
    */
    virtual double range() const;

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "CustomRenderingNode"; }

    // EntityNode interface
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
    * Get the object ID of the GPR rendered by this node
    * @return Custom's object ID
    */
    virtual simData::ObjectId getId() const;

    /** Get the custom's host's ID; returns true if out_hostId is set */
    virtual bool getHostId(simData::ObjectId& out_hostId) const;

    /**
    * Returns the entity name. Can be used to get the actual name always or the
    * actual/alias depending on the commonprefs.usealias flag.
    * @param nameType  enum option to always return real/alias name or name based on
    *            the commonprefs usealias flag.
    * @param allowBlankAlias If true DISPLAY_NAME will return blank if usealias is true and alias is blank
    * @return actual/alias entity name string
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


    /** This entity type is, at this time, unpickable. */
    virtual unsigned int objectIndexTag() const;

    /**
    * Gets the world position for this custom's origin. This is a convenience
    * function that extracts the Position information (not rotation) from the underlying locatorNode matrix.
    * @param[out] out_position If not NULL, resulting position stored here, in coordinate system as specified by coordsys
    * @param[in ] coordsys Requested coord sys of the output position (only LLA, ECEF, or ECI supported)
    * @return 0 if the output parameter is populated successfully, nonzero on failure
    */
    virtual int getPosition(simCore::Vec3* out_position, simCore::CoordinateSystem coordsys = simCore::COORD_SYS_ECEF) const;

    /**
    * Gets the world position & orientation for this custom's origin. This is a convenience
    * function that extracts the Position information and rotation from the underlying locatorNode matrix.
    * @param[out] out_position If not NULL, resulting position stored here, in coordinate system as specified by coordsys
    * @param[out] out_orientation If not NULL, resulting orientation stored here, in coordinate system as specified by coordsys
    * @param[in ] coordsys Requested coord sys of the output position (only LLA, ECEF, or ECI supported)
    * @return 0 if the output parameter is populated successfully, nonzero on failure
    */
    virtual int getPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation,
      simCore::CoordinateSystem coordsys = simCore::COORD_SYS_ECEF) const;

    /**
    * Get the traversal mask for this node type
    * @return a traversal mask
    */
    static unsigned int getMask() { return simVis::DISPLAY_MASK_CUSTOM_RENDERING; }

    // Method unique to Custom

    // An outside source can control if the custom is active
    bool customActive() const;
    void setCustomActive(bool value);

    // Expose the locator node so an outside source can add graphics.
    LocatorNode* locatorNode() const;

    /// Returns the host
    const EntityNode* host() const;

    /// Returns the pop up text based on the label content callback, update and preference
    std::string popupText() const;

  protected:
    /// osg::Referenced-derived; destructor body needs to be in the .cpp
    virtual ~CustomRenderingNode();

  private:
    /** Copy constructor, not implemented or available. */
    CustomRenderingNode(const CustomRenderingNode&);

    /**
    * Update the custom label with the specified custom preferences
    * @param prefs the custom preferences to update
    */
    void updateLabel_(const simData::CustomRenderingPrefs& prefs);

    /**
    * Update the color with the specified custom rendering preferences
    * @param prefs the custom rendering preferences
    */
    void updateOverrideColor_(const simData::CustomRenderingPrefs& prefs);

    osg::observer_ptr<const ScenarioManager> scenario_;
    osg::observer_ptr<const EntityNode> host_;
    osg::ref_ptr<LabelContentCallback> contentCallback_;
    osg::ref_ptr<UpdateCallback> updateCallback_;
    osg::ref_ptr<LocalGridNode> localGrid_;
    osg::ref_ptr<EntityLabelNode> label_;
    osg::ref_ptr<LocatorNode>  customLocatorNode_;
    osg::ref_ptr<OverrideColor> overrideColor_;
    simData::CustomRenderingProperties lastProps_;
    simData::CustomRenderingPrefs lastPrefs_;
    bool hasLastPrefs_;
    bool customActive_;
    unsigned int objectIndexTag_;
  };

} //namespace simVis

#endif //SIMVIS_CUSTOM_H

