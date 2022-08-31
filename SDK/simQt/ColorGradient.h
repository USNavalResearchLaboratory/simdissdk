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
#ifndef SIMQT_COLORGRADIENT_H
#define SIMQT_COLORGRADIENT_H

#include <map>
#include <utility>
#include <vector>
#include <QColor>
#include <QMetaType>
#include "osg/TransferFunction"
#include "simCore/Common/Export.h"

#define OLD_SIMQT_COLORGRADIENT_API
#define NEW_SIMQT_COLORGRADIENT_API

namespace simQt {

/** String template to format a QLinearGradient background like our UTILS::ColorGradient */
static const QString GRADIENT_STR_TEMPLATE = "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, %1);";
/** Format for each stop.  %1 is the percentage (0-1), %2 is a RGBA color string. */
static const QString GRADIENT_STOP_TEMPLATE = "stop: %1 rgba(%2)";

/**
 * Represents a color gradient between magnitude values 0 and 1.
 * Wraps osg::TransferFunction1D as underlying implementation.
 * This class enforces a minimum of two color stops at all times.
 */
class SDKQT_EXPORT ColorGradient
{
public:
  /** Creates a default gradient. */
  ColorGradient();

#ifdef OLD_SIMQT_COLORGRADIENT_API
  /** Creates a gradient with colors in given range. Values outside [0,1] are discarded. */
  explicit ColorGradient(const std::map<float, QColor>& colors);
  /** Creates a gradient with colors in given range. Values outside [0,1] are discarded. */
  explicit ColorGradient(const std::map<float, osg::Vec4>& colors);
#endif // OLD_SIMQT_COLORGRADIENT_API

  /** Copy constructor required for dynamic memory */
  ColorGradient(const ColorGradient& rhs);

  /** Assignment operator required for dynamic memory */
  ColorGradient& operator=(const ColorGradient& rhs);

  virtual ~ColorGradient();

  // Factory methods for making new built-in gradients
  static ColorGradient newDefaultGradient();
  static ColorGradient newDarkGradient();
  static ColorGradient newGreyscaleGradient();
  static ColorGradient newDopplerGradient();
  /** Interpolates a color between lowColor and highColor, using low and high as guideposts against val. */
  static QColor interpolate(const QColor& lowColor, const QColor& highColor, float low, float val, float high);

  /** Set whether this gradient is discrete. If true, colorAt() and osgColorAt() will not interpolate colors between stops. */
  void setDiscrete(bool discrete);
  /** Get the discrete flag. */
  bool discrete() const;

  /**
   * Retrieves the color mapping to the given value. Values range [0,1].
   * Colors outside the configured min/max values will be clamped.
   * If no colors have been set, then black is returned. If discrete flag
   * is set to true (via setDiscrete()), no interpolation is performed.
   */
  QColor colorAt(float zeroToOne) const;

  /**
   * Retrieves the color mapping to the given value. Values range [0,1].
   * Colors outside the configured min/max values will be clamped.
   * If no colors have been set, then black is returned. If discrete flag
   * is set to true (via setDiscrete()), no interpolation is performed.
   */
  osg::Vec4 osgColorAt(float zeroToOne) const;

#ifdef OLD_SIMQT_COLORGRADIENT_API
  /** Adds a control color, returning 0 on success. Overwrites existing colors. */
  int setColor(float zeroToOne, const QColor& color);
  /** Adds a control color, returning 0 on success. Overwrites existing colors. */
  int setColor(float zeroToOne, const osg::Vec4& color);

  /** Removes a single control color, by its value. Returns 0 on success. */
  int removeColor(float zeroToOne);
  /**
   * Removes all configured colors using osg::TransferFunction1D::clear()
   * which sets two white stops at 0.f and 1.f.
   */
  void clearColors();

  /** Retrieves all color values, converted to QColors. */
  std::map<float, QColor> colors() const;
  /** Retrieves all color values. */
  std::map<float, osg::Vec4> getColorMap() const;

  /** Retrieves count of registered colors. */
  int colorCount() const;

  /**
   * Sets all control colors at once, replacing old values. Discards values outside [0,1].
   * New map should provide at least two valid stops. Returns 0 on success, non-zero on error.
   * If the given map is invalid, no changes are made to the gradient.
   */
  int setColors(const std::map<float, QColor>& colors);
  /**
   * Sets all control colors at once, replacing old values. Discards values outside [0,1].
   * New map should provide at least two valid stops. Returns 0 on success, non-zero on error.
   * If the given map is invalid, no changes are made to the gradient.
   */
  int setColors(const std::map<float, osg::Vec4>& colors);
#endif // OLD_SIMQT_COLORGRADIENT_API

#ifdef NEW_SIMQT_COLORGRADIENT_API
  /** Adds a control color. The percentage need not be unique. Index of color returned. (QColor version)*/
  size_t addControlColor(float zeroToOne, const QColor& color);
  /** Adds a control color. The percentage need not be unique. Index of color returned. (osg::Vec4 version)*/
  size_t addControlColor(float zeroToOne, const osg::Vec4& color);

  /** Sets a control color by index (QColor version). Returns 0 on success. */
  int setControlColor(size_t index, float zeroToOne, const QColor& color);
  /** Sets a control color by index (osg::Vec4 version). Returns 0 on success. */
  int setControlColor(size_t index, float zeroToOne, const osg::Vec4& color);

  /**
   * Removes the control color at the given index. Index 0 and 1 cannot be removed. Returns 0 on success.
   * This function will reorder control colors if given an index in the middle. In other words, control color
   * indices are not persistent through this call.
   */
  int removeControlColor(size_t index);
  /** Removes all control colors and resets to 0=white and 1=white */
  void clearControlColors();

  /** Retrieves a control color's color (transparent black if not found) - QColor version */
  QColor controlColor(size_t index) const;
  /** Retrieves a control color's color (transparent black if not found) - osg::Vec4 version */
  osg::Vec4 osgControlColor(size_t index) const;
  /** Retrieves the percentage (0-1) of a given control color index (-1 on invalid index) */
  float controlColorPct(size_t index) const;

  /** Retrieves total number of control colors */
  size_t numControlColors() const;

  /**
   * Replaces the content with the given color map. Additional control stops may be added
   * if the color map provided does not have stops at 0.0 and 1.0. Note that a std::map
   * of colors is incapable of representing the underlying data structure of the color
   * gradient because it cannot have multiple stops at the same percentage value, so relying
   * on this function can result in an incomplete color mapping.
   */
  void importColorMap(const std::map<float, QColor>& colors);

  /**
   * Replaces colors with those specified in the vector of colors. Unlike importColorMap(),
   * this function can be lossless because there may be multiple entries for a single stop pct.
   */
  void importColorVector(const std::vector<std::pair<float, QColor> >& colorVec);

  /**
   * Retrieves the effective color gradient. Use this to get an ordered list of all stops,
   * with duplicates removed. This is not a whole representation of the underlying data model,
   * because the underlying data might have duplicates and this representation (by definition)
   * will have no duplicate stops.
   */
  std::map<float, osg::Vec4> effectiveColorMap() const;


#endif // NEW_SIMQT_COLORGRADIENT_API

  /** Comparison operator */
  bool operator==(const ColorGradient& rhs) const;
  /** Inequality operator */
  bool operator!=(const ColorGradient& rhs) const;

private:
  osg::ref_ptr<osg::TransferFunction1D> function_;
  bool discrete_ = false;
};

}

Q_DECLARE_METATYPE(simQt::ColorGradient);

#endif /* SIMQT_COLORGRADIENT_H */
