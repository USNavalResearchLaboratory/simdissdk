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
#ifndef SIMUTIL_MAPSCALE_H
#define SIMUTIL_MAPSCALE_H

#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "osg/Group"
#include "simCore/Common/Common.h"

namespace osgText { class Font; }
namespace simCore { class Units; }
namespace simVis {
  class FocusManager;
  class Text;
  class View;
}

namespace simUtil {

/**
 * simUtil::MapScale is a graphic that can be placed in a HUD in order to draw a representative
 * scale of the view that it monitors.  The scale shows a human readable, rounded value in the
 * provided units (via UnitsProvider), and draws demarcations from the 0.0 to the maximum value
 * at reasonable intervals.
 */
class SDKUTIL_EXPORT MapScale : public osg::Group
{
public:
  /**
   * Interface for a class that will provide the appropriate units to use in a map scale,
   * given a maximum range of units.  The maximum range specifies the range from the start
   * of the legend graphic to the end of the legend graphic.  Note that the actual range
   * displayed will be smaller than this due to rounding to whole numbers, and the rounding
   * is directly dependent on the units representation of numbers displayed.
   */
  class UnitsProvider : public osg::Referenced
  {
  public:
    /** Given a maximum range in meters, returns the unit type to use in the map scale. */
    virtual const simCore::Units& units(double maxRangeM) const = 0;

  protected:
    /** Derived from osg::Referenced */
    virtual ~UnitsProvider() {}
  };

  /** Default constructor. */
  MapScale();
  /** Copy constructor. */
  explicit MapScale(const MapScale& rhs, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

  /** Define attributes for osg::Node inheritance. */
  META_Node(simUtil, MapScale);

  /** Changes the view for which the scale is showing distances. */
  void setView(simVis::View* view);
  /** Binds the class to a focus manager, such that changes in focus calls setView() */
  void bindToFocusManager(simVis::FocusManager* focusManager);

  /** Sets a provider that will give a unit given a max range of the scale.  Useful for changing units when zooming. */
  void setUnitsProvider(UnitsProvider* unitsProvider);
  /** Retrieves the currently configured units provider.  This is never NULL. */
  UnitsProvider* unitsProvider() const;

  /** Retrieves current total height in pixels. */
  float height() const;

  /** Changes the target width of the scale.  Note that text values may stray outside the bounds. */
  void setWidth(float widthPx);
  /** Retrieves current width in pixels. */
  float width() const;

  /** Changes the color of the unit type text. */
  void setUnitsColor(const osg::Vec4f& color);
  /** Changes the font of the unit type text. */
  void setUnitsFont(osgText::Font* font);
  /** Changes the character size (height) of the unit type text. */
  void setUnitsCharacterSize(float sizePx);

  /** Changes the color of the values text. */
  void setValuesColor(const osg::Vec4f& color);
  /** Changes the font of the values text. */
  void setValuesFont(osgText::Font* font);
  /** Changes the character size (height) of the values text. */
  void setValuesCharacterSize(float sizePx);

  /** Sets the height of the bar used to demarcate segments of range. */
  void setBarHeight(float sizePx);
  /** Changes the first bar color. */
  void setBarColor1(const osg::Vec4f& color);
  /** Changes the second bar color. */
  void setBarColor2(const osg::Vec4f& color);

protected:
  /** Derived from osg::Referenced */
  virtual ~MapScale();

private:
  /** Use an update callback to recalculate the map scale */
  class UpdateCallback;

  /** Determines heightPx_ based on height of text and bars, and adjusts text position. */
  void recalculateHeight_();
  /** Draws bars out to maxValue using numDivisions, with a width provided and a given precision for text */
  void drawBars_(double maxValue, unsigned int numDivisions, float width, unsigned int precision);

  /** Determines the distance of the map scale in meters.  Uses map intersection testing. */
  void recalculatePixelDistance_();
  /** Given the max range of the map scale graphic, shrinks to the nearest whole number in appropriate units. */
  void recalculateScale_(double maxDataRangeM);
  /** Given a value, converts to a string with given precision. */
  std::string valueToString_(double value, unsigned int precision) const;

  /** Internal, calculated height based on text height and bar height */
  float heightPx_;
  /** User-provided width of the legend in pixels */
  float widthPx_;

  /** Geode holding the text and display data */
  osg::ref_ptr<osg::Geode> geode_;

  /** Representative text for the values on the top */
  osg::ref_ptr<simVis::Text> valueTextPrototype_;
  /** Displays the unit type on the bottom, such as "meters" */
  osg::ref_ptr<simVis::Text> unitsText_;

  /** Height of the bar in pixels */
  float barHeightPx_;
  /** First color for the bar */
  osg::Vec4f barColor1_;
  /** Second color for the bar */
  osg::Vec4f barColor2_;

  /** View that provides the data required for determining the scale */
  osg::observer_ptr<simVis::View> view_;
  /** Units provider (non-NULL) that gives units for current range */
  osg::ref_ptr<UnitsProvider> unitsProvider_;
};

/** Simple implementation of MapScale::UnitsProvider that always returns a single units value. */
class /*SDKUTIL_EXPORT*/ MapScaleOneUnitProvider : public MapScale::UnitsProvider  // Header-only
{
public:
  explicit MapScaleOneUnitProvider(const simCore::Units& units)
    : units_(units)
  {
  }

  // Override from MapScaleUnitsProvider
  virtual const simCore::Units& units(double maxRangeM) const
  {
    return units_;
  }

protected:
  virtual ~MapScaleOneUnitProvider() {}

private:
  const simCore::Units& units_;
};

/**
 * Implementation of MapScale::UnitsProvider that toggles between two units based on a given
 * cut-off value.  For ranges below the cut-off, a smaller distance unit is returned.  For
 * ranges at or above the cut-off, a larger distance units is returned.  This class can be
 * used to provide a metric scale or an imperial scale using appropriate units.
 */
class /*SDKUTIL_EXPORT*/ MapScaleTwoUnitsProvider : public MapScale::UnitsProvider  // Header-only
{
public:
  MapScaleTwoUnitsProvider(const simCore::Units& smallUnits, const simCore::Units& largeUnits, double cutoffM)
    : smallUnits_(smallUnits),
      largeUnits_(largeUnits),
      cutoffM_(cutoffM)
  {
  }

  // Override from MapScaleUnitsProvider
  virtual const simCore::Units& units(double maxRangeM) const
  {
    if (maxRangeM < cutoffM_)
      return smallUnits_;
    return largeUnits_;
  }

protected:
  virtual ~MapScaleTwoUnitsProvider() {}

private:
  const simCore::Units& smallUnits_;
  const simCore::Units& largeUnits_;
  double cutoffM_;
};

}

#endif /* SIMUTIL_MAPSCALE_H */
