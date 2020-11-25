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
#include <fstream>

#include "simCore/Common/Version.h"
#include "simCore/String/UtfUtils.h"
#include "simData/MemoryDataStore.h"
#include "simData/LinearInterpolator.h"
#include "simData/DataTable.h"
#include "simData/CategoryData/CategoryFilter.h"
#include "simCore/Time/Utils.h"
#include "simCore/Calc/Math.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simUtil/DataStoreTestHelper.h"


enum TableSparsity
{
  SPARSITY_NONE,
  SPARSITY_SMALL,
  SPARSITY_MAXIMUM
};

/// Counts the number of callbacks
struct CallbackCounters
{
  CallbackCounters()
  : add(0),
    remove(0),
    pref(0),
    time(0),
    category(0),
    name(0)
  {
  }

  size_t add;
  size_t remove;
  size_t pref;
  size_t time;
  size_t category;
  size_t name;
};

/// The listener for counting the callbacks
class CountListener : public simData::DataStore::DefaultListener
{
public:
  explicit CountListener(CallbackCounters* counters)
    : counters_(counters)
  {
  }

  /// new entity has been added, with the given id and type
  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    counters_->add++;
  }

  /// entity with the given id and type will be removed after all notifications are processed
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    counters_->remove++;
  }

  /// prefs for the given entity have been changed
  virtual void onPrefsChange(simData::DataStore *source, simData::ObjectId id)
  {
    counters_->pref++;
  }

  /// data store has changed
  virtual void onChange(simData::DataStore *source)
  {
    counters_->time++;
  }

  /// something has changed in the entity category data
  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::ObjectType ot)
  {
    counters_->category++;
  }

  /// entity name has changed
  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId)
  {
    counters_->name++;
  }

  /// The actual counts
  CallbackCounters* counters_;
};

/// The base class handles the algorithms for adding to the DataStore
/// The derived classes handle the different calls needed for each entity type
class Entity
{
public:
  explicit Entity(simUtil::DataStoreTestHelper& helper) :
    helper_(helper),
    initialId_(0),
    number_(0),
    dataPerSecond_(0),
    dataLimitPoints_(600),
    dataLimitSeconds_(0.0),
    initialCategoryData_(0),
    categoryPerDataPoint_(0),
    initialGenericData_(0),
    genericPerDataPoint_(0),
    includeInitialColor_(false),
    includeColorPerData_(false),
    downSample_(1),
    numTables_(0),
    tableNumColumns_(0),
    tableSparsity_(SPARSITY_NONE),
    tableVariableType_(simData::VT_DOUBLE)
  {
  }

  virtual ~Entity() {}

  /// Adds all, if any, entities for a particular type.
  virtual void addEntities(uint64_t initialId)
  {
    initialId_ = initialId;
    for (size_t ii = 0; ii < number_; ii++)
    {
      uint64_t id = initialId_+ii;
      addEntity(id);
      createTables_(id, numTables_, tableNumColumns_);

      if (dataLimitPoints_ > 0 || dataLimitSeconds_ > 0.0)
        addDataLimit(id);

      if (includeInitialColor_)
        addColor(id, -1.0);

      addCategory(helper_, id, initialCategoryData_, -1.0);
      addGeneric(helper_, id, initialGenericData_, 0.0);
    }
  }

  /// Do the type specific add
  virtual void addEntity(uint64_t id) = 0;

  /// Do the type specific data limit
  virtual void addDataLimit(uint64_t id) = 0;

  /// Do the type specific color command
  virtual void addColor(uint64_t id, double time) = 0;

  virtual void addUpdates(size_t seconds, size_t factionSeconds, size_t maxRate)
  {
    if ((factionSeconds % downSample_) == 0)
    {
      double time = static_cast<double>(seconds) + static_cast<double>(factionSeconds)/static_cast<double>(maxRate);
      for (size_t kk = 0; kk < number_; kk++)
      {
        uint64_t id = initialId_+kk;
        addUpdate(id, time);

        if (includeColorPerData_)
          addColor(id, time);

        addCategory(helper_, id, categoryPerDataPoint_, time);
        addGeneric(helper_, id, genericPerDataPoint_, time);
        addTableData(helper_, id, time);
      }
    }
  }

  /// Do the type specific update
  virtual void addUpdate(uint64_t id, double time) = 0;

  /// Add category data
  void addCategory(simUtil::DataStoreTestHelper& helper, uint64_t id, size_t number, double time)
  {
    for (size_t ii = 0; ii < number; ii++)
    {
      std::ostringstream key;
      key << "Key_" << id << "_" << ii;
      helper.addCategoryData(id, key.str(), "12345678901234567890", time);
    }
  }

  /// Add generic data
  void addGeneric(simUtil::DataStoreTestHelper& helper, uint64_t id, size_t number, double time)
  {
    for (size_t ii = 0; ii < number; ii++)
    {
      std::ostringstream key;
      key << "Key_" << id << "_" << ii;
      helper.addGenericData(id, key.str(), "12345678901234567890", time);
    }
  }

  void addTableData(simUtil::DataStoreTestHelper& helper, uint64_t id, double time)
  {
    // Controls the number of columns placed into the row being added
    size_t rowModulus = 1;
    if (tableSparsity() == SPARSITY_SMALL)
      rowModulus = 2;
    else if (tableSparsity() == SPARSITY_MAXIMUM)
      rowModulus = std::numeric_limits<size_t>::max();
    rowModulus = simCore::sdkMin(rowModulus, tableNumColumns());

    // Visitor for table list that adds a row at the given time to every table
    class AddRowPerTable : public simData::TableList::Visitor
    {
    public:
      AddRowPerTable(double time, size_t rowModulus)
        : time_(time),
          rowModulus_(rowModulus)
      {}
      virtual void visit(simData::DataTable* table)
      {
        addRow_(table);
      }
    private:
      void addRow_(simData::DataTable* table) const
      {
        static const double TIME_EPSILON = 1e-6;
        const size_t numColumns = table->columnCount();
        double realTime = time_;
        for (size_t currentModulus = 0; currentModulus < rowModulus_; ++currentModulus)
        {
          simData::TableRow row;
          row.setTime(realTime);
          for (size_t col = 0; col < numColumns; ++col)
          {
            // Only set values in row if modulus matches
            if (col % rowModulus_ == currentModulus)
            {
              row.setValue(col, col);
            }
          }
          if (!row.empty())
            table->addRow(row);

          // Add a small epsilon to introduce nullptrs
          realTime += TIME_EPSILON;
        }
      }

      double time_;
      size_t rowModulus_;
    };
    AddRowPerTable addRowPerTable(time, rowModulus);
    const simData::TableList* tables = helper_.dataStore()->dataTableManager().tablesForOwner(id);
    if (tables != nullptr)
      tables->accept(addRowPerTable);
  }

  uint64_t initialId() const { return initialId_; }
  void setInitialId(uint64_t value) { initialId_ = value; }
  size_t number() const { return number_; }
  void setNumber(size_t value) { number_ = value; }
  size_t dataPerSecond() const { return dataPerSecond_; }
  void setDataPerSecond(size_t value) { dataPerSecond_ = value; }
  size_t dataLimitPoints() const { return dataLimitPoints_; }
  void setDataLimitPoints(size_t value) { dataLimitPoints_ = value; }
  double dataLimitSeconds() const { return dataLimitSeconds_; }
  void setDataLimitSeconds(double value) { dataLimitSeconds_ = value; }
  size_t initialCategoryData() const { return initialCategoryData_; }
  void setInitialCategoryData(size_t value) { initialCategoryData_ = value; }
  size_t categoryPerDataPoint() const { return categoryPerDataPoint_; }
  void setCategoryPerDataPoint(size_t value) { categoryPerDataPoint_ = value; }
  size_t initialGenericData() const { return initialGenericData_; }
  void setInitialGenericData(size_t value) { initialGenericData_ = value; }
  size_t genericPerDataPoint() const { return genericPerDataPoint_; }
  void setGenericPerDataPoint(size_t value) { genericPerDataPoint_ = value; }
  bool includeInitialColor() const { return includeInitialColor_; }
  void setIncludeInitialColor(bool value) { includeInitialColor_ = value; }
  bool includeColorPerData() const { return includeColorPerData_; }
  void setIncludeColorPerData(bool value) { includeColorPerData_ = value; }
  size_t downSample() const { return downSample_; }
  void setDownSample(size_t value) { downSample_ = value; }
  size_t numTables() const { return numTables_; }
  void setNumTables(size_t value) { numTables_ = value; }
  size_t tableNumColumns() const { return tableNumColumns_; }
  void setTableNumColumns(size_t value) { tableNumColumns_ = value; }
  TableSparsity tableSparsity() const { return tableSparsity_; }
  void setTableSparsity(TableSparsity value) { tableSparsity_ = value; }
  simData::VariableType tableVariableType() const { return tableVariableType_; }
  void setTableVariableType(simData::VariableType value) { tableVariableType_ = value; }

  uint64_t lastId() const
  {
    return initialId_ + number_;
  }

protected:
  simUtil::DataStoreTestHelper& helper_;
  uint64_t initialId_;  // The Id of the first entity, counts by one afterwards
  size_t number_;  // Number of entities, can be zero
  size_t dataPerSecond_;  // Number of data points per second
  size_t dataLimitPoints_;  // Data limit value in maximum number of points
  double dataLimitSeconds_; // Data limit value in seconds
  size_t initialCategoryData_;  // Number of initial category data entries, can be zero
  size_t categoryPerDataPoint_;  // Number of category data entries, can be zero
  size_t initialGenericData_;  // Number of initial generic data entries, can be zero
  size_t genericPerDataPoint_;  // Number of generic data entries, can be zero
  bool includeInitialColor_;  // Include an initial color
  bool includeColorPerData_;  // Include a color for each data point
  size_t downSample_;  // Live mode calculates the max rate to handle all data rates, so need to down sample to get the correct rate for this entity
  size_t numTables_; // Number of data tables to create
  size_t tableNumColumns_; // Number of columns per data table
  TableSparsity tableSparsity_; // Number of NULLs to include in the data table rows
  simData::VariableType tableVariableType_; // Table variable type for underlying storage

  void setLimits_(simData::CommonPrefs* prefs) const
  {
    if (dataLimitPoints_ > 0)
      prefs->set_datalimitpoints(static_cast<uint32_t>(dataLimitPoints_));
    if (dataLimitSeconds_ > 0)
      prefs->set_datalimittime(dataLimitSeconds_);
  }

  std::string asString_(size_t num) const
  {
    std::stringstream ss;
    ss << num;
    return ss.str();
  }

  void createTables_(simData::ObjectId id, size_t numTables, size_t numColPerTable)
  {
    simData::DataTableManager& mgr = helper_.dataStore()->dataTableManager();
    for (size_t table = 0; table < numTables; ++table)
    {
      simData::DataTable* newTable = nullptr;
      mgr.addDataTable(id, "Table " + asString_(table), &newTable);
      for (size_t col = 0; col < numColPerTable; ++col)
      {
        newTable->addColumn("Column " + asString_(col), tableVariableType_, 0, nullptr);
      }
    }
  }
};

/// Handles special processing for platforms
class Platforms : public Entity
{
public:
  explicit Platforms(simUtil::DataStoreTestHelper& helper) : Entity(helper) {};
  virtual ~Platforms() {}
  virtual void addEntity(uint64_t id)
  {
    helper_.addPlatform();
  }

  virtual void addDataLimit(uint64_t id)
  {
    simData::PlatformPrefs prefs;
    setLimits_(prefs.mutable_commonprefs());
    helper_.updatePlatformPrefs(prefs, id);
  }

  virtual void addColor(uint64_t id, double time)
  {
    simData::PlatformCommand cmd;
    cmd.set_time(time);
    cmd.mutable_updateprefs()->mutable_commonprefs()->set_color(0xFF00FF00);
    helper_.addPlatformCommand(cmd, id);
  }

  virtual void addUpdate(uint64_t id, double time)
  {
    helper_.addPlatformUpdate(time, id);
  };
};

/// Handles special processing for beams
class Beams : public Entity
{
public:
  explicit Beams(simUtil::DataStoreTestHelper& helper) : Entity(helper) {};
  virtual ~Beams() {}

  virtual void addEntity(uint64_t id)
  {
    helper_.addBeam(id);
  }

  virtual void addDataLimit(uint64_t id)
  {
    simData::BeamPrefs prefs;
    setLimits_(prefs.mutable_commonprefs());
    helper_.updateBeamPrefs(prefs, id);
  }

  virtual void addColor(uint64_t id, double time)
  {
    simData::BeamCommand cmd;
    cmd.set_time(time);
    cmd.mutable_updateprefs()->mutable_commonprefs()->set_color(0xFF00FF00);
    helper_.addBeamCommand(cmd, id);
  }

  virtual void addUpdate(uint64_t id, double time)
  {
    helper_.addBeamUpdate(time, id);
  };
};

/// Handles special processing for gates
class Gates : public Entity
{
public:
  explicit Gates(simUtil::DataStoreTestHelper& helper) : Entity(helper) {};
  virtual ~Gates() {}

  virtual void addEntity(uint64_t id)
  {
    helper_.addGate(id);
  }

  virtual void addDataLimit(uint64_t id)
  {
    simData::GatePrefs prefs;
    setLimits_(prefs.mutable_commonprefs());
    helper_.updateGatePrefs(prefs, id);
  }

  virtual void addColor(uint64_t id, double time)
  {
    simData::GateCommand cmd;
    cmd.set_time(time);
    cmd.mutable_updateprefs()->mutable_commonprefs()->set_color(0xFF00FF00);
    helper_.addGateCommand(cmd, id);
  }

  virtual void addUpdate(uint64_t id, double time)
  {
    helper_.addGateUpdate(time, id);
  };
};

/// Handles special processing for lasers
class Lasers : public Entity
{
public:
  explicit Lasers(simUtil::DataStoreTestHelper& helper) : Entity(helper) {};
  virtual ~Lasers() {}

  virtual void addEntity(uint64_t id)
  {
    helper_.addLaser(id);
  }

  virtual void addDataLimit(uint64_t id)
  {
    simData::LaserPrefs prefs;
    setLimits_(prefs.mutable_commonprefs());
    helper_.updateLaserPrefs(prefs, id);
  }

  virtual void addColor(uint64_t id, double time)
  {
    simData::LaserCommand cmd;
    cmd.set_time(time);
    cmd.mutable_updateprefs()->mutable_commonprefs()->set_color(0xFF00FF00);
    helper_.addLaserCommand(cmd, id);
  }

  virtual void addUpdate(uint64_t id, double time)
  {
    helper_.addLaserUpdate(time, id);
  };
};

/// Handles special processing for LOB Groups
class LobGroups : public Entity
{
public:
  explicit LobGroups(simUtil::DataStoreTestHelper& helper) : Entity(helper) {};
  virtual ~LobGroups() {}

  virtual void addEntity(uint64_t id)
  {
    helper_.addLOB(id);
  }

  virtual void addDataLimit(uint64_t id)
  {
    simData::LobGroupPrefs prefs;
    setLimits_(prefs.mutable_commonprefs());
    helper_.updateLOBPrefs(prefs, id);
  }

  virtual void addColor(uint64_t id, double time)
  {
    simData::LobGroupCommand cmd;
    cmd.set_time(time);
    cmd.mutable_updateprefs()->mutable_commonprefs()->set_color(0xFF00FF00);
    helper_.addLOBCommand(cmd, id);
  }

  virtual void addUpdate(uint64_t id, double time)
  {
    helper_.addLOBUpdate(time, id);
  };
};

/// Helper for holding all the different entity types
struct Entities
{
  explicit Entities(simUtil::DataStoreTestHelper& helper)
   : platforms(new Platforms(helper)),
     beams(new Beams(helper)),
     gates(new Gates(helper)),
     lasers(new Lasers(helper)),
     lobGroups(new LobGroups(helper))
  {
  }
  ~Entities()
  {
    delete lobGroups;
    delete lasers;
    delete gates;
    delete beams;
    delete platforms;
  }

  Platforms* platforms;
  Beams* beams;
  Gates* gates;
  Lasers* lasers;
  LobGroups* lobGroups;

private:
  /** Not implemented */
  Entities(const Entities& noCopyConstructor);
};

/// Helper for the top level options
struct TopLevelOptions
{
  TopLevelOptions() :
    numberOfSeconds(0),
    frameRate(0),
    fileMode(true),
    interpolate(true),
    dataLimiting(false),
    playforward(true),
    addListener(true),
    testCD(false)
  {
  }

  int numberOfSeconds;  // The number of seconds for data
  int frameRate;   // The simulated frame rate
  bool fileMode;  // True = file mode, False = live mode
  bool interpolate; // True = interpolate data
  bool dataLimiting;  // True = data limiting
  bool playforward;  // True = move time forwards, False = move time backwards
  bool addListener;  // True = count the number of callbacks
  bool testCD;       // True = testing will include testing of CategoryData
};

/// Initializes the DataStore and creates all the entities
simData::LinearInterpolator* initializeDataStore(simData::DataStore& ds, simUtil::DataStoreTestHelper& helper, const TopLevelOptions& options, Entities& entities, CallbackCounters* counters)
{
  simData::LinearInterpolator* interpolator = new simData::LinearInterpolator();
  ds.setInterpolator(interpolator);
  ds.enableInterpolation(options.interpolate);

  if (options.addListener)
    ds.addListener(simData::DataStore::ListenerPtr(new CountListener(counters)));

  ds.setDataLimiting(options.dataLimiting);

  entities.platforms->addEntities(1);
  entities.beams->addEntities(entities.platforms->lastId());
  entities.gates->addEntities(entities.beams->lastId());
  entities.lasers->addEntities(entities.gates->lastId());
  entities.lobGroups->addEntities(entities.lasers->lastId());

  return interpolator;
}

/// Removes all the entities and checks the callback counters
int cleanUpDataStore(simData::DataStore& ds, const TopLevelOptions& options, Entities& entities, CallbackCounters& counters)
{
  size_t size = entities.platforms->number() +
    entities.beams->number() +
    entities.gates->number() +
    entities.lasers->number() +
    entities.lobGroups->number();

  for (uint64_t ii = 1; ii <= size; ii++)
    ds.removeEntity(ii);

  // Need to call update to force the callbacks
  ds.update(0.0);

  // Minor sanity checks
  int rv = 0;
  if (options.addListener)
  {
    rv += SDK_ASSERT(counters.add == size);
    rv += SDK_ASSERT(counters.remove == size);
    rv += SDK_ASSERT(counters.name == size);
    if (options.interpolate)
    {
      // time counter will be <= calculated count due to ds not updating when nothing changes
      rv += SDK_ASSERT(counters.time <= static_cast<size_t>(options.numberOfSeconds*options.frameRate + 1));  // The plus 1 because of the update to force the remove callback
    }
    else
      rv += SDK_ASSERT(counters.time == static_cast<size_t>(options.numberOfSeconds*options.frameRate + 1));  // The plus 1 because of the update to force the remove callback
  }

  return rv;
}

/// Simulates file mode by loading the data than doing one playback
double fileMode(simData::DataStore& ds, simUtil::DataStoreTestHelper& helper, TopLevelOptions& options, Entities& entities)
{
  std::cout << "In File Mode" << std::endl;
  std::cout << "Creating Data" << std::endl;

  for (size_t ii = 0; ii < static_cast<size_t>(options.numberOfSeconds); ii++)
  {
    for (size_t jj = 0; jj < entities.platforms->dataPerSecond(); jj++)
      entities.platforms->addUpdates(ii, jj, entities.platforms->dataPerSecond());

    for (size_t jj = 0; jj < entities.beams->dataPerSecond(); jj++)
      entities.beams->addUpdates(ii, jj, entities.beams->dataPerSecond());

    for (size_t jj = 0; jj < entities.gates->dataPerSecond(); jj++)
      entities.gates->addUpdates(ii, jj, entities.gates->dataPerSecond());

    for (size_t jj = 0; jj < entities.lasers->dataPerSecond(); jj++)
      entities.lasers->addUpdates(ii, jj, entities.lasers->dataPerSecond());

    for (size_t jj = 0; jj < entities.lobGroups->dataPerSecond(); jj++)
      entities.lobGroups->addUpdates(ii, jj, entities.lobGroups->dataPerSecond());
  }

  std::cout << "Starting updates" << std::endl;
  // The sleep helps with looking at the data in the Intel tools
  Sleep(1000);

  double direction = 1.0;
  int offset = 0;
  if (!options.playforward)
  {
    // Change the values to cause a reverse playback
    direction *= -1.0;
    offset = -options.numberOfSeconds*options.frameRate;
  }

  const double startTime = simCore::systemTimeToSecsBgnYr();
  for (int ii = 0; ii < options.numberOfSeconds*options.frameRate; ii++)
  {
    // Add the 0.0001 so we never get an exact hit
    const double time = 0.0001 + direction*static_cast<double>(ii+offset)/static_cast<double>(options.frameRate);
    ds.update(time);
    if (options.testCD && entities.platforms->initialId() > 0)
    {
      simData::CategoryFilter::CurrentCategoryValues curVals;
      simData::CategoryFilter::getCurrentCategoryValues(ds, entities.platforms->initialId(), curVals);
      simData::CategoryFilter::CurrentCategoryValues curVals2;
      simData::CategoryFilter::getCurrentCategoryValues(ds, entities.platforms->lastId(), curVals2);
    }
  }

  const double endTime = simCore::systemTimeToSecsBgnYr();
  return endTime-startTime;
}

/// Simulates live mode by repeatedly adding data and doing an update
double liveMode(simData::DataStore& ds, simUtil::DataStoreTestHelper& helper, TopLevelOptions& options, Entities& entities)
{
  std::cout << "In Live Mode" << std::endl;

  // In Live mode the data needs to be interleaved
  // So calculate the max rate, then down sample the individual entity types
  size_t maxRate = options.frameRate;
  if ((maxRate % entities.platforms->dataPerSecond()) != 0)
    maxRate *= entities.platforms->dataPerSecond();

  if ((entities.beams->dataPerSecond() > 0) && ((maxRate % entities.beams->dataPerSecond()) != 0))
    maxRate *= entities.beams->dataPerSecond();

  if ((entities.gates->dataPerSecond() > 0) && ((maxRate % entities.gates->dataPerSecond()) != 0))
    maxRate *= entities.gates->dataPerSecond();

  if ((entities.lasers->dataPerSecond() > 0) && ((maxRate % entities.lasers->dataPerSecond()) != 0))
    maxRate *= entities.lasers->dataPerSecond();

  if ((entities.lobGroups->dataPerSecond() > 0) && ((maxRate % entities.lobGroups->dataPerSecond()) != 0))
    maxRate *= entities.lobGroups->dataPerSecond();

  size_t frameRateDownSample = maxRate / options.frameRate;
  entities.platforms->setDownSample(maxRate / entities.platforms->dataPerSecond());
  entities.beams->setDownSample(maxRate / (entities.beams->dataPerSecond() > 0 ? entities.beams->dataPerSecond()  : 1));
  entities.gates->setDownSample(maxRate / (entities.gates->dataPerSecond() > 0 ? entities.gates->dataPerSecond()  : 1));
  entities.lasers->setDownSample(maxRate / (entities.lasers->dataPerSecond() > 0 ? entities.lasers->dataPerSecond()  : 1));
  entities.lobGroups->setDownSample(maxRate / (entities.lobGroups->dataPerSecond() > 0 ? entities.lobGroups->dataPerSecond()  : 1));

  // The sleep helps with looking at the data in the Intel tools
  Sleep(1000);

  double startTime = simCore::systemTimeToSecsBgnYr();
  for (size_t ii = 0; ii < static_cast<size_t>(options.numberOfSeconds); ii++)
  {
    for (size_t jj = 0; jj < maxRate; jj++)
    {
      // Add data when necessary
      entities.platforms->addUpdates(ii, jj, maxRate);
      entities.beams->addUpdates(ii, jj, maxRate);
      entities.gates->addUpdates(ii, jj, maxRate);
      entities.lasers->addUpdates(ii, jj, maxRate);
      entities.lobGroups->addUpdates(ii, jj, maxRate);

      // Update data when necessary
      if ((jj % frameRateDownSample) == 0)
      {
        double time;

        if (options.interpolate)
        {
          // Need to interpolate behind the current time, cannot extrapolate, so pause for 1 second
          if (ii == 0)
          {
            // repeated ds updates for same time are ignored, if no changes in entities
            // in this case, counter.time will not be incremented since no actual update occurs.
            time = 0.0;
          }
          else
            time = 0.0001 + static_cast<double>(ii-1) + static_cast<double>(jj)/static_cast<double>(maxRate);
        }
        else
          time = 0.0001 + static_cast<double>(ii) + static_cast<double>(jj)/static_cast<double>(maxRate);
        ds.update(time);

        if (options.testCD && entities.platforms->initialId() > 0 && time > 0.0)
        {
          simData::CategoryFilter::CurrentCategoryValues curVals;
          simData::CategoryFilter::getCurrentCategoryValues(ds, entities.platforms->initialId(), curVals);
          simData::CategoryFilter::CurrentCategoryValues curVals2;
          simData::CategoryFilter::getCurrentCategoryValues(ds, entities.platforms->lastId(), curVals2);
        }
      }
    }
  }

  double endTime = simCore::systemTimeToSecsBgnYr();
  return endTime-startTime;
}

void writeEntityConfigurationPart(std::ofstream& output, const std::string& entity, int number)
{
  output << entity << " Number " << number << " # Number of entities, can be zero for all entity types except platforms" << std::endl;
  output << entity << " DataPerSecond 1         # Integer number of data points per second (TSPI, RAE), must be 1 or greater" << std::endl;
  output << entity << " DataLimitPoints 600     # The data limit #points, if live mode and data limiting is turned on" << std::endl;
  output << entity << " DataLimitSeconds 600    # The data limit #seconds, if live mode and data limiting is turned on" << std::endl;
  output << entity << " InitialCategoryData 0   # The amount of category data to create when the entity is created, can be zero" << std::endl;
  output << entity << " CategoryPerDataPoint 0  # The amount of category data to create with each data point, can be zero" << std::endl;
  output << entity << " InitialGenericData 0    # The amount of generic data to create when the entity is created, can be zero" << std::endl;
  output << entity << " GenericPerDataPoint 0   # The amount of generic data to create with each data point, can be zero" << std::endl;
  output << entity << " IncludeInitialColor False  # True means add an initial color" << std::endl;
  output << entity << " IncludeColorPerData False  # True means add color to each data point" << std::endl;
  // TODO: Add table-specific values here
  output << std::endl;
}

void writeExampleConfigurationFile()
{
  std::ofstream output("DataStorePerformanceTest.conf");

  if (!output.good())
  {
    std::cerr << "Failed to open DataStorePerformanceTest.conf for writing" << std::endl;
    return;
  }

  output << "# Configuration file for the DataStore Performance Application" << std::endl;
  output << "Mode File                 # Mode options are File or Live" << std::endl;
  output << "FrameRate 20              # Simulated display rate in frames per seconds" << std::endl;
  output << "Interpolate true          # State of the DataStore interpolation" << std::endl;
  output << "NumberOfSeconds 150       # Seconds of data" << std::endl;
  output << "DataLimiting false        # Used in Live mode to limit the amount of data, limits are set below" << std::endl;
  output << std::endl;

  writeEntityConfigurationPart(output, "Platform", 1000);
  writeEntityConfigurationPart(output, "Beam", 0);
  writeEntityConfigurationPart(output, "Gate", 0);
  writeEntityConfigurationPart(output, "Laser", 0);
  writeEntityConfigurationPart(output, "LobGroup", 0);

  output.close();
}

void usage()
{
    std::cerr << "DataStorePerformanceTest InputConfigfile | --help | --testCD | --WriteExampleConfigFile" << std::endl;
    std::cerr << "  InputConfigFile specifies the parameters for the performance test" << std::endl;
    std::cerr << "  --testCD include testing of CategoryData" << std::endl;
    std::cerr << "  --WriteExampleConfigFile writes out an example configuration file to DataStorePerformanceTest.conf" << std::endl;
    std::cerr << "  --help display this text" << std::endl;
}
/// Look for the configuration file name on the command line
int parseCommandLine(int argc, char** argv, std::string& fileName, TopLevelOptions& options)
{
  if (argc < 2 || argc > 3)
  {
    usage();
    return -1;
  }

  fileName = "";

  std::string inputValue = std::string(argv[1]);
  if (inputValue == "--help")
  {
    usage();
    return 0;
  }
  if (inputValue == "--WriteExampleConfigFile")
  {
    writeExampleConfigurationFile();
    return 0;
  }

  for (int i = 1; i < argc; i++)
  {
    std::string inputValue = std::string(argv[i]);
    if (inputValue == "--testCD")
      options.testCD = true;
    else
      fileName = inputValue;
  }

  if (fileName.empty())
  {
    usage();
    return -1;
  }

  return 0;
}

/// Processes an entity command from the configuration file
int loadEntity(std::vector<std::string>& tokens, size_t lineNumber, Entity* entity)
{
  if (tokens.size() != 3)
    return -1;

  if (simCore::caseCompare(tokens[1], "Number") == 0)
  {
    entity->setNumber(static_cast<size_t>(atoi(tokens[2].c_str())));
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "DataPerSecond") == 0)
  {
    entity->setDataPerSecond(static_cast<size_t>(atoi(tokens[2].c_str())));
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "DataLimitPoints") == 0)
  {
    entity->setDataLimitPoints(static_cast<size_t>(atoi(tokens[2].c_str())));
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "DataLimitSeconds") == 0)
  {
    entity->setDataLimitSeconds(static_cast<double>(atof(tokens[2].c_str())));
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "InitialCategoryData") == 0)
  {
    entity->setInitialCategoryData(static_cast<size_t>(atoi(tokens[2].c_str())));
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "CategoryPerDataPoint") == 0)
  {
    entity->setCategoryPerDataPoint(static_cast<size_t>(atoi(tokens[2].c_str())));
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "InitialGenericData") == 0)
  {
    entity->setInitialGenericData(static_cast<size_t>(atoi(tokens[2].c_str())));
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "GenericPerDataPoint") == 0)
  {
    entity->setGenericPerDataPoint(static_cast<size_t>(atoi(tokens[2].c_str())));
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "IncludeInitialColor") == 0)
  {
    entity->setIncludeInitialColor(simCore::caseCompare(tokens[2], "True") == 0);
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "IncludeColorPerData") == 0)
  {
    entity->setIncludeColorPerData(simCore::caseCompare(tokens[2], "True") == 0);
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "NumTables") == 0)
  {
    entity->setNumTables(static_cast<size_t>(atoi(tokens[2].c_str())));
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "TableNumColumns") == 0)
  {
    entity->setTableNumColumns(static_cast<size_t>(atoi(tokens[2].c_str())));
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "TableSparsity") == 0)
  {
    if (simCore::caseCompare(tokens[2], "none") == 0)
      entity->setTableSparsity(SPARSITY_NONE);
    else if (simCore::caseCompare(tokens[2], "small") == 0)
      entity->setTableSparsity(SPARSITY_SMALL);
    else if (simCore::caseCompare(tokens[2], "maximum") == 0)
      entity->setTableSparsity(SPARSITY_MAXIMUM);
    else
    {
      std::cout << "Unknown sparsity type " << tokens[2] << " on line " << lineNumber << std::endl;
      return -1;
    }
    return 0;
  }

  if (simCore::caseCompare(tokens[1], "TableVariableType") == 0)
  {
    if (simCore::caseCompare(tokens[2], "double") == 0)
      entity->setTableVariableType(simData::VT_DOUBLE);
    else if (simCore::caseCompare(tokens[2], "float") == 0)
      entity->setTableVariableType(simData::VT_FLOAT);
    else if (simCore::caseCompare(tokens[2], "string") == 0)
      entity->setTableVariableType(simData::VT_STRING);
    else if (simCore::caseCompare(tokens[2], "uint8_t") == 0)
      entity->setTableVariableType(simData::VT_UINT8);
    else if (simCore::caseCompare(tokens[2], "int8_t") == 0)
      entity->setTableVariableType(simData::VT_INT8);
    else if (simCore::caseCompare(tokens[2], "uint16_t") == 0)
      entity->setTableVariableType(simData::VT_UINT16);
    else if (simCore::caseCompare(tokens[2], "int16_t") == 0)
      entity->setTableVariableType(simData::VT_INT16);
    else if (simCore::caseCompare(tokens[2], "uint32_t") == 0)
      entity->setTableVariableType(simData::VT_UINT32);
    else if (simCore::caseCompare(tokens[2], "int32_t") == 0)
      entity->setTableVariableType(simData::VT_INT32);
    else if (simCore::caseCompare(tokens[2], "uint64_t") == 0)
      entity->setTableVariableType(simData::VT_UINT64);
    else if (simCore::caseCompare(tokens[2], "int64_t") == 0)
      entity->setTableVariableType(simData::VT_INT64);
    else
    {
      std::cout << "Unknown variable type " << tokens[2] << " on line " << lineNumber << std::endl;
      return -1;
    }
    return 0;
  }

  std::cout << "Unknown command on line " << lineNumber << std::endl;
  return -1;
}

/// Loads a configuration file
int loadConfigurationFile(const std::string& fileName, TopLevelOptions& options, Entities& entities)
{
  if (fileName.empty())
    return -1;

  // open the config file
  std::ifstream is(simCore::streamFixUtf8(fileName));
  if (!is.is_open())
  {
    std::cerr << "Could not open configuration file: " << fileName << std::endl;
    return -1;
  }

  std::string st;
  std::vector<std::string> tokens;

  size_t currentLineNumber = 0;

  // steps through each line of the file
  while (getline(is, st))
  {
    tokens.clear();
    simCore::quoteCommentTokenizer(st, tokens);

    ++currentLineNumber;

    // Blank lines are OK
    if (tokens.empty())
      continue;

    if (tokens.size() == 2)
    {
      // Top Level Option
      if (simCore::caseCompare(tokens[0], "Mode") == 0)
        options.fileMode = (simCore::caseCompare(tokens[1], "File") == 0);
      else if (simCore::caseCompare(tokens[0], "Interpolate") == 0)
        options.interpolate = (simCore::caseCompare(tokens[1], "True") == 0);
      else if (simCore::caseCompare(tokens[0], "FrameRate") == 0)
        options.frameRate = atoi(tokens[1].c_str());
      else if (simCore::caseCompare(tokens[0], "NumberOfSeconds") == 0)
        options.numberOfSeconds = atoi(tokens[1].c_str());
      else if (simCore::caseCompare(tokens[0], "DataLimiting") == 0)
        options.dataLimiting = (simCore::caseCompare(tokens[1], "True") == 0);
      else
      {
        std::cerr << "Unknown command on line " << currentLineNumber << std::endl;
        return -1;
      }
    }
    else if (tokens.size() == 3)
    {
      // Entity command
      if (simCore::caseCompare(tokens[0], "Platform") == 0)
      {
        if (loadEntity(tokens, currentLineNumber, entities.platforms) != 0)
          return -1;
      }
      else if (simCore::caseCompare(tokens[0], "Beam") == 0)
      {
        if (loadEntity(tokens, currentLineNumber, entities.beams) != 0)
          return -1;
      }
      else if (simCore::caseCompare(tokens[0], "Gate") == 0)
      {
        if (loadEntity(tokens, currentLineNumber, entities.gates) != 0)
          return -1;
      }
      else if (simCore::caseCompare(tokens[0], "Laser") == 0)
      {
        if (loadEntity(tokens, currentLineNumber, entities.lasers) != 0)
          return -1;
      }
      else if (simCore::caseCompare(tokens[0], "LobGroup") == 0)
      {
        if (loadEntity(tokens, currentLineNumber, entities.lobGroups) != 0)
          return -1;
      }
      else
      {
        std::cerr << "Unknown command on line " << currentLineNumber << std::endl;
        return -1;
      }
    }
    else
    {
      std::cerr << "Unknown command on line " << currentLineNumber << std::endl;
      return -1;
    }
  }

  return 0;
}


int main(int argc, char *argv[])
{
  simCore::checkVersionThrow();

  // Need to get configuration file name
  std::string fileName;
  TopLevelOptions options;
  if (parseCommandLine(argc, argv, fileName, options) != 0)
    return -1;

  if (fileName.empty())
    return 0;  // Not an error, the user asked for an operation that did not return a file name

  simData::MemoryDataStore ds;
  simUtil::DataStoreTestHelper helper(&ds);

  Entities entities(helper);
  CallbackCounters counters;

  if (loadConfigurationFile(fileName, options, entities))
  {
    std::cerr << "Failed to read configuration file: " << fileName << std::endl;
    return -1;
  }

  simData::LinearInterpolator* interpolator = initializeDataStore(ds, helper, options, entities, &counters);

  double updateTime;
  if (options.fileMode)
    updateTime = fileMode(ds, helper, options, entities);
  else
    updateTime = liveMode(ds, helper, options, entities);

  std::cout << "Done, Average Update Rate (milliseconds) = " << updateTime * 1000.0 / (options.numberOfSeconds*options.frameRate) << std::endl;
  // The sleep helps with looking at the data in the Intel tools
  Sleep(1000);

  cleanUpDataStore(ds, options, entities, counters);
  delete interpolator;

  return 0;
}

