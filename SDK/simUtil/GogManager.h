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
#ifndef SIMUTIL_GOGMANAGER_H
#define SIMUTIL_GOGMANAGER_H

#include <string>
#include <vector>
#include <osg/Vec3d>
#include <osg/Vec4f>
#include "simCore/Common/Common.h"
#include "simVis/GOG/GogNodeInterface.h"

namespace osg { class Vec4f; }
namespace simCore { class Vec3; }

namespace simUtil {

/** List of GOG overlay interface objects that wrap each shape in the GOG */
typedef std::vector<simVis::GOG::GogNodeInterfacePtr> OverlayNodeVector;

/// Shared ptr wrapper for the GogNodeInterface
typedef std::shared_ptr<simVis::GOG::GogNodeInterface> GogNodeInterfacePtr;


/** Interface for the object representing a GOG file loaded by the GogManager */
class SDKUTIL_EXPORT GogObject
{
public:
  /// Definition of the GOG draw state.
  enum DrawState
  {
    ON = 0, ///< all child shape nodes are drawn
    OFF,    ///< all child shape nodes are not drawn
    PARTIAL ///< some child shapes nodes are drawn, but not all
  };

  /** Observer class to listen to the GogObject for when a property is changed */
  class GogObjectObserver
  {
  public:
    virtual ~GogObjectObserver() {}
    /** Passes the gog that was changed */
    virtual void propertyChanged(const GogObject& gogObject) = 0;
    /** Passes the gog whose draw state changed */
    virtual void drawChanged(const GogObject& gogObject) = 0;
  };

  /** Shared pointer to GOG Object Observer */
  typedef std::shared_ptr<GogObjectObserver> GogObjectObserverPtr;

  virtual ~GogObject() {}

  /**
  * Add a GogObjectObserver to be notified of GOG Object changes
  * @param observer an observer to be added
  */
  void addGogObjectObserver(GogObjectObserverPtr observer);

  /**
  * Remove a GogObjectObserver
  * @param observer an observer to be removed
  */
  void removeGogObjectObserver(GogObjectObserverPtr observer);

  /**
  * Retrieve the GOG's attached platform id from the DataStore, returns 0 if not attached
  * @return DataStore platform id
  */
  virtual simData::ObjectId attachedTo() const = 0;

  /**
  * Retrieve the GOG's file name, as passed into the GogManager (should be full path)
  * @return gog file name
  */
  virtual std::string fileName() const = 0;

  /**
  * Retrieve the GOG's shape nodes
  * @param shapeNodes vector of the OverlayNodeObjects
  */
  virtual void getShapeNodes(OverlayNodeVector& shapeNodes) const = 0;

  /** Return the shared ptr if this shape is managed by this GogObject. Returns an invalid shared ptr otherwise */
  virtual GogNodeInterfacePtr getShapePtr(const simVis::GOG::GogNodeInterface* shapeId) = 0;

  /**
  * Get the shape's current position. If the shape is attached to a platform, the platform's position must be used as
  * a reference position. Does nothing if this shape doesn't belong to this GOG.
  * Fills in position parameter in osgEarth format: lon/lat/alt, deg/deg/meters
  * @param shapeNode  node for which position is to be returned
  * @param position to be filled in if the shape's position was found, osgEarth format: lon/lat/alt, deg/deg/meters
  * @return 0 if a position was found, non-zero otherwise
  */
  virtual int getShapePosition(const simVis::GOG::GogNodeInterface* shapeNode, osg::Vec3d& position) const = 0;

  /**
  * Indicates if the GOG is attached to a platform
  * @return true if GOG is attached to a platform, false otherwise
  */
  virtual bool isAttached() const = 0;

  /**
  * Remove this GOG shape node from the GogObject
  * @param shapeNode shape node to remove
  */
  virtual void removeShape(const simVis::GOG::GogNodeInterface* shapeNode) = 0;

  /**
  * Serialize the GOG into the provided stream. Serializes into GOG file format
  * @param gogOutputStream  ostream to hold serialized GOG
  */
  virtual void serializeToStream(std::ostream& gogOutputStream) const = 0;

  /**
  * Update the draw state of the GOG. Will apply the draw state to all shape nodes
  * @param draw  new draw state
  */
  virtual void setDrawState(bool draw) = 0;

  /**
  * Defines the current draw state of the GOG, which depends on the draw state of all the child shape nodes
  * @return the current draw state of the whole GOG
  */
  virtual DrawState getDrawState() const = 0;

  /**
  * Update the fill color of the GOG. Will apply the fill color to all shape nodes that support filled.
  * This will not automatically set the nodes to be filled, just update their fill color, which will
  * only display if they are filled or toggled to filled
  * @param fillColor  in osg format (r,g,b,a) between 0.0 - 1.0
  */
  virtual void setFillColor(const osg::Vec4f& fillColor) = 0;

  /**
  * Update the line color of the GOG. Will apply the line color to all shape nodes that support line
  * @param lineColor  in osg format (r,g,b,a) between 0.0 - 1.0
  */
  virtual void setLineColor(const osg::Vec4f& lineColor) = 0;

  /**
   * Sets the orientation offsets for the GOG in radians; attached GOGs only.
   * @param followYaw if true, set the GOG to follow the platform's yaw(heading)
   * @param followPitch if true, set the GOG to follow the platform's pitch
   * @param followRoll if true, set the GOG to follow the platform's roll
   * @param yprOffsets Orientation offsets in yaw, pitch, and roll, in radians
   * @return 0 on success, non-zero on error (including attempt to call for a non-attached GOG)
   */
  virtual int setOrientationOffsets(bool followYaw, bool followPitch, bool followRoll, const simCore::Vec3& yprOffsets) = 0;

  /**
   * Retrieves the orientation offsets for the GOG in radians; attached GOGs only.
   * @param followYaw Set true if locked on to host platform's yaw
   * @param followPitch Set true if locked on to host platform's pitch
   * @param followRoll Set true if locked on to host platform's roll
   * @param yprOffsets Yaw, Pitch, Roll orientation offset in radians
   * @return 0 on success, non-zero on error (including attempt to call for a non-attached GOG)
   */
  virtual int orientationOffsets(bool& followYaw, bool& followPitch, bool& followRoll, simCore::Vec3& yprOffsets) const = 0;

protected:
  /** Informs all observers that a GOG property was changed. */
  void firePropertyChanged_() const;
  /** Informs all observers that the GOG's draw state was changed. */
  void fireDrawChanged_() const;

private:
  std::vector<GogObjectObserverPtr> observers_;  ///< vector of all observers
};
/// Shared ptr wrapper for the GogObject
typedef std::shared_ptr<GogObject> GogObjectPtr;

/** Interface for loading GOG files into a visualization */
class GogManager
{
public:
  /** Observer class to listen to the GogManager for new or removed GOGs */
  class GogChangeObserver
  {
  public:
    virtual ~GogChangeObserver() {}
    /** Passes newly added GOGs */
    virtual void addGogs(const std::vector<GogObjectPtr>& addedGogs) = 0;
    /** Passes GOGs about to be removed. The GogObject pointers are still valid memory when this is called */
    virtual void aboutToRemoveGogs(const std::vector<GogObjectPtr>& removedGogs) = 0;
    /** Passes affected GOG and shape */
    virtual void aboutToRemoveShape(GogObjectPtr parentGog, GogNodeInterfacePtr removedShape) = 0;
    /** Passes GOGs after they've been removed from the GogManager. The GogObject pointers are still valid when this is called */
    virtual void removedGogs(const std::vector<const GogObject*>& removedGogs) = 0;
  };

  /** Shared pointer to GOG Change Observer */
  typedef std::shared_ptr<GogChangeObserver> GogChangeObserverPtr;

  virtual ~GogManager() {}

  /**
  * Retrieve the GogObject that defines the specified gogFile, if it exists in the GogManager. GOG may be attached or absolute.
  * In the case that the same GOG file has been loaded multiple times, will return the first item found in the manager
  * @param gogFile  file name of the GOG
  * @return GogObject  loaded GOG in shared ptr
  */
  virtual GogObjectPtr getGog(const std::string& gogFile) const = 0;

  /**
  * Retrieve the GogObject that defines the specified attached gogFile, attached to the specified hostId, if it exists in the GogManager
  * @param gogFile  file name of the GOG
  * @param hostId  data store id of the host platform
  * @return GogObject  loaded GOG in shared ptr
  */
  virtual GogObjectPtr getAttachedGog(const std::string& gogFile, simData::ObjectId hostId) const = 0;

  /** Returns the shared ptr to this GogObject if it is managed. Returns an invalid shared ptr otherwise */
  virtual GogObjectPtr getGogPtr(const simUtil::GogObject* gogObjectId) const = 0;

  /**
  * Retrieve all loaded GOGs. Only returns finalized GOGs, not provisional
  * @param loadedGogs Will contain vector of all loaded GogObjects
  */
  virtual void getLoadedGogs(std::vector<simUtil::GogObjectPtr>& loadedGogs) const = 0;

  /** Identifies if this GogObject is valid in the GogManager, either finalized or provisional */
  virtual bool isValidGog(const simUtil::GogObject* gogObject) const = 0;

  /**
  * Identifies if this GogObject is a provisional GOG.
  * Provisional GOGs are not part of the scenario, rather they are visualization only. Provisional GOGs will not
  * appear as loaded in the GogManager, so they will not be returned by getLoadedGogs, and they will not
  * initiate any notifications in the GogObserver.
  * Provisional GOGs will return true for isValidGog, as they are being managed by the GogManager
  */
  virtual bool isProvisionalGog(const simUtil::GogObject* gogObject) const = 0;

  /**
  * Load Absolute GOG as finalized or provisional. The finalized GOGs are loaded into the scenario,
  * while provisional GOGs are not, rather they are visualization only. Provisional GOGs will not
  * appear as loaded in the GogManager, so they will not be returned by getLoadedGogs, and they will not
  * initiate any notifications in the GogObserver.
  * Provisional GOGs will return true for isValidGog, as they are being managed by the GogManager
  * @param gogFile  file to load
  * @param finalized  true if this is a finalized GOG, false if provisional
  * @return GogObject  loaded GOG in shared ptr
  */
  virtual GogObjectPtr loadGog(const std::string& gogFile, bool finalized) = 0;

  /**
  * Load GOG attached to platform as finalized or provisional. The finalized GOGs are loaded into the scenario,
  * while provisional GOGs are not, rather they are visualization only. Provisional GOGs will not
  * appear as loaded in the GogManager, so they will not be returned by getLoadedGogs, and they will not
  * initiate any notifications in the GogObserver.
  * Provisional GOGs will return true for isValidGog, as they are being managed by the GogManager
  * @param gogFile  file to load
  * @param platformId  host platform
  * @param finalized  true if this is a finalized GOG, false if provisional
  * @return GogObject  loaded GOG in shared ptr
  */
  virtual GogObjectPtr loadAttachedGog(const std::string& gogFile, uint64_t platformId, bool finalized) = 0;

  /**
  * Load a provisional GOG from a stream
  * @param input  streamed gog
  * @param platformId Host platform for attached GOGs; leave as 0 for non-attached GOGs
  * @return GogObject loaded GOG in shared ptr
  */
  virtual GogObjectPtr loadGog(std::istream& input, uint64_t platformId) = 0;

  /**
  * Remove GOG
  * @param gog  GOG to remove
  * @return 0 if found GOG to delete, non-zero otherwise
  */
  virtual int deleteGog(const GogObject* gog) = 0;

  /**
  * Remove all GOGs, including provisional GOGs
  */
  virtual void deleteAllGogs() = 0;

  /**
  * Add a GogChangeObserver to be notified of GOG changes
  * @param observer an observer to be added
  */
  virtual void addGogObserver(GogChangeObserverPtr observer) = 0;

  /**
  * Remove a GogChangeObserver
  * @param observer an observer to be removed
  */
  virtual void removeGogObserver(GogChangeObserverPtr observer) = 0;

};

}

#endif /* SIMUTIL_GOGMANAGER_H */
