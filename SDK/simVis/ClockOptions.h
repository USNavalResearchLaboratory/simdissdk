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

#ifndef SIMVIS_CLOCK_OPTIONS
#define SIMVIS_CLOCK_OPTIONS

#include <osgDB/Options>
#include "simCore/Time/Clock.h"

namespace simCore { class Clock; }

namespace simVis
{

/** Adds a simCore::Clock to an osgDB::Options, useful for passing clock to OSG plugins */
class SDKVIS_EXPORT ClockOptions : public osgDB::Options
{
public:
  /** Constructs an osgDB::Options derived class around the clock pointer */
  explicit ClockOptions(simCore::Clock* clock);

  /** Retrieves the clock pointer from the constructor */
  simCore::Clock* clock() const;

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "ClockOptions"; }

protected:
  virtual ~ClockOptions();

private:
  simCore::Clock* clock_;
};

}

#endif

