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
#ifndef SIMVIS_RFPROP_COMPOSITE_COLOR_PROVIDER_H
#define SIMVIS_RFPROP_COMPOSITE_COLOR_PROVIDER_H

#include <osg/ref_ptr>
#include <osg/observer_ptr>
#include "simVis/RFProp/GradientColorProvider.h"
#include "simVis/RFProp/ThresholdColorProvider.h"
#include "simVis/RFProp/ColorProvider.h"

namespace simRF
{

/**
 * A ColorProvider that manages both threshold and gradient type color providers
 */
class SDKVIS_EXPORT CompositeColorProvider : public ColorProvider
{
public:

  /**
   * Creates a new CompositeColorProvider
   */
  CompositeColorProvider();

  /// Threshold methods

  /**
   * Gets the display color mode
   */
  virtual ColorMode getMode() const;

  /**
   * Sets the display color mode
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

  /// Gradient methods

  /** Sets the color of the given value in the gradient */
  void setGradientColor(float value, const osg::Vec4f& color);

  /** Sets all the colors for the gradient. Clears out any current colors in the map and replaces with the color map specified */
  void setGradientColorMap(const simRF::GradientColorProvider::ColorMap& colors);

  /** Gets whether to return discrete values along the gradient */
  bool getGradientDiscrete() const;

  /**
   * Sets whether to return discrete values along the gradient.
   * @param discrete If true, discrete values in the gradient will be returned.  If false, colors will be interpolated along the gradient
   */
  void setGradientDiscrete(bool discrete);

  /** Clears all colors in the gradient */
  void clearGradient();

  /**
   * Controls the visibility of the color by adjusting alpha component
   * Range of values 0 (opaque) to 100 (transparent)
   * @param transparency Transparency percentage value for drawn propagation data
   */
  void setTransparency(int transparency);

  /**
   * Returns the transparency
   * @return Transparency between 0 and 100
   */
  int transparency() const;

  /**
   * Installs this color provider from the given state set
   */
  virtual void install(osg::StateSet* stateset);

  /**
   * Uninstall this color provider from the given state set
   */
  virtual void uninstall(osg::StateSet* stateset);

protected:

  /// osg::Referenced-derived
  virtual ~CompositeColorProvider() {}

private:
  /// update the gradient color provider's color map
  void updateGradientColorMap_();

  /// current color mode
  ColorMode colorMode_;
  /// current transparency, 100 is fully transparent, 0 is opaque
  int transparency_;
  /// color provider that will be used for gradient-based displays
  osg::ref_ptr<GradientColorProvider> gradientProvider_;
  /// color provider that will be used for threshold-based displays
  osg::ref_ptr<ThresholdColorProvider> thresholdProvider_;
  /// last defined stateset for the color providers
  osg::observer_ptr<osg::StateSet> lastStateSet_;
  /// local map of gradient colors for initializing gradient color provider
  simRF::GradientColorProvider::ColorMap gradientColors_;

};
}

#endif /* SIMVIS_RFPROP_THRESHOLD_COLOR_PROVIDER_H */
