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

#ifndef SDK_QTHREAD_EXAMPLE_READER
#define SDK_QTHREAD_EXAMPLE_READER

#include <QObject>

#include "simData/DataStore.h"

namespace SdkQThreadExample
{

class DataGenerator;

/** Example class using a thread to get platform data */
class Reader : public QObject
{
  Q_OBJECT;
public:
  explicit Reader(simData::DataStore& dataStore);
  virtual ~Reader();

  /** Returns the number of data points processed */
  unsigned int numberProcessed() const;

signals:
  /** Signaled when finally finishes. */
  void finished();

public slots:
  /** Starts the reading of data */
  void start();
  /** Stops the reading of data */
  void stop();

private slots:
  /** The reader get data from the thread via this slot */
  void addDataPoint_(double lat, double lon, double alt);

private:
  /** Adds a platform to the DataStore */
  simData::ObjectId addPlatform_();
  /** Adds a data point to the DataStore for the given platform id */
  void addPlatformPoint_(simData::ObjectId id, double time, double lat, double lon, double alt);

  simData::DataStore& dataStore_; ///< The DataStore that hold all the data
  DataGenerator* threadedDataGen_;  ///< The thread that generates the data
  simData::ObjectId id_; ///< The unique ID for the platform
  unsigned int numberProcessed_;  ///< Number of points added to the platform

};

}

#endif
