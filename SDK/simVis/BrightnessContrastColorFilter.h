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
#ifndef SIMVIS_BRIGHTNESSCONTRASTCOLORFILTER_H
#define SIMVIS_BRIGHTNESSCONTRASTCOLORFILTER_H

#include "osg/Uniform"
#include "osgEarth/ColorFilter"
#include "simCore/Common/Common.h"

namespace simVis {

/**
 * Color filter that adjust the brightness/contrast of a texel.  Adapted with permission
 * from osgEarth 2.x prior to removal from osgEarth::Util.
 */
class SDKVIS_EXPORT BrightnessContrastColorFilter : public osgEarth::ColorFilter
{
public:
  BrightnessContrastColorFilter();
  /** Initializes from a Config object */
  explicit BrightnessContrastColorFilter(const osgEarth::Config& conf);

  /**
  * The brightness and contrast as percentages of the incoming pixel value.
  * (For example, brightness => 1.2 to increase brightness by 20%.)
  *
  * @param bc Brightness in X, Contrast in Y.  Range is [0..inf], results are clamped to [0..1].
  */
  void setBrightnessContrast(const osg::Vec2f& bc);
  /** Retrieves the brightness (x) and contrast (y) values */
  osg::Vec2f getBrightnessContrast(void) const;

  // ColorFilter methods

  /** Name of function to call in custom shader */
  virtual std::string getEntryPointFunctionName() const;
  /** Installs uniforms and bindings required on the provided stateset */
  virtual void install(osg::StateSet* stateSet) const;
  /** Serializes into a Config */
  virtual osgEarth::Config getConfig() const;

protected:
  virtual ~BrightnessContrastColorFilter() {} // osg::Referenced object

private:
  void init_();

  unsigned int instanceId_;
  osg::ref_ptr<osg::Uniform> uniform_;
};

}

#endif /* SIMVIS_BRIGHTNESSCONTRASTCOLORFILTER_H */
