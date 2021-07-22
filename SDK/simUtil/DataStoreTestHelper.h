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
#ifndef SIMUTIL_DATASTORETESTHELPER_H
#define SIMUTIL_DATASTORETESTHELPER_H

#include <set>
#include "simCore/Common/Common.h"
#include "simData/MemoryDataStore.h"

namespace simData { class DataTable; }

namespace simUtil
{

/**
 * Provides convenience methods for adding data to the data store.  Will create a new instance of the
 * data store if none is passed in.  Note that the data store is deleted on destruction ONLY if created in the constructor.
 * If you pass in your own data store to the constructor, you are responsible for deleting it.
 */
class SDKUTIL_EXPORT DataStoreTestHelper
{
public:
  /** Will create a data store if none is passed in; passed-in memory still belongs to caller */
  DataStoreTestHelper(simData::DataStore* dataStore = nullptr);
  /** Deletes data store if data store was created inside constructor */
  virtual ~DataStoreTestHelper();

  /** get a reference to the data store */
  simData::DataStore* dataStore();

  /**
  * Add a platform,
  * Uses the id to construct a name of 'platform<id>'
  * @param originalId Original ID to use for entity
  * @return uint64_t  the id for the entity
  */
  uint64_t addPlatform(uint64_t originalId = 0);

  /**
  * Add a beam
  * Uses the id to construct a name of 'beam<id>_<hostId>'
  * @param hostId platform host
  * @param originalId Original ID to use for entity
  * @param targetBeam Flag to indicate if beam is a target type
  * @return uint64_t  the id for the entity
  */
  uint64_t addBeam(uint64_t hostId, uint64_t originalId = 0, bool targetBeam = false);

  /**
  * Add a gate
  * Uses the id to construct a name of 'gate<id>_<hostId>'
  * @param hostId beam host
  * @param originalId Original ID to use for entity
  * @param targetGate Flag to indicate if gate is a target type
  * @return uint64_t  the id for the entity
  */
  uint64_t addGate(uint64_t hostId, uint64_t originalId = 0, bool targetGate = false);

  /**
  * Add a laser
  * Uses the id to construct a name of 'laser<id>_<hostId>'
  * @param hostId platform host
  * @param originalId Original ID to use for entity
  * @return uint64_t  the id for the entity
  */
  uint64_t addLaser(uint64_t hostId, uint64_t originalId = 0);

  /**
  * Add a LOB
  * Uses the id to construct a name of 'lob<id>_<hostId>'
  * @param hostId platform host
  * @param originalId Original ID to use for entity
  * @return uint64_t  the id for the entity
  */
  uint64_t addLOB(uint64_t hostId, uint64_t originalId = 0);

  /**
  * Add a projector
  * Uses the id to construct a name of 'projector<id>_<hostId>'
  * @param hostId platform host
  * @param originalId Original ID to use for entity
  * @return uint64_t  the id for the entity
  */
  uint64_t addProjector(uint64_t hostId, uint64_t originalId = 0);

  /**
  * Add a custom rendering
  * Uses the id to construct a name of 'customRendering<id>_<hostId>'
  * @param hostId platform host
  * @param originalId Original ID to use for entity
  * @return uint64_t  the id for the entity
  */
  uint64_t addCustomRendering(uint64_t hostId, uint64_t originalId = 0);

  /**
  * Update platform prefs as with object specified, does a merge with prefs passed in
  * @param prefs  new prefs changes
  * @param id entity id
  */
  void updatePlatformPrefs(const simData::PlatformPrefs& prefs, uint64_t id);

  /**
  * Update beam prefs as with object specified, does a merge with prefs passed in
  * @param prefs  new prefs changes
  * @param id entity id
  */
  void updateBeamPrefs(const simData::BeamPrefs& prefs, uint64_t id);

  /**
  * Update gate prefs as with object specified, does a merge with prefs passed in
  * @param prefs  new prefs changes
  * @param id entity id
  */
  void updateGatePrefs(const simData::GatePrefs& prefs, uint64_t id);

  /**
  * Update laser prefs as with object specified, does a merge with prefs passed in
  * @param prefs  new prefs changes
  * @param id entity id
  */
  void updateLaserPrefs(const simData::LaserPrefs& prefs, uint64_t id);

  /**
  * Update LOB prefs as with object specified, does a merge with prefs passed in
  * @param prefs  new prefs changes
  * @param id entity id
  */
  void updateLOBPrefs(const simData::LobGroupPrefs& prefs, uint64_t id);

  /**
  * Update projector prefs as with object specified, does a merge with prefs passed in
  * @param prefs  new prefs changes
  * @param id entity id
  */
  void updateProjectorPrefs(const simData::ProjectorPrefs& prefs, uint64_t id);

  /**
  * Adds a point with position generated based on time.  Note that time should be < 360.0
  * @param time  time of update  should be < 360
  * @param id of entity
  */
  void addPlatformUpdate(double time, uint64_t id);

  /**
  * Adds a point with az, el and range generated based on time.  Note that time should be < 360.0
  * @param time  time of update  should be < 360
  * @param id of entity
  */
  void addBeamUpdate(double time, uint64_t id);

  /**
  * Adds a point with az, el and range generated based on time.  Note that time should be < 360.0
  * @param time  time of update  should be < 360
  * @param id of entity
  */
  void addGateUpdate(double time, uint64_t id);

  /**
  * Adds a point with orientation generated based on time.  Note that time should be < 360.0
  * @param time  time of update  should be < 360
  * @param id of entity
  */
  void addLaserUpdate(double time, uint64_t id);

  /**
  * Adds a point with two detections, where az and el are generated based on time.  Note that time should be < 360.0
  * @param time  time of update  should be < 360
  * @param id of entity
  */
  void addLOBUpdate(double time, uint64_t id);

  /**
  * Adds a point with fov generated based on time.  Note that time should be < 360.0
  * @param time  time of update  should be < 360
  * @param id of entity
  */
  void addProjectorUpdate(double time, uint64_t id);

  /**
  * Adds the passed in command
  * @param command
  * @param id of entity
  */
  void addPlatformCommand(const simData::PlatformCommand& command, uint64_t id);

  /**
  * Adds the passed in command
  * @param command
  * @param id of entity
  */
  void addBeamCommand(const simData::BeamCommand& command, uint64_t id);

  /**
  * Adds the passed in command
  * @param command
  * @param id of entity
  */
  void addGateCommand(const simData::GateCommand& command, uint64_t id);

  /**
  * Adds the passed in command
  * @param command
  * @param id of entity
  */
  void addLaserCommand(const simData::LaserCommand& command, uint64_t id);

  /**
  * Adds the passed in command
  * @param command
  * @param id of entity
  */
  void addLOBCommand(const simData::LobGroupCommand& command, uint64_t id);

  /**
  * Adds the passed in command
  * @param command
  * @param id of entity
  */
  void addProjectorCommand(const simData::ProjectorCommand& command, uint64_t id);

  /**
  * Adds the passed in command
  * @param command
  * @param id of entity
  */
  void addCustomRenderingCommand(const simData::CustomRenderingCommand& command, uint64_t id);
    ;
  /**
  * Adds the passed in Category Data
  * @param id of entity
  * @param key of category data
  * @param value of category data
  * @param startTime of category data
  */
  void addCategoryData(uint64_t id, const std::string& key, const std::string& value, double startTime);

  /**
  * Adds the passed in command with -1 expiration
  * @param id of entity
  * @param key of generic data
  * @param value of generic data
  * @param startTime of generic data
  * @param ignoreDuplicate If true, data is rejected as duplicate if the key's value prior to startTime is the same as 'value'.  Performance / storage optimization.
  */
  void addGenericData(uint64_t id, const std::string& key, const std::string& value, double startTime, bool ignoreDuplicate=false);

  /**
  * Add a DataTable to the specified entity, will auto generate a new name if none is passed in.
  * @param entityId ID of the table owner
  * @param numRows Number of rows to generate
  * @param tableName Name of the table to create
  * @return The table of ID of the newly created table
  */
  uint64_t addDataTable(uint64_t entityId, int numRows = 2, const std::string& tableName = "");

private:
  /** add a DataTable column with the specified number of rows.  Will set null or null-less based on id value */
  void addDataTableRows_(simData::DataTable* table, int numRows, uint64_t id);

  simData::DataStore* dataStore_; ///< reference to the data store
  bool ownDataStore_; ///< flag to determine if the reference to the data store should be deleted
  std::set<uint64_t> entityIds_; ///< holds the entityIds to ensure uniqueness of generated ids
  uint64_t tableId_; ///< Used to give tables unique names

  /// Private copy constructor to prevent static analysis hits; not implemented
  DataStoreTestHelper(const DataStoreTestHelper& noCopyConstr);
  /// Private copy operator to prevent static analysis hits; not implemented
  DataStoreTestHelper& operator=(const DataStoreTestHelper& noCopyOperator);
};

}

#endif /* SIMUTIL_DATASTORETESTHELPER_H */

