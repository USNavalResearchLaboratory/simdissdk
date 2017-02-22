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
#include "simVis/LightAmbient.h"

namespace simVis {

LightAmbient::LightAmbient()
  : StateAttribute(),
    ambient_(0.5f, 0.5f, 0.5f, 1.f),
    lightNum_(0),
    useLightMaster_(false)
{
}

LightAmbient::LightAmbient(const simVis::Color& ambient, unsigned int lightNum)
  : StateAttribute(),
    ambient_(ambient),
    lightNum_(lightNum),
    useLightMaster_(false)
{
}

LightAmbient::LightAmbient(osg::Light* lightMaster)
  : StateAttribute(),
    lightNum_(0),
    lightMaster_(lightMaster),
    useLightMaster_(true)
{
}

LightAmbient::LightAmbient(const LightAmbient& rhs, const osg::CopyOp& copyOp)
  : StateAttribute(rhs, copyOp),
    ambient_(rhs.ambient_),
    lightNum_(rhs.lightNum_),
    lightMaster_(rhs.lightMaster_),
    useLightMaster_(rhs.useLightMaster_)
{
}

LightAmbient::~LightAmbient()
{
}

unsigned int LightAmbient::getMember() const
{
  return lightNum_;
}

void LightAmbient::setAmbient(const simVis::Color& color)
{
  ambient_ = color;
  useLightMaster_ = false;
}

void LightAmbient::setAmbient(float magnitude)
{
  setAmbient(simVis::Color(magnitude, magnitude, magnitude, 1.f));
}

const simVis::Color& LightAmbient::ambient() const
{
  return ambient_;
}

void LightAmbient::setLightNum(unsigned int lightNumber)
{
  lightNum_ = lightNumber;
  useLightMaster_ = false;
}

unsigned int LightAmbient::lightNum() const
{
  return lightNum_;
}

void LightAmbient::setLightMaster(osg::Light* lightMaster)
{
  lightMaster_ = lightMaster;
  useLightMaster_ = true;
}

osg::Light* LightAmbient::lightMaster() const
{
  return lightMaster_.get();
}

bool LightAmbient::useLightMaster() const
{
  return useLightMaster_;
}

int LightAmbient::compare(const StateAttribute& sa) const
{
  // check the types are equal and then create the rhs variable
  // used by the COMPARE_StateAttribute_Parameter macros below.
  COMPARE_StateAttribute_Types(LightAmbient, sa);

  // compare each parameter in turn against the rhs.
  COMPARE_StateAttribute_Parameter(ambient_);
  COMPARE_StateAttribute_Parameter(lightNum_);
  COMPARE_StateAttribute_Parameter(lightMaster_);
  COMPARE_StateAttribute_Parameter(useLightMaster_);

  return 0; // passed all the above comparison macros, must be equal.
}

void LightAmbient::apply(osg::State& state) const
{
#ifdef SIMVIS_GL_FIXED_FUNCTION_AVAILABLE
  if (useLightMaster_)
  {
    // If non-NULL, use the master's values.  Else do nothing.
    if (lightMaster_.valid())
      glLightfv(static_cast<GLenum>(GL_LIGHT0 + lightMaster_->getLightNum()), GL_AMBIENT, lightMaster_->getAmbient().ptr());
  }
  else
    glLightfv(static_cast<GLenum>(GL_LIGHT0 + lightNum_), GL_AMBIENT, ambient_.ptr());
#else
  SIM_NOTICE << "Warning: LightAmbient::apply(State&) - not supported.\n";
#endif
}

}

#endif /* USE_DEPRECATED_SIMDISSDK_API */
