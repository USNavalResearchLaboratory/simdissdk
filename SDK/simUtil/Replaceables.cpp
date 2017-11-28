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
#include "simCore/Calc/Angle.h"
#include "simCore/Time/Clock.h"
#include "simCore/String/TextReplacer.h"
#include "simVis/EarthManipulator.h"
#include "simVis/Entity.h"
#include "simVis/Viewer.h"
#include "simVis/View.h"
#include "simUtil/Replaceables.h"

namespace simUtil
{

namespace {

/// convenience function to determine which view is in focus
const simVis::View* getView(simVis::View* view)
{
  if (!view)
    return NULL;
  const simVis::FocusManager* focusManager =view->getFocusManager();
  if (focusManager && focusManager->getFocusedView())
    return focusManager->getFocusedView();
  return view;
}

}

///////////////////////////////////////////////////////////////////////////

TimeVariable::TimeVariable(const simCore::Clock& clock)
  : clock_(clock),
    timeFormat_(simCore::TIMEFORMAT_SECONDS)
{
}

std::string TimeVariable::getText() const
{
  return formatters_.toString(timeFormat_, clock_.currentTime(), clock_.currentTime().referenceYear(), 2);
}

std::string TimeVariable::getVariableName() const
{
  return "%TIME%";
}

void TimeVariable::setFormat(simCore::TimeFormat format)
{
  timeFormat_ = format;
}

simCore::TimeFormat TimeVariable::format() const
{
  return timeFormat_;
}

void TimeVariable::cycleFormat()
{
  if (timeFormat_ == simCore::TIMEFORMAT_DTG)
    timeFormat_ = simCore::TIMEFORMAT_SECONDS;
  else
    timeFormat_ = static_cast<simCore::TimeFormat>(timeFormat_ + 1);
}

///////////////////////////////////////////////////////////////////////////

AzimuthVariable::AzimuthVariable(simVis::View* mainView)
  : mainView_(mainView)
{
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
AzimuthVariable::AzimuthVariable(simVis::Viewer* viewer)
  : mainView_(viewer == NULL ? NULL : viewer->getMainView())
{
}
#endif

std::string AzimuthVariable::getText() const
{
  const simVis::View* view = getView(mainView_.get());
  if (!view)
    return "";

  const osgEarth::Util::EarthManipulator* manip = view->getEarthManipulator();
  double azimuth = 0.0;
  // Prefer the azimuth from getCompositeEulerAngles on the manipulator
  if (manip != NULL)
  {
    manip->getCompositeEulerAngles(&azimuth, NULL);
    azimuth *= simCore::RAD2DEG; // format_() expects degrees
  }
  else // Fall back to the viewpoint
    azimuth = view->getViewpoint().heading()->as(osgEarth::Units::DEGREES);

  return format_(azimuth);
}

std::string AzimuthVariable::getVariableName() const
{
  return "%AZ%";
}

std::string AzimuthVariable::format_(double value) const
{
  std::stringstream str;
  str << std::fixed << std::setprecision(2) << simCore::angFix360(value) << " deg";
  return str.str();
}

///////////////////////////////////////////////////////////////////////////

ElevationVariable::ElevationVariable(simVis::View* mainView)
  : mainView_(mainView)
{
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
ElevationVariable::ElevationVariable(simVis::Viewer* viewer)
  : mainView_(viewer == NULL ? NULL : viewer->getMainView())
{
}
#endif

std::string ElevationVariable::getText() const
{
  const simVis::View* view = getView(mainView_.get());
  if (!view)
    return "";

  const osgEarth::Util::EarthManipulator* manip = view->getEarthManipulator();
  double elevation = 0.0;
  // Prefer the azimuth from getCompositeEulerAngles on the manipulator
  if (manip != NULL)
  {
    manip->getCompositeEulerAngles(NULL, &elevation);
    elevation = -elevation * simCore::RAD2DEG; // format_() expects degrees
  }
  else // Fall back to the viewpoint
    elevation = -view->getViewpoint().pitch()->as(osgEarth::Units::DEGREES);

  // Correct for angles near 90.0
  if (elevation > 89.8)
    elevation = 90;
  return format_(elevation);
}

std::string ElevationVariable::getVariableName() const
{
  return "%EL%";
}

std::string ElevationVariable::format_(double value) const
{
  std::stringstream str;
  str << std::fixed << std::setprecision(2) << simCore::angFix90(value) << " deg";
  return str.str();
}

///////////////////////////////////////////////////////////////////////////

LatitudeVariable::LatitudeVariable(simVis::View* mainView, int precision)
  : mainView_(mainView),
    precision_(precision)
{
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
LatitudeVariable::LatitudeVariable(simVis::Viewer* viewer)
  : mainView_(viewer == NULL ? NULL : viewer->getMainView()),
    precision_(2)
{
}
#endif

std::string LatitudeVariable::getText() const
{
  const simVis::View* view = getView(mainView_.get());
  if (!view || view->getViewpoint().nodeIsSet())
    return "";
  return format_(view->getViewpoint().focalPoint()->y());
}

std::string LatitudeVariable::getVariableName() const
{
  return "%LAT%";
}

std::string LatitudeVariable::format_(double value) const
{
  std::stringstream str;
  str << std::fixed << std::setprecision(precision_) << value << " deg";
  return str.str();
}

///////////////////////////////////////////////////////////////////////////

LongitudeVariable::LongitudeVariable(simVis::View* mainView, int precision)
  : mainView_(mainView),
    precision_(precision)
{
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
LongitudeVariable::LongitudeVariable(simVis::Viewer* viewer)
  : mainView_(viewer == NULL ? NULL : viewer->getMainView()),
    precision_(2)
{
}
#endif

std::string LongitudeVariable::getText() const
{
  const simVis::View* view = getView(mainView_.get());
  if (!view || view->getViewpoint().nodeIsSet())
    return "";
  return format_(view->getViewpoint().focalPoint()->x());
}

std::string LongitudeVariable::getVariableName() const
{
  return "%LON%";
}

std::string LongitudeVariable::format_(double value) const
{
  std::stringstream str;
  str << std::fixed << std::setprecision(precision_) << value << " deg";
  return str.str();
}

///////////////////////////////////////////////////////////////////////////

AltitudeVariable::AltitudeVariable(simVis::View* mainView)
  : mainView_(mainView)
{
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
AltitudeVariable::AltitudeVariable(simVis::Viewer* viewer)
  : mainView_(viewer == NULL ? NULL : viewer->getMainView())
{
}
#endif

std::string AltitudeVariable::getText() const
{
  const simVis::View* view = getView(mainView_.get());
  if (!view)
    return "";
  return format_(view->getViewpoint().range()->as(osgEarth::Units::METERS));
}

std::string AltitudeVariable::getVariableName() const
{
  return "%ALT%";
}

std::string AltitudeVariable::format_(double value) const
{
  std::stringstream str;
  str << std::fixed << std::setprecision(2) << value << " m";
  return str.str();
}

///////////////////////////////////////////////////////////////////////////

CenteredVariable::CenteredVariable(simVis::View* mainView)
  : mainView_(mainView)
{
}

std::string CenteredVariable::getText() const
{
  if (mainView_.valid())
  {
    // Figure out the right view
    simVis::View* focusedView = mainView_.get();
    const simVis::FocusManager* focusManager = mainView_->getFocusManager();
    if (focusManager && focusManager->getFocusedView())
    {
      focusedView = focusManager->getFocusedView();
    }

    // Pull out the centered node; note that in a Watched view, the Watcher is considered centered
    // even though the tether is set (the tether in this case is the watched)
    const simVis::EntityNode* entityNode = NULL;
    if (focusedView->isWatchEnabled())
      entityNode = focusedView->getWatcherNode();
    else if (focusedView->getCameraTether() != NULL)
      entityNode = dynamic_cast<simVis::EntityNode*>(focusedView->getCameraTether()->getParent(0));

    // Return the name
    if (entityNode)
    {
      std::string rv = entityNode->getEntityName(simVis::EntityNode::DISPLAY_NAME);
      if (rv.empty())
        rv = " ";
      return rv;
    }
  }
  return "None";
}

std::string CenteredVariable::getVariableName() const
{
  return "%CENTERED%";
}

///////////////////////////////////////////////////////////////////////////

WatchedVariable::WatchedVariable(simVis::View* mainView)
  : mainView_(mainView)
{
}

std::string WatchedVariable::getText() const
{
  if (mainView_.valid())
  {
    simVis::View* focusedView = mainView_.get();
    const simVis::FocusManager* focusManager = mainView_->getFocusManager();
    if (focusManager && focusManager->getFocusedView())
    {
      focusedView = focusManager->getFocusedView();
    }

    // Pull out the watched node and make sure it is valid
    if (focusedView->isWatchEnabled() && focusedView->getWatchedNode())
    {
      std::string rv = focusedView->getWatchedNode()->getEntityName(simVis::EntityNode::DISPLAY_NAME);
      if (rv.empty())
        rv = " ";
      return rv;
    }
  }
  return "";
}

std::string WatchedVariable::getVariableName() const
{
  return "%WATCHED%";
}

} // namespace simUtil
