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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_COLORGRADIENT_H
#define SIMQT_COLORGRADIENT_H

#include <map>
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
 * Represents a color gradient between magnitude values 0 and 1.
 * Wraps osg::TransferFunction1D as underlying implementation.
 * This class enforces a minimum of two color stops at all times.
 */
class SDKQT_EXPORT ColorGradient
{
public:
  /** Creates a default gradient. */
  ColorGradient();
  /** Creates a gradient with colors in given range. Values outside [0,1] are discarded. */
  explicit ColorGradient(const std::map<float, QColor>& colors);
  /** Creates a gradient with colors in given range. Values outside [0,1] are discarded. */
  explicit ColorGradient(const std::map<float, osg::Vec4>& colors);

  virtual ~ColorGradient();

  // Factory methods for making new built-in gradients
  static ColorGradient newDefaultGradient();
  static ColorGradient newDarkGradient();
  static ColorGradient newGreyscaleGradient();
  static ColorGradient newDopplerGradient();
  /** Interpolates a color between lowColor and highColor, using low and high as guideposts against val. */
  static QColor interpolate(const QColor& lowColor, const QColor& highColor, float low, float val, float high);

  /**
   * Retrieves the color mapping to the given value. Values range [0,1].
   * Colors outside the configured min/max values will be clamped.
   * If no colors have been set, then black is returned.
   */
  QColor colorAt(float zeroToOne) const;

  /**
   * Retrieves the color mapping to the given value. Values range [0,1].
   * Colors outside the configured min/max values will be clamped.
   * If no colors have been set, then black is returned.
   */
  osg::Vec4 osgColorAt(float zeroToOne) const;

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

  /** Comparison operator */
  bool operator==(const ColorGradient& rhs) const;
  /** Inequality operator */
  bool operator!=(const ColorGradient& rhs) const;

private:
  osg::ref_ptr<osg::TransferFunction1D> function_;
};

}

Q_DECLARE_METATYPE(simQt::ColorGradient);

#endif /* SIMQT_COLORGRADIENT_H */
