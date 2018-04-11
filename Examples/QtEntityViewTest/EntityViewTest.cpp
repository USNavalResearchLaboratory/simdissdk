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
#include "simData/MemoryDataStore.h"
#include "simQt/EntityTreeComposite.h"
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

MainWindow::MainWindow(QWidget *parent) : QDialog(parent)
{
  dataStore_ = new simData::MemoryDataStore();
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

  connect(mainWindowGui_->platformButton, SIGNAL(clicked()), this, SLOT(addPlatforms_()));
  connect(mainWindowGui_->beamButton, SIGNAL(clicked()), this, SLOT(addBeams_()));
  connect(mainWindowGui_->gateButton, SIGNAL(clicked()), this, SLOT(addGates_()));
  connect(mainWindowGui_->testButton, SIGNAL(clicked()), this, SLOT(test_()));
  connect(mainWindowGui_->deleteButton, SIGNAL(clicked()), this, SLOT(deleteEntity_()));

  entityTreeModel_ = new simQt::EntityTreeModel(NULL, dataStore_);
  entityTreeComposite_ = mainWindowGui_->entityTreeComposite;
  entityTreeComposite_->setModel(entityTreeModel_);
  connect(entityTreeComposite_, SIGNAL(itemsSelected(QList<uint64_t>)), this, SLOT(itemsSelected_(QList<uint64_t>)));
  connect(entityTreeComposite_, SIGNAL(itemDoubleClicked(uint64_t)), this, SLOT(itemDoubleClicked_(uint64_t)));
  entityTreeComposite_->addButton(new QPushButton("Test", parent));
}

MainWindow::~MainWindow()
{
  delete mainWindowGui_;
  delete entityTreeModel_;
  delete dataStore_;
}

void MainWindow::addPlatforms_()
{
  simData::ObjectId platformId = createPlatform(dataStore_, "Sample Platform 1");
  entityTreeComposite_->setSelected(platformId, true);
}

void MainWindow::addBeams_()
{
  simData::ObjectId beamId = createBeam(dataStore_, 9, "Beam 2");
  entityTreeComposite_->setSelected(beamId, true);
}

void MainWindow::addGates_()
{
  simData::ObjectId gateId = createGate(dataStore_, 10, "Gate 2");
  entityTreeComposite_->setSelected(gateId, true);
}

void MainWindow::test_()
{
  for (int ii = 0; ii < 10000; ii++)
  {
    QString name = QString("New Platform %1").arg(ii, 0, 16);
    createPlatform(dataStore_, name);
  }
}

void MainWindow::itemsSelected_(QList<uint64_t> ids)
{
}

void MainWindow::itemDoubleClicked_(uint64_t id)
{
}

void MainWindow::deleteEntity_()
{
  QList<uint64_t> selectedItems = entityTreeComposite_->selectedItems();
  Q_FOREACH(uint64_t id, selectedItems)
  {
    dataStore_->removeEntity(id);
  }
}

//////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  MainWindow* window = new MainWindow(NULL);
  window->show();

  int rv = app.exec();
  delete window;
  return rv;
}

