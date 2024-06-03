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

#include "DataGenerator.h"

namespace SdkQThreadExample
{

DataGenerator::DataGenerator()
  : QObject(),
    timer_(nullptr),
    done_(false),
    lat_(0.0),
    lon_(0.0),
    alt_(0.0)
{
}

DataGenerator::~DataGenerator()
{
}

void DataGenerator::start()
{
  ///Define PI locally
  const double s_PI = 3.141592654;
  ///Define DEG2RAD locally
  const double s_DEG2RAD = s_PI / 180.0;

  // Make some fake data compatible with Simple Server
  lat_ = 22.0 * s_DEG2RAD;
  lon_ = -159.0 * s_DEG2RAD;
  alt_ = 100; // Meters

  // Must create the timer in the start so that it is in the correct thread
  timer_ = new QTimer();
  // When the timer times out it will call the routine update_
  connect(timer_, SIGNAL(timeout()), this, SLOT(update_()));
  // Start the timer with an update rate specified in milliseconds
  timer_->start(100);
}

void DataGenerator::update_()
{
  if (done_)
  {
    // Stop the timer
    timer_->stop();
    delete timer_;
    timer_ = nullptr;
    // Send out signal indicating the thread is done
    Q_EMIT finished();
    return;
  }

  // Send out a data point
  Q_EMIT newData(lat_, lon_, alt_);
  lat_ += 0.00001;  // Move the platform North
}

void DataGenerator::stop()
{
  // mutex is not required here because the setting of the boolean is atomic
  done_ = true;

  // Cannot stop the timer here since this call is not in the thread
}

}

