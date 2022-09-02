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

namespace simQt {

/** String template to format a QLinearGradient background like our UTILS::ColorGradient */
static const QString GRADIENT_STR_TEMPLATE = "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, %1);";
/** Format for each stop.  %1 is the percentage (0-1), %2 is a RGBA color string. */
static const QString GRADIENT_STOP_TEMPLATE = "stop: %1 rgba(%2)";

/**
 * Represents a color gradient between magnitude values 0 and 1. The gradient is defined by a
 * series of indexed control colors, or color stops. There are always at least 2 color stops.
 * The first guaranteed control color is index 0, at 0%. The second guaranteed control color is
 * index 1, at 100% (1.f). Additional control colors may be added in any order.
 *
 * The gradient supports inspection of effective colors using colorAt(). The entire effective
 * gradient in std::map format can also be inspected. Individual control colors can be set,
 * added, and removed. The 0th index is always present, and always at 0.f percent. The 1st
 * index is also always present, and always at 1.f percent. Though the color values can
 * be modified, these indexed values cannot be removed, as they represent the end stops.
 *
 * Multiple control colors can refer to the same stop percentage. In this case, the latest
 * defined control color takes precedence. In other words, although the 0.f and 1.f values at
 * index 0 and 1 cannot move, they can be both changed, and overridden with other control color
 * values. This organization will allow for control colors to shift within spectrum, or be
 * compressed to one edge or another, without a loss in fidelity.
 */
class SDKQT_EXPORT ColorGradient
{
public:
  /** Creates a default gradient. */
  ColorGradient();
  /** Copy constructor required for dynamic memory */
  ColorGradient(const ColorGradient& rhs);

  /** Assignment operator, required for dynamic memory */
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
   * Retrieves the effective color for the given percentage. Values range [0,1]. Colors outside
   * the configured min/max values will be clamped. If discrete flag is set to true (via
   * setDiscrete()), no interpolation is performed.
   */
  QColor colorAt(float zeroToOne) const;

  /**
   * Retrieves the effective color for the given percentage. Values range [0,1]. Colors outside
   * the configured min/max values will be clamped. If discrete flag is set to true (via
   * setDiscrete()), no interpolation is performed.
   */
  osg::Vec4 osgColorAt(float zeroToOne) const;

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

  /**
   * Retrieves total number of control colors. This is always equal or greater to the number of defined
   * colors in the effective gradient because of the overlap feature.
   */
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

  /**
   * Compresses the gradient, creating a new gradient. The user specifies a lower percentage
   * value [0,1] and a higher percentage value [0,1]. All control points (except the 0th and
   * 1st index) are compressed to fit within the new scaling. This is useful e.g. for a gradient
   * widget control that allows the user to adjust endpoints while maintaining the relative
   * ratio of colors.
   *
   * For example, a gradient with 2 additional control points at 0.25f and 0.5f:
   * - compress(0.f, 1.f) results in no changes.
   * - compress(0.5f, 1.f) results in a gradient with a control point at 0.625f and 0.75f.
   * - compress(0.25f, 0.75f) results in a gradient with a control point at 0.375f and 0.5f.
   *
   * @param lowPercent Low boundary for the gradient compression, between 0.f and 1.f inclusive.
   *   This value should be less than highPercent.
   * @param highPercent High boundary for the gradient compression, between 0.f and 1.f inclusive.
   *   This value should be greater than lowPercent.
   * @return New gradient that contains the compressed color scale.
   */
  ColorGradient compress(float lowPercent, float highPercent) const;

  /** Comparison operator */
  bool operator==(const ColorGradient& rhs) const;
  /** Inequality operator */
  bool operator!=(const ColorGradient& rhs) const;

private:
  /** Copies the control colors into the transfer function, updating the effective gradient */
  void updateTransferFunc_();

  osg::ref_ptr<osg::TransferFunction1D> function_;
  bool discrete_ = false;

  /** Vector of control colors. Guaranteed to have a minimum of 2 entries, 0th at 0%, 1st at 100% */
  std::vector<std::pair<float, osg::Vec4> > controlColors_;
};

}

Q_DECLARE_METATYPE(simQt::ColorGradient);

#endif /* SIMQT_COLORGRADIENT_H */
