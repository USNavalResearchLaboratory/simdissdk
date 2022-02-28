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
#ifndef SIMVIS_CHROMAKEYCOLORFILTER_H
#define SIMVIS_CHROMAKEYCOLORFILTER_H

#include "osg/Uniform"
#include "osgEarth/ColorFilter"
#include "simCore/Common/Common.h"

namespace simVis
{

/**
 * Color filter that makes a color transparent.  Adapted with permission
 * from osgEarth 2.x prior to removal from osgEarth::Util.
 */
class SDKVIS_EXPORT ChromaKeyColorFilter : public osgEarth::ColorFilter
{
public:
  ChromaKeyColorFilter();
  /** Initializes from a Config object */
  explicit ChromaKeyColorFilter(const osgEarth::Config& conf);

  /**
   * The color to make transparent, each component is [0..1]
   * @param color Color value for the chroma key
   */
  void setColor(const osg::Vec3f& color);
  /** Retrieves the color value used */
  osg::Vec3f getColor() const;

  /**
   * The linear distance to search for "similar" colors to make transparent.
   * Currently this is doing a simple 3D distance comparison to find similar colors.
   */
  void setDistance(float distance);
  /** Retrieves linear distance */
  float getDistance() const;

  // ColorFilter methods

  /** Name of function to call in custom shader */
  virtual std::string getEntryPointFunctionName() const;
  /** Installs uniforms and bindings required on the provided stateset */
  virtual void install(osg::StateSet* stateSet) const;
  /** Serializes into a Config */
  virtual osgEarth::Config getConfig() const;

protected:
  virtual ~ChromaKeyColorFilter() {} // osg::Referenced object

private:
  void init_();

  unsigned int instanceId_;
  osg::ref_ptr<osg::Uniform> colorUniform_;
  osg::ref_ptr<osg::Uniform> distanceUniform_;
};

}

#endif /* SIMVIS_CHROMAKEYCOLORFILTER_H */
