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
#ifndef SIMUTIL_STATSHANDLER_H
#define SIMUTIL_STATSHANDLER_H

#include <osg/observer_ptr>
#include <osgViewer/ViewerEventHandlers>
#include "simCore/Common/Export.h"

namespace simVis { class View; }

namespace simUtil {

/**
 * Specialization of the osgViewer::StatsHandler that allows for easy programmatic
 * changes to the currently displayed statistics.  Note that the default hotkeys
 * for the osgViewer::StatsHandler ('s' and 'S') are not respected unless explicitly
 * set by the user.
 */
class SDKUTIL_EXPORT StatsHandler : public osgViewer::StatsHandler
{
public:
  /** Typedef the base class StatsType for ease of use */
  typedef osgViewer::StatsHandler::StatsType StatsType;

  /**
   * Instantiate a new StatsHandler.  This instance should be associated with any view
   * or viewer using the addEventHandler() call, otherwise window resize events will
   * not be observed.
   */
  StatsHandler();

  /**
   * Programmatically alter the stats type shown.  This is equivalent to pressing the
   * toggling hotkey specified in setKeyeventTogglesOnScreenStats().
   * @param statsType Statistics to show on the main view
   * @param onWhichView View with which to associate the stats
   */
  void setStatsType(StatsType statsType, simVis::View* onWhichView);

  /** Cycles to the next stats type for the given view. */
  void cycleStats(simVis::View* onWhichView);

  /** Retrieves the currently displayed statistics. */
  StatsType statsType() const;

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "StatsHandler"; }

private:
  /** Safely bounds the enum to [0,LAST) */
  StatsType validate_(StatsType type) const;
};

} // end namespace simUtil

#endif /* SIMUTIL_STATSHANDLER_H */
