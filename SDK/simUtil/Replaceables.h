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
#ifndef SIMUTIL_REPLACEABLES_H
#define SIMUTIL_REPLACEABLES_H

#include "osg/observer_ptr"
#include "simCore/Common/Common.h"
#include "simCore/Time/String.h"
#include "simCore/String/TextReplacer.h"

namespace simCore { class Clock; }
namespace simVis {
  class Viewer;
  class View;
}

// These are not very useful, because the units are fixed.
// But they do demonstrate how to get various values out of the viewer.
namespace simUtil {

/// Display current time (seconds)
class SDKUTIL_EXPORT TimeVariable : public simCore::TextReplacer::Replaceable
{
public:
  /** Constructor */
  TimeVariable(const simCore::Clock& clock);
  virtual std::string getText() const;
  virtual std::string getVariableName() const;

  /** Sets the time format for display */
  void setFormat(simCore::TimeFormat timeFormat);
  /** Retrieves currently displayed time format */
  simCore::TimeFormat format() const;
  /** Cycles between formats */
  void cycleFormat();

private:
  const simCore::Clock& clock_;
  simCore::TimeFormat timeFormat_;
  simCore::TimeFormatterRegistry formatters_;
};

/// Display current view azimuth (degrees)
class SDKUTIL_EXPORT AzimuthVariable : public simCore::TextReplacer::Replaceable
{
public:
  /** Constructor */
  AzimuthVariable(simVis::View* mainView);
  virtual std::string getText() const;
  virtual std::string getVariableName() const;
private:
  std::string format_(double value) const;
private:
  osg::observer_ptr<simVis::View> mainView_;
};

/// Displays current view elevation (degrees)
class SDKUTIL_EXPORT ElevationVariable : public simCore::TextReplacer::Replaceable
{
public:
  /** Constructor */
  ElevationVariable(simVis::View* mainView);
  virtual std::string getText() const;
  virtual std::string getVariableName() const;
private:
  std::string format_(double value) const;
private:
  osg::observer_ptr<simVis::View> mainView_;
};

/// Display current focal point latitude (degrees)
class SDKUTIL_EXPORT LatitudeVariable : public simCore::TextReplacer::Replaceable
{
public:
  /** Constructor */
  LatitudeVariable(simVis::View* mainView, int precision=2);
  virtual std::string getText() const;
  virtual std::string getVariableName() const;
private:
  std::string format_(double value) const;
private:
  osg::observer_ptr<simVis::View> mainView_;
  int precision_;
};

/// Display current focal point longitude (degrees)
class SDKUTIL_EXPORT LongitudeVariable : public simCore::TextReplacer::Replaceable
{
public:
  /** Constructor */
  LongitudeVariable(simVis::View* mainView, int precision=2);
  virtual std::string getText() const;
  virtual std::string getVariableName() const;
private:
  std::string format_(double value) const;
private:
  osg::observer_ptr<simVis::View> mainView_;
  int precision_;
};

/// Display eye point altitude (meters)
class SDKUTIL_EXPORT AltitudeVariable : public simCore::TextReplacer::Replaceable
{
public:
  /** Constructor */
  AltitudeVariable(simVis::View* mainView);
  virtual std::string getText() const;
  virtual std::string getVariableName() const;
private:
  std::string format_(double value) const;
private:
  osg::observer_ptr<simVis::View> mainView_;
};

/// Display name of the centered platform (or "None")
class SDKUTIL_EXPORT CenteredVariable : public simCore::TextReplacer::Replaceable
{
public:
  /** Constructor */
  CenteredVariable(simVis::View* mainView);
  virtual std::string getText() const;
  virtual std::string getVariableName() const;
private:
  osg::observer_ptr<simVis::View> mainView_;
};

/// Display name of currently watched platform (empty string if none)
class SDKUTIL_EXPORT WatchedVariable : public simCore::TextReplacer::Replaceable
{
public:
  /** Constructor */
  WatchedVariable(simVis::View* mainView);
  virtual std::string getText() const;
  virtual std::string getVariableName() const;
private:
  osg::observer_ptr<simVis::View> mainView_;
};

} // namespace simUtil

#endif // SIMUTIL_REPLACEABLES_H
