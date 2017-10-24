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

/**
 * ObserverMaker Example
 *
 * Demonstrates how to use MakeObserver functions to create a DataStore
 * observer object.  This example does not create a graphics window.
 */

#include <iostream>

#include "osg/Group"

#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simData/MemoryDataStore.h"

using namespace std;
using namespace simData;

//----------------------------------------------------------------------------
/// Function for adding a platform to the scene
class BasicAdd : public simData::DataStore::DefaultListener
{
public:
  void onAddEntity(simData::DataStore *source, simData::ObjectId id, simData::ObjectType ot)
  {
    if (ot != simData::PLATFORM)
      return;

    DataStore::Transaction transaction;
    const PlatformPrefs *prefs = source->platformPrefs(id, &transaction);

    cout << "Basic Callback Function:" << endl;
    cout << "\tAdding platform " << prefs->commonprefs().name() << " to the scene as a " << prefs->icon() << endl;
    cout << endl;

    transaction.complete(&prefs);
  }
};

/// Function for adding a platform to the scene as the child of a specific node
class AddAsChild : public simData::DataStore::DefaultListener
{
public:
  explicit AddAsChild(osg::Node *parent)
  : parent_(parent)
  {
  }

  void onAddEntity(simData::DataStore *source, simData::ObjectId id, simData::ObjectType ot)
  {
    if (ot != simData::PLATFORM)
      return;

    DataStore::Transaction transaction;
    const PlatformPrefs *prefs = source->platformPrefs(id, &transaction);

    cout << "Callback Function with User Data:" << endl;
    cout << "\tAdding platform " << prefs->commonprefs().name() << " to the scene as a " << prefs->icon() << endl;
    cout << "\tPlatform parent is node with address " << parent_ << endl;
    cout << endl;

    transaction.complete(&prefs);
  }

private:
  osg::Node *parent_;
};

/// Representation of a class responsible for managing the objects in a scene
class SceneManager
{
public: // types
  class BasicListener : public DataStore::DefaultListener
  {
  public:
    explicit BasicListener(SceneManager *sceneManager)
    : sceneManager_(sceneManager)
    {
    }

    void onAddEntity(simData::DataStore *source, simData::ObjectId id, simData::ObjectType ot)
    {
      if (ot == simData::PLATFORM)
        sceneManager_->addPlatform(id, source);
    }

  private:
    SceneManager *sceneManager_;
  };

  class ListenerWithArg : public DataStore::DefaultListener
  {
  public:
    ListenerWithArg(SceneManager *sceneManager, osg::Node *arg)
    : sceneManager_(sceneManager),
      arg_(arg)
    {
    }

    void onAddEntity(simData::DataStore *source, simData::ObjectId id, simData::ObjectType ot)
    {
      if (ot == simData::PLATFORM)
        sceneManager_->addPlatformAsChild(id, source, arg_);
    }

  private:
    SceneManager *sceneManager_;
    osg::ref_ptr<osg::Node> arg_;
  };
public: // methods
  /// Function for adding a platform to the scene
  void addPlatform(ObjectId id, DataStore *source)
  {
    DataStore::Transaction transaction;
    const PlatformPrefs *prefs = source->platformPrefs(id, &transaction);

    cout << "Class Member Callback Function:" << endl;
    cout << "\tAdding platform " << prefs->commonprefs().name() << " to the scene as a " << prefs->icon() << endl;
    cout << endl;

    transaction.complete(&prefs);
  }

  /// Function for adding a platform to the scene as the child of a specific node
  void addPlatformAsChild(ObjectId id, DataStore *source, osg::Node *parent)
  {
    DataStore::Transaction transaction;
    const PlatformPrefs *prefs = source->platformPrefs(id, &transaction);

    cout << "Class Member Callback Function with User Data:" << endl;
    cout << "\tAdding platform " << prefs->commonprefs().name() << " to the scene as a " << prefs->icon() << endl;
    cout << "\tPlatform parent is node with address " << parent << endl;
    cout << endl;

    transaction.complete(&prefs);
  }
};

/// Function to report changes to a platform's preference settings
struct PlatformPrefListener : public simData::DataStore::DefaultListener
{
  void onPrefsChange(DataStore *source, simData::ObjectId id)
  {
    if (source->objectType(id) != simData::PLATFORM)
      return;

    DataStore::Transaction transaction;
    const PlatformPrefs *prefs = source->platformPrefs(id, &transaction);

    cout << "Basic Callback Function (Preferences):" << endl;
    cout << "\tPlatform's name is now " << prefs->commonprefs().name() << endl;
    cout << endl;

    transaction.complete(&prefs);
  }
};

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  SIM_NOTICE << "NOTIFICATION MAKER EXAMPLE" << std::endl << std::endl;

  // The root node for the scene
  osg::ref_ptr<osg::Node> rootnode = new osg::Group();

  // The scene manager
  SceneManager manager;

  // The data store
  MemoryDataStore datastore;

  // Make the observers
  DataStore::ListenerPtr functionNotification(new BasicAdd);

  DataStore::ListenerPtr functionArgNotification(new AddAsChild(rootnode.get()));

  DataStore::ListenerPtr classMethodNotification(new SceneManager::BasicListener(&manager));

  DataStore::ListenerPtr classMethodArgNotification(new SceneManager::ListenerWithArg(&manager, rootnode.get()));

  DataStore::ListenerPtr prefsFunctionNotification((new PlatformPrefListener));

  // Add the observers to the data store
  datastore.addListener(functionNotification);
  datastore.addListener(functionArgNotification);
  datastore.addListener(classMethodNotification);
  datastore.addListener(classMethodArgNotification);

  datastore.addListener(prefsFunctionNotification);

  // Add a new platform; this will cause the observer objects to print
  // their messages to the console when the transaction is completed/released
  SIM_NOTICE << "Initiating add new platform transaction..." << std::endl;

  DataStore::Transaction transaction;

  PlatformProperties *props = datastore.addPlatform(&transaction);
  simData::ObjectId id = props->id();    // Store the id for later use

  // Complete the transaction (commit transaction and release transaction
  // handle) and raise notifications
  SIM_NOTICE << "Completing transaction" << std::endl;

  transaction.complete(&props);

  SIM_NOTICE << "Transaction complete" << std::endl << std::endl;

  // Change platform's name; this will cause the preference observer object to
  // print its message to the console when the transaction is completed/released
  SIM_NOTICE << "Initiating platform preference change transaction..." << std::endl;

  PlatformPrefs *prefs = datastore.mutable_platformPrefs(id, &transaction);
  prefs->set_icon("sphere.opt");         // Use a sphere to represent platform
  prefs->mutable_commonprefs()->set_name("Modified Platform");    // Name to display in scene

  // Complete the transaction (commit transaction and release transaction
  // handle) and raise notifications
  SIM_NOTICE << "Completing transaction" << std::endl;

  transaction.complete(&prefs);

  SIM_NOTICE << "Transaction complete" << std::endl << std::endl;

  // Perform a platform prefs transaction, but don't change anything; the preference
  // observer object will not print a message to the console when the transaction is completed/released
  SIM_NOTICE << "Initiating platform preference change transaction..." << std::endl;

  prefs = datastore.mutable_platformPrefs(id, &transaction);

  // Complete the transaction (commit transaction and release transaction
  // handle) and raise notifications
  SIM_NOTICE << "Completing transaction" << std::endl;

  transaction.complete(&prefs);

  SIM_NOTICE << "Transaction complete" << std::endl;

  return 0;
}

