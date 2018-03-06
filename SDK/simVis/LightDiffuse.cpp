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
#ifdef USE_DEPRECATED_SIMDISSDK_API

#include "osg/Light"
#include "simNotify/Notify.h"
#include "simVis/LightDiffuse.h"

namespace simVis {

LightDiffuse::LightDiffuse()
  : StateAttribute(),
    diffuse_(0.5f, 0.5f, 0.5f, 1.f),
    lightNum_(0),
    useLightMaster_(false)
{
}

LightDiffuse::LightDiffuse(const simVis::Color& diffuse, unsigned int lightNum)
  : StateAttribute(),
    diffuse_(diffuse),
    lightNum_(lightNum),
    useLightMaster_(false)
{
}

LightDiffuse::LightDiffuse(osg::Light* lightMaster)
  : StateAttribute(),
    lightNum_(0),
    lightMaster_(lightMaster),
    useLightMaster_(true)
{
}

LightDiffuse::LightDiffuse(const LightDiffuse& rhs, const osg::CopyOp& copyOp)
  : StateAttribute(rhs, copyOp),
    diffuse_(rhs.diffuse_),
    lightNum_(rhs.lightNum_),
    lightMaster_(rhs.lightMaster_),
    useLightMaster_(rhs.useLightMaster_)
{
}

LightDiffuse::~LightDiffuse()
{
}

unsigned int LightDiffuse::getMember() const
{
  return lightNum_;
}

void LightDiffuse::setDiffuse(const simVis::Color& color)
{
  diffuse_ = color;
  useLightMaster_ = false;
}

void LightDiffuse::setDiffuse(float magnitude)
{
  setDiffuse(simVis::Color(magnitude, magnitude, magnitude, 1.f));
}

const simVis::Color& LightDiffuse::diffuse() const
{
  return diffuse_;
}

void LightDiffuse::setLightNum(unsigned int lightNumber)
{
  lightNum_ = lightNumber;
  useLightMaster_ = false;
}

unsigned int LightDiffuse::lightNum() const
{
  return lightNum_;
}

void LightDiffuse::setLightMaster(osg::Light* lightMaster)
{
  lightMaster_ = lightMaster;
  useLightMaster_ = true;
}

osg::Light* LightDiffuse::lightMaster() const
{
  return lightMaster_.get();
}

bool LightDiffuse::useLightMaster() const
{
  return useLightMaster_;
}

int LightDiffuse::compare(const StateAttribute& sa) const
{
  // check the types are equal and then create the rhs variable
  // used by the COMPARE_StateAttribute_Parameter macros below.
  COMPARE_StateAttribute_Types(LightDiffuse, sa);

  // compare each parameter in turn against the rhs.
  COMPARE_StateAttribute_Parameter(diffuse_);
  COMPARE_StateAttribute_Parameter(lightNum_);
  COMPARE_StateAttribute_Parameter(lightMaster_);
  COMPARE_StateAttribute_Parameter(useLightMaster_);

  return 0; // passed all the above comparison macros, must be equal.
}

void LightDiffuse::apply(osg::State& state) const
{
#if defined(SIMVIS_GL_FIXED_FUNCTION_AVAILABLE) && defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
  if (useLightMaster_)
  {
    // If non-NULL, use the master's values.  Else do nothing.
    if (lightMaster_.valid())
      glLightfv(static_cast<GLenum>(GL_LIGHT0 + lightMaster_->getLightNum()), GL_DIFFUSE, lightMaster_->getDiffuse().ptr());
  }
  else
    glLightfv(static_cast<GLenum>(GL_LIGHT0 + lightNum_), GL_DIFFUSE, diffuse_.ptr());
#else
  SIM_NOTICE << "Warning: LightDiffuse::apply(State&) - not supported.\n";
#endif
}

}

#endif /* USE_DEPRECATED_SIMDISSDK_API */
