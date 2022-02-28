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
#ifndef SIMVIS_RFPROP_THRESHOLD_COLOR_PROVIDER_H
#define SIMVIS_RFPROP_THRESHOLD_COLOR_PROVIDER_H

#include "simVis/RFProp/ColorProvider.h"

namespace simRF
{

/**
 * A ColorProvider that provides colors compared to a threshold value.
 */
class SDKVIS_EXPORT ThresholdColorProvider : public ColorProvider
{
public:

  /**
   * Creates a new ThresholdColorProvider
   */
  ThresholdColorProvider();

  /**
   * Creates a new ThresholdColorProvider
   * @param belowColor The color that values below the threshold should be displayed
   * @param aboveColor The color that values above the threshold should be displayed
   * @param threshold The threshold to compare values against
   * @param mode The mode to return colors.
   */
  ThresholdColorProvider(const osg::Vec4f& belowColor, const osg::Vec4f& aboveColor, float threshold = 0.0f, ColorProvider::ColorMode mode = COLORMODE_ABOVE_AND_BELOW);

  /**
   * Gets the display mode
   */
  virtual ColorMode getMode() const;

  /**
   * Sets the display mode
   */
  void setMode(ColorMode mode);

  /**
   * Gets the color that values below the threshold should be displayed
   */
  const osg::Vec4f& getBelowColor() const;

  /**
   * Sets the color that values below the threshold should be displayed
   */
  void setBelowColor(const osg::Vec4f& belowColor);

  /**
   * Gets the color that values above the threshold should be displayed
   */
  const osg::Vec4f& getAboveColor() const;

  /**
   * Sets the color that values below the threshold should be displayed
   */
  void setAboveColor(const osg::Vec4f& aboveColor);

  /**
   * Gets the threshold to compare values against.
   */
  float getThreshold() const;

  /**
   * Sets the threshold to compare values against.
   */
  void setThreshold(float threshold);

  /**
   * Installs this color provider from the given state set
   */
  virtual void install(osg::StateSet* stateset);

  /**
   * Uninstall this color provider from the given state set
   */
  virtual void uninstall(osg::StateSet* stateset);

protected:

  /// Initialize the color provider
  void init_();

  /// osg::Referenced-derived
  virtual ~ThresholdColorProvider() {}

protected:
  osg::Vec4f belowColor_; ///< Color for values below threshold
  osg::Vec4f aboveColor_; ///< Color for values above threshold
  float threshold_;       ///< Threshold value
  ColorMode mode_;        ///< Threshold draw mode

  /** Shader uniform variable for the below color */
  osg::ref_ptr<osg::Uniform> belowColorUniform_;
  /** Shader uniform variable for the above color */
  osg::ref_ptr<osg::Uniform> aboveColorUniform_;
  /** Shader uniform variable for the threshold value */
  osg::ref_ptr<osg::Uniform> thresholdUniform_;
  /** Shader uniform variable for the color mode */
  osg::ref_ptr<osg::Uniform> modeUniform_;
};
}

#endif /* SIMVIS_RFPROP_THRESHOLD_COLOR_PROVIDER_H */
