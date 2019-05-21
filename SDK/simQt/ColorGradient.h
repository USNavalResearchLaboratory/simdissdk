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
#ifndef SIMQT_COLORGRADIENT_H
#define SIMQT_COLORGRADIENT_H

#include <map>
#include <QColor>
#include <QMetaType>
#include "simCore/Common/Export.h"

namespace simQt {

/** Represents a color gradient between magnitude values 0 and 1. */
class SDKQT_EXPORT ColorGradient
{
public:
  /** Creates a default gradient. */
  ColorGradient();
  /** Creates a gradient with colors in given range.  Values outside [0,1] are discarded. */
  explicit ColorGradient(const std::map<double, QColor>& colors);
  virtual ~ColorGradient();

  // Factory methods for making new built-in gradients
  static ColorGradient newDefaultGradient();
  static ColorGradient newDarkGradient();
  static ColorGradient newGreyscaleGradient();
  static ColorGradient newDopplerGradient();
  /** Interpolates a color between lowColor and highColor, using low and high as guideposts against val. */
  static QColor interpolate(const QColor& lowColor, const QColor& highColor, double low, double val, double high);

  /**
   * Retrieves the color mapping to the given value.  Values range [0,1].  If there are
   * no colors, then black is returned.  Otherwise colors are interpolated.  Colors
   * requested outside the configured range are clamped.
   */
  QColor colorAt(double zeroToOne) const;

  /** Adds a control color, returning 0 on success.  Overwrites existing colors. */
  int setColor(double zeroToOne, const QColor& color);
  /** Removes a single control color, by its value. Returns 0 on success. */
  int removeColor(double zeroToOne);
  /** Removes all control colors. */
  void clearColors();

  /** Retrieves all color values */
  std::map<double, QColor> colors() const;
  /** Sets all control colors at once, replacing old values.  Values must be [0,1]. */
  void setColors(const std::map<double, QColor>& colors);

  /** Comparison operator */
  bool operator==(const ColorGradient& rhs) const;
  /** Inequality operator */
  bool operator!=(const ColorGradient& rhs) const;

private:

  std::map<double, QColor> colors_;
};

}

Q_DECLARE_METATYPE(simQt::ColorGradient);

#endif /* SIMQT_COLORGRADIENT_H */
