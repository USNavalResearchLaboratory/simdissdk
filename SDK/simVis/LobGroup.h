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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_LOB_GROUP_H
#define SIMVIS_LOB_GROUP_H

#include "simData/DataTypes.h"
#include "simVis/Constants.h"
#include "simVis/Entity.h"

namespace osg { class MatrixTransform; }
namespace simCore { class CoordinateConverter; }
namespace simData {
  class DataStore;
  class DataTable;
}

namespace simVis
{
class AnimatedLineNode;
class EntityLabelNode;
class LocalGridNode;

/**
 * Scene graph node that renders a group of "Lines of Bearing" (LOB)
 *
 * Each line is drawn from a platform position in time to some az/el/range.
 * Lines are drawn as historical data, and there can be multiple lines at the
 * same time.  All the lines for a group have the same drawing attributes
 * (color, width, etc)
 */
class SDKVIS_EXPORT LobGroupNode : public EntityNode
{
public:
  /**
  * Construct a new node that displays a LobGroup
  * @param props Initial properties
  * @param host Entity from which this should be based
  * @param surfaceClamping handles clamping LOB coordinates to map surface
  * @param ds Data store this LOB is associated with
  */
  LobGroupNode(const simData::LobGroupProperties& props,
               EntityNode*                        host,
               CoordSurfaceClamping*              surfaceClamping,
               simData::DataStore&                ds);

  /**
    * Installs the global LOB shader program and initializes the default uniform variables
    * for the shader into the StateSet provided.  This is required in the scene graph somewhere
    * at or above the LOBs in order for blinking to work.
    */
  static void installShaderProgram(osg::StateSet* intoStateSet);

  /**
  * Apply new preferences, replacing any existing prefs
  *
  * @param prefs New preferences to apply
  */
  void setPrefs(const simData::LobGroupPrefs &prefs);

  /** Retrieves the currently visible end points */
  void getVisibleEndPoints(std::vector<osg::Vec3d>& ecefVec) const;

  /**
  * Get the traversal mask for this node type
  * @return a traversal mask
  */
  static unsigned int getMask() { return simVis::DISPLAY_MASK_LOB_GROUP; }

  /**
  * Returns the last update for the LOB Group
  * @return the last update for the LOB Group
  */
  const simData::LobGroupUpdate* update() const { return &lastUpdate_; }

public: // EntityNode interface
  /**
  * Whether the entity is active within the scenario at the current time.
  * The entity is considered active if it has a valid position update for the
  * current scenario time, and has not received a command to turn off
  * @return true if active; false if not
  */
  virtual bool isActive() const override;

  /**
  * Returns the entity name. Can be used to get the actual name always or the
  * actual/alias depending on the commonprefs.usealias flag.
  * @param nameType  enum option to always return real/alias name or name based on
  *            the commonprefs usealias flag.
  * @param allowBlankAlias If true DISPLAY_NAME will return blank if usealias is true and alias is blank
  * @return    actual/alias entity name string
  */
  virtual const std::string getEntityName(EntityNode::NameType nameType, bool allowBlankAlias = false) const override;

  /// Returns the pop up text based on the label content callback, update and preference
  virtual std::string popupText() const override;
  /// Returns the hook text based on the label content callback, update and preference
  virtual std::string hookText() const override;
  /// Returns the legend text based on the label content callback, update and preference
  virtual std::string legendText() const override;

  /**
  * Gets the unique ID of the database entity underlying this node.
  * @return The object ID
  */
  virtual simData::ObjectId getId() const override;

  /** Get the lob's host's ID */
  virtual bool getHostId(simData::ObjectId& out_hostId) const override;

  /**
  * Update this based on the slice from the data store.
  * @param updateSlice  Data store update slice (could be nullptr)
  * @param force true to force the update to be applied; false allows entity to use its own internal logic to decide whether the update should be applied
  * @return true if update applied, false if not
  */
  virtual bool updateFromDataStore(const simData::DataSliceBase *updateSlice, bool force=false) override;

  /**
  * Flushes all the entity's data point visualization
  */
  virtual void flush() override;

  /**
  * Returns a range value used for visualization.  Will return zero for platforms and projectors.
  */
  virtual double range() const override;

  /** This entity type is, at this time, unpickable. */
  virtual unsigned int objectIndexTag() const override;

  /** Return the proper library name */
  virtual const char* libraryName() const  override { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const  override { return "LobGroupNode"; }

private: // types
  class Cache;

private: // methods
  /** Copy constructor, not implemented or available. */
  LobGroupNode(const LobGroupNode&);
  /** Assignment operator, not implemented or available. */
  LobGroupNode& operator=(const LobGroupNode&);

  /// osg::Referenced-derived
  virtual ~LobGroupNode();

  /// apply clamping to this platform coordinate. Assumes coord is ECEF. Will update the coordinate converter ref lla
  void applyPlatformCoordClamping_(simCore::Coordinate& platformCoord);
  /// apply clamping to this endpoint coordinate. Assumes coord is XEAST
  void applyEndpointCoordClamping_(simCore::Coordinate& endpointCoord);

  /// get the value for the specified colume from the specified data table, at the specified time. Returns 0 on success, non-zero on failure
  template <class T>
  int getColumnValue_(const std::string& columnName, const simData::DataTable& table, double time, T& value) const;
  /// set the line's LOB draw style at the specified time, using default values if not found in the internal data table
  void setLineDrawStyle_(double time, AnimatedLineNode& line, const simData::LobGroupPrefs& defaultValues);
  /// set the line LOB draw style values from the specified prefs
  void setLineValueFromPrefs_(AnimatedLineNode& line, const simData::LobGroupPrefs& prefs) const;

  /// update the cache so it has lines for every point in 'u'
  void updateCache_(const simData::LobGroupUpdate &u, const simData::LobGroupPrefs& prefs);

  /// updates the label with the given preferences
  void updateLabel_(const simData::LobGroupPrefs& prefs);

private: // data
  /// lobGroup properties
  simData::LobGroupProperties lastProps_;
  /// latest copy of prefs received
  simData::LobGroupPrefs lastPrefs_;
  /// last data update
  simData::LobGroupUpdate lastUpdate_;
  /// is there anything in lastUpdate_
  bool hasLastUpdate_;
  /// does lastPrefs_ validly represent the state of the LobGroup
  bool lastPrefsValid_;
  /// handles clamping coordinates to map surface
  CoordSurfaceClamping* surfaceClamping_;
  /// coordinate converter for use with the surface clamping
  simCore::CoordinateConverter* coordConverter_;
  /// reference to the data store for the LOB
  simData::DataStore &ds_;
  /// Host platform ID
  simData::ObjectId hostId_;

  /// Cache of lines drawn
  Cache *lineCache_;
  /// the transform for this lobgroup that positions the entity label and supports tether
  osg::ref_ptr<osg::MatrixTransform> xform_;
  /// the localgrid node for this lobgroup
  osg::ref_ptr<LocalGridNode> localGrid_;

  /// The actual label for displaying
  osg::ref_ptr<EntityLabelNode> label_;
  /// Cache state to optimize call
  bool lastFlashingState_;

  /// Tag used for picking
  unsigned int objectIndexTag_;
};
}

#endif // SIMVIS_LOB_GROUP_H
