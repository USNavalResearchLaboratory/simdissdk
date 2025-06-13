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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simCore/System/Utils.h"
#include "simData/MemoryDataStore.h"
#include "simQt/EntityTreeModel.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"

namespace
{

simData::ObjectId createPlatform(simData::DataStore* dataStore, const QString& name)
{
  simData::DataStore::Transaction xaction;
  simData::PlatformProperties* props = dataStore->addPlatform(&xaction);
  simData::ObjectId id = props->id();
  if (id == 1)
    props->set_originalid(id*100);
  else
    props->set_originalid(id);
  xaction.complete(&props);

  simData::DataStore::Transaction transaction;
  simData::PlatformPrefs* prefs = dataStore->mutable_platformPrefs(id, &transaction);
  assert(prefs);
  prefs->mutable_commonprefs()->set_name(name.toStdString());
  transaction.complete(&prefs);

  return id;
}

simData::ObjectId createBeam(simData::DataStore* dataStore, simData::ObjectId platformId, const QString& name)
{
  simData::DataStore::Transaction xaction;
  simData::BeamProperties* props = dataStore->addBeam(&xaction);
  simData::ObjectId id = props->id();
  props->set_hostid(platformId);
  props->set_originalid(id);
  xaction.complete(&props);

  simData::DataStore::Transaction transaction;
  simData::BeamPrefs* prefs = dataStore->mutable_beamPrefs(id, &transaction);
  assert(prefs);
  prefs->mutable_commonprefs()->set_name(name.toStdString());
  transaction.complete(&prefs);

  return id;
}

simData::ObjectId createGate(simData::DataStore* dataStore, simData::ObjectId beamId, const QString& name)
{
  simData::DataStore::Transaction xaction;
  simData::GateProperties* props = dataStore->addGate(&xaction);
  simData::ObjectId id = props->id();
  props->set_hostid(beamId);
  props->set_originalid(id);
  xaction.complete(&props);

  simData::DataStore::Transaction transaction;
  simData::GatePrefs* prefs = dataStore->mutable_gatePrefs(id, &transaction);
  assert(prefs);
  prefs->mutable_commonprefs()->set_name(name.toStdString());
  transaction.complete(&prefs);

  return id;
}

}

MainWindow::MainWindow(QWidget *parent, simData::DataStore* dataStore) : QDialog(parent), dataStore_(dataStore)
{
  simData::ObjectId platformId = createPlatform(dataStore_, "Platform 1");
  simData::ObjectId beamId = createBeam(dataStore_, platformId, "Beam 1");
  simData::ObjectId gateId = createGate(dataStore_, beamId, "Gate 1");

  createPlatform(dataStore_, "Platform 2");
  createPlatform(dataStore_, "Platform 3");
  createPlatform(dataStore_, "Platform 4");
  createPlatform(dataStore_, "Platform 5");
  createPlatform(dataStore_, "Platform 6");
  createPlatform(dataStore_, "Platform 7");

  mainWindowGui_ = new Ui_MainWindow();
  mainWindowGui_->setupUi(this);

  connect(mainWindowGui_->platformButton, SIGNAL(clicked()), this, SLOT(addPlatform_()));
  connect(mainWindowGui_->beamButton, SIGNAL(clicked()), this, SLOT(addBeam_()));
  connect(mainWindowGui_->gateButton, SIGNAL(clicked()), this, SLOT(addGate_()));
  connect(mainWindowGui_->addManyButton, SIGNAL(clicked()), this, SLOT(addPlatforms_()));
  connect(mainWindowGui_->deleteButton, SIGNAL(clicked()), this, SLOT(delete_()));
  connect(mainWindowGui_->renameButton, SIGNAL(clicked()), this, SLOT(rename_()));

  entityTreeModel_ = new simQt::EntityTreeModel(nullptr, dataStore_);
  entityTreeModel_->setToListView();
  mainWindowGui_->entityLine->setModel(entityTreeModel_, simData::PLATFORM);
  connect(mainWindowGui_->entityLine, SIGNAL(itemSelected(uint64_t)), this, SLOT(itemSelected_(uint64_t)));
}

void MainWindow::itemSelected_(uint64_t id)
{
}

MainWindow::~MainWindow()
{
  mainWindowGui_->entityLine->setModel(nullptr, simData::PLATFORM);
  delete mainWindowGui_;
  delete entityTreeModel_;
}

void MainWindow::addPlatform_()
{
  simData::ObjectId platformId = createPlatform(dataStore_, "Sample Platform 1");
}

void MainWindow::addBeam_()
{
  simData::ObjectId beamId = createBeam(dataStore_, 9, "Beam 2");
}

void MainWindow::addGate_()
{
  simData::ObjectId gateId = createGate(dataStore_, 10, "Gate 2");
}

void MainWindow::addPlatforms_()
{
  for (int ii = 0; ii < 10000; ii++)
  {
    QString name = QString("New Platform %1").arg(ii, 0, 16);
    createPlatform(dataStore_, name);
  }
}

void MainWindow::delete_()
{
  dataStore_->removeEntity(1);
}

void MainWindow::rename_()
{
  simData::DataStore::Transaction txn;
  simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(1, &txn);
  prefs->mutable_commonprefs()->set_name("New Name");
  txn.complete(&prefs);
}

//////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  simCore::initializeSimdisEnvironmentVariables();
  QApplication app(argc, argv);

  simData::MemoryDataStore* dataStore = new simData::MemoryDataStore();
  MainWindow* window = new MainWindow(nullptr, dataStore);
  window->show();

  int rv = app.exec();
  delete window;
  delete dataStore;
  return rv;
}

