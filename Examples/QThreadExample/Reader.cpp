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

#include <QThread>

#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"

#include "DataGenerator.h"
#include "Reader.h"

namespace SdkQThreadExample
{

Reader::Reader(simData::DataStore& dataStore)
  : dataStore_(dataStore),
    threadedDataGen_(NULL),
    id_(0),
    numberProcessed_(0)
{
}

Reader::~Reader()
{
  stop();
}

void Reader::start()
{
  if (threadedDataGen_ != NULL)
    return;


  numberProcessed_ = 0;

  QThread *thread = new QThread;
  threadedDataGen_ = new DataGenerator();

  // Start the thread with the reader
  threadedDataGen_->moveToThread(thread);
  // http://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
  connect(threadedDataGen_, SIGNAL(newData(double, double, double)), this, SLOT(addDataPoint_(double, double, double)));
  connect(thread, SIGNAL(started()), threadedDataGen_, SLOT(start()));
  connect(threadedDataGen_, SIGNAL(finished()), thread, SLOT(quit()));
  connect(threadedDataGen_, SIGNAL(finished()), threadedDataGen_, SLOT(deleteLater()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  thread->start();
}

void Reader::stop()
{
  if (threadedDataGen_ != NULL)
  {
    // Disconnect before shutting down to prevent race condition
    disconnect(threadedDataGen_, SIGNAL(newData(double, double, double)), this, SLOT(addDataPoint_(double, double, double)));
    threadedDataGen_->stop();
  }

  // thread_ gets deleted automatically with deleteLater() signal
  threadedDataGen_ = NULL;
  id_ = 0;
}

simData::ObjectId Reader::addPlatform_()
{
  simData::ObjectId hostId;

  // create the platform
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformProperties* props = dataStore_.addPlatform(&xaction);
    hostId = props->id();
    xaction.complete(&props);
  }

  // configure initial preferences
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(hostId, &xaction);
    prefs->set_icon("aqm-37c/aqm-37c.ive");
    prefs->set_scale(1000.0);  // large so we can see it
    prefs->set_dynamicscale(false);
    prefs->mutable_commonprefs()->set_name("My Platform");
    prefs->mutable_commonprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  return hostId;
}

void Reader::addPlatformPoint_(simData::ObjectId id, double time, double lat, double lon, double alt)
{
  simCore::Coordinate lla(
    simCore::COORD_SYS_LLA,
    simCore::Vec3(lat, lon, alt),
    simCore::Vec3(0.0, 0.0, 0.0));  // Define orientation otherwise get strange results
  simCore::Coordinate ecef;
  simCore::CoordinateConverter::convertGeodeticToEcef(lla, ecef);  // DataStore needs ECEF

  simData::DataStore::Transaction t;
  simData::PlatformUpdate* u = dataStore_.addPlatformUpdate(id, &t);
  u->set_time(time);
  u->set_x(ecef.x());
  u->set_y(ecef.y());
  u->set_z(ecef.z());
  u->set_psi(ecef.psi());
  u->set_theta(ecef.theta());
  u->set_phi(ecef.phi());
  t.complete(&u);

  dataStore_.update(time);
}

void Reader::addDataPoint_(double lat, double lon, double alt)
{
  if (id_ == 0)
    id_ = addPlatform_();

  if (id_ != 0)
    addPlatformPoint_(id_, numberProcessed_, lat, lon, alt);  // Use numberProcessed_ as a time


  numberProcessed_++;
}

unsigned int Reader::numberProcessed() const
{
  return numberProcessed_;
}

}

