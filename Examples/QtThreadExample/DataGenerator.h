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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#ifndef SDK_QTHREAD_EXAMPLE_DATA_GENERATOR
#define SDK_QTHREAD_EXAMPLE_DATA_GENERATOR

#include <QObject>
#include <QTimer>


namespace SdkQThreadExample
{

/**
 * A example class for showing QThreads.
 * Signals interface circumvents the need for a mutex because of delayed Queued Connection (via Auto Connect).
 * See http://qt-project.org/doc/qt-4.8/threads-qobject.html#signals-and-slots-across-threads for details.
 */
class DataGenerator : public QObject
{
  Q_OBJECT;
public:
  DataGenerator();
  virtual ~DataGenerator();

Q_SIGNALS:
  /** Signaled when finally finishes. */
  void finished();
  /** Signaled when new data arrives (radians, radians, meters) */
  void newData(double lat, double lon, double alt);

public Q_SLOTS:
  /** Starts the loop with the file descriptor set up on construction */
  void start();
  /** Tell the loop to stop execution; returns immediately, listen to the finished signal if need to know the actual finish */
  void stop();

  /** Slots not part of the public interface.  http://stackoverflow.com/questions/9147636/qt-private-slots-what-is-this */
private Q_SLOTS:
  /** Called by the QTimer to generate more data */
  void update_();

private:
  QTimer* timer_; ///< Used to generate the data
  bool done_;  ///< Set to true when the thread should exit
  double lat_;  ///< Latitude in radians
  double lon_;  ///< Longitude in radians
  double alt_;  ///< Altitude in meters
};

}

#endif
