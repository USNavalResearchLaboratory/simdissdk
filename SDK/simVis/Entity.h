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
#ifndef SIMVIS_ENTITY_NODE_H
#define SIMVIS_ENTITY_NODE_H

#include "osg/observer_ptr"
#include "osgEarth/MapNode"
#include "simVis/Locator.h"
#include "simData/DataStore.h"
#include <vector>

namespace osgEarth {
  class Map;
}

namespace simCore {
  class Clock;
}

namespace simVis
{
  /**
   * Pure interface class for a node that you can call EntityNode::attach with
   * and that will communicate tracking components
   */
  class EntityAttachable
  {
  public:
    /** Gets the mask of components to inherit (see Locator::Components) */
    virtual unsigned int getLocatorComponents() const = 0;

    virtual ~EntityAttachable() {}
  };

  /** Helper class for entity's to help clamp coordinate points to map surface */
  class SDKVIS_EXPORT CoordSurfaceClamping
  {
  public:
    CoordSurfaceClamping();
    virtual ~CoordSurfaceClamping();

    /**
    * reset coordinate's altitude value to be clamped to the surface. Queries the terrain for
    * elevation at the specified coordinate position. Coordinate must be in LLA or ECEF
    * @param coord  coordinate to clamp to map surface, will do nothing if coordinate is not LLA or ECEF
    */
    void clampCoordToMapSurface(simCore::Coordinate& coord);

    /** Return true if able to apply clamping, false otherwise */
    bool isValid() const;

    /** Set the map node for querying terrain height for clamping */
    void setMapNode(const osgEarth::MapNode* map);

  private:
    osg::observer_ptr<const osgEarth::MapNode> mapNode_;
  };

  /**
  * Scene graph node that represents an entity, with its EntityAttachments as children.
  */
  class SDKVIS_EXPORT EntityNode : public osg::Group
  {
  protected:
    /**
    * Constructs a new entity node (from subclass only)
    * @param type Type of entity node to alleviate the need for dynamic casting
    * @param locator Locator that will position this node
    */
    EntityNode(simData::DataStore::ObjectType type, Locator* locator = NULL);

  public:
    /** Enumerates different name types for the label */
    enum NameType {
      REAL_NAME,  ///< Real entity name
      ALIAS_NAME,  ///< Alias name
      DISPLAY_NAME,  ///< Real or entity name based on usealias pref
    };

    /**
    * Returns the type of entity
    */
    simData::DataStore::ObjectType type() const { return type_; }

    /**
    * Whether the entity is active within the scenario at the current time.
    * The entity is considered active if it has a valid position update for the
    * current scenario time. It may or may not be visible.
    * @return true if active; false if not
    */
    virtual bool isActive() const { return getNodeMask() != 0; }

    /**
    * Whether this entity is visible.
    */
    virtual bool isVisible() const;

    /**
    * Returns the entity name. Can be used to get the actual name always or the
    * actual/alias depending on the commonprefs.usealias flag.
    * @param nameType  enum option to always return real/alias name or name based on
    *            the commonprefs usealias flag.
    * @param allowBlankAlias If true DISPLAY_NAME will return blank if usealias is true and alias is blank
    * @return    actual/alias entity name string
    */
    virtual const std::string getEntityName(NameType nameType, bool allowBlankAlias = false) const = 0;

    /// Returns the hook text based on the label content callback, update and preference
    virtual std::string hookText() const = 0;

    /// Returns the legend text based on the label content callback, update and preference
    virtual std::string legendText() const = 0;

    /**
     * Attaches a tracking child to this entity. The child will track
     * the entity's locator according to the Locator properties.
     *
     * @param node Node to attach to this platform
     * @param comp Tracking components mask to use (see Locator::Components)
     */
    void attach(osg::Node* node, unsigned int comp);

    /**
     * Attaches a tracking child to this entity. The child will track
     * the entity's locator according to the Locator properties.
     *
     * @param node Object to attach to this platform. If the node implements
     *             The EntityAttachable interface, it will take the tracking
     *             components from there. Otherwise all components track by
     *             default.
     */
    void attach(osg::Node* node);

    /**
    * Locates an attachment of the specified type.
    * @return The attachment node, or null if not found
    */
    template<typename T>
    T* findAttachment()
    {
      for (unsigned int i = 0; i < getNumChildren(); ++i)
      {
        osg::Node* node = getChild(i);
        if (dynamic_cast<T*>(node))
          return static_cast<T*>(node);
      }
      return NULL;
    }

    /**
    * Locates an attachment of the specified type using the given parent node.
    * @return The attachment node, or null if not found
    */
    template<typename T>
    T* findAttachment(osg::Group* parent)
    {
      for (unsigned int i = 0; i < parent->getNumChildren(); ++i)
      {
        osg::Node* node = parent->getChild(i);
        if (dynamic_cast<T*>(node))
          return static_cast<T*>(node);
      }
      return NULL;
    }

    /**
    * Sets the locator to store in this entity. This node does not actually do anything
    * with the locator; rather it is a useful place to store its reference.
    * @param locator Locator to store
    */
    void setLocator(Locator* locator);

    /**
    * Gets the locator stored in this entity node
    * @return Stored locator
    */
    Locator* getLocator() { return locator_.get(); }

    /**
    * Gets the locator stores in this entity node
    * @return Stored locator
    */
    const Locator* getLocator() const { return locator_.get(); }

    /**
    * Gets the unique ID of the database entity underlying this node.
    * @return The object ID
    */
    virtual simData::ObjectId getId() const = 0;

    /**
     * Gets the unique ID of this entity's Host object, if there is one.
     * @param[out] hostId Host ID is one exists
     * @return     True if the output param is set.
     */
    virtual bool getHostId(simData::ObjectId& hostId) const = 0;

    /**
    * Updates the entity based on the bound data store. The implementation class
    * must provide this method.
    * @param updateSlice  Data store update slice (could be NULL)
    * @param force true to force the update to be applied; false allows entity to use its own internal logic to decide whether the update should be applied
    * @return true if update applied, false if not
    */
    virtual bool updateFromDataStore(const simData::DataSliceBase* updateSlice, bool force=false) = 0;

    /**
    * Notify the entity of a clock mode update. The implementation may
    * optionally override this method to respond to a mode change.
    * @param clock Clock pointer that contains current clock mode and other clock data
    */
    virtual void updateClockMode(const simCore::Clock* clock) { }

    /**
    * Flushes all the entity's data point visualization
    */
    virtual void flush() = 0;

    /**
    * Returns a range value (meters) used for visualization.  Will return zero for platforms and projectors.
    */
    virtual double range() const = 0;

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "EntityNode"; }

  protected:
    virtual ~EntityNode();

  private:
    simData::DataStore::ObjectType type_;
    osg::ref_ptr<Locator> locator_;
  };

  /** Vector of EntityNode ref_ptr */
  typedef std::vector< osg::ref_ptr<EntityNode> >      EntityVector;
  /** Vector of EntityNode observer_ptr */
  typedef std::vector< osg::observer_ptr<EntityNode> > EntityObserverVector;

} // namespace simVis

#endif // SIMVIS_PLATFORM_NODE_H

