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
#ifndef SIMVIS_LASER_H
#define SIMVIS_LASER_H

#include "osg/Geometry"

#include "simVis/Constants.h"
#include "simVis/Entity.h"
#include "simVis/LocalGrid.h"
#include "simVis/EntityLabel.h"
#include "simVis/LabelContentManager.h"

namespace simVis
{
  /// Scene graph node that renders a Laser
  class SDKVIS_EXPORT LaserNode : public EntityNode
  {
  public:
    /**
    * Construct a new node that displays a Laser.
    *
    * @param props Initial laser properties
    * @param locator Parent locator from which this laser's locator should inherit
    * @param host This laser's host platform
    * @param referenceYear The calculation for the Speed Rings Fixed Time preference needs the scenario reference year
    */
    LaserNode(
      const simData::LaserProperties& props,
      Locator*                        locator = NULL,
      const simVis::EntityNode*       host = NULL,
      int                             referenceYear = 1970);

    /**
    * Access the properties object currently representing this laser.
    *
    * @return Current properties
    */
    const simData::LaserProperties& getProperties() const { return lastProps_; }

    /**
    * Access to last known preferences.
    *
    * @return Current preferences
    */
    const simData::LaserPrefs& getPrefs() const { return lastPrefs_; }

    /**
    * Apply new preferences, replacing any existing prefs.
    *
    * @param prefs New preferences to apply
    */
    void setPrefs(const simData::LaserPrefs& prefs);

    /**
    * Sets a custom callback that will be used to generate the string that goes in the label.
    * @param callback Callback that will generate content; if NULL will only display platform name/alias
    */
    void setLabelContentCallback(LabelContentCallback* callback);

    /// Returns current content callback
    LabelContentCallback* labelContentCallback() const;

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
    * Get the object ID of the entity rendered by this node
    * @return Object ID
    */
    virtual simData::ObjectId getId() const;

    /** Get the laser's host's ID */
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

    /**
     * Gets a pointer to the last data store update, or NULL if
     * none have been applied.
     */
    const simData::LaserUpdate* getLastUpdateFromDS() const;

    /**
    * Get the traversal mask for this node type
    * @return a traversal mask
    */
    static unsigned int getMask() { return simVis::DISPLAY_MASK_LASER; }

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "LaserNode"; }

  protected:
    /// osg::Referenced-derived
    virtual ~LaserNode() {}

  private: // methods
    void refresh_(const simData::LaserUpdate* update, const simData::LaserPrefs* prefs);

    /**
    * Updates the locator if required, based on specified arguments
    * @param newUpdate new update data (could be NULL)
    * @param newPrefs new prefs settings (could be NULL)
    * @param force  true to force locator update regardless of other params
    */
    void updateLocator_(const simData::LaserUpdate* newUpdate, const simData::LaserPrefs* newPrefs, bool force);

    osg::Geode* createGeometry_(const simData::LaserPrefs& prefs);

  private: // data
    simData::LaserProperties  lastProps_;      ///< laser properties
    simData::LaserPrefs       lastPrefs_;      ///< latest copy of prefs received
    simData::LaserUpdate      lastUpdate_;     ///< last data update
    bool                      hasLastUpdate_;  ///< is there anything in lastUpdate_
    osg::ref_ptr<LocatorNode> locatorNode_;    ///< the parent node for all laser-related graphics
    osg::ref_ptr<Locator>     laserXYZOffsetLocator_; ///< extra locator used only for non-relative lasers
    bool                      visible_;        ///< whether the prefs indicate that this laser should be drawn
    osg::ref_ptr<osg::Geode>  node_;           ///< the node that contains the actual laser geometry
    osg::observer_ptr<const EntityNode> host_; ///< the platform that hosts this laser
    osg::ref_ptr<LocalGridNode> localGrid_;    ///< the localgrid node for this laser
    bool hasLastPrefs_;                        ///< Whether lastPrefs_ has been set by prefs we received

    void updateLabel_(const simData::LaserPrefs& prefs);
    osg::ref_ptr<EntityLabelNode> label_;
    osg::ref_ptr<LabelContentCallback> contentCallback_;

  };

} //namespace simVis

#endif // SIMVIS_LASER_H

