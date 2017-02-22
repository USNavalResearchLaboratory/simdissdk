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
#include "osg/StateSet"
#include "osgEarth/Registry"
#include "osgEarth/Capabilities"
#include "osgEarth/VirtualProgram"
#include "simCore/Time/Utils.h"
#include "simVis/Shaders.h"
#include "simVis/GlowHighlight.h"

namespace simVis
{

GlowHighlight::GlowHighlight(osg::Node* onNode)
  : node_(onNode),
    enabled_(false),
    supported_(false),
    shaderCreated_(false)
{
  supported_ = osgEarth::Registry::capabilities().supportsGLSL(110u);
  if (supported_ && node_.valid())
  {
    // Save the stateset and define the callback
    stateSet_ = node_->getOrCreateStateSet();
  }

  // Sets up the uniform in addition to saving the default color
  setColor(simVis::Color::White);
  // Set the uniform as needed
  setEnabled(false);
}

GlowHighlight::~GlowHighlight()
{
  if (supported_)
  {
    // Clean up the GLSL program if it was created
    if (shaderCreated_)
    {
      osg::ref_ptr<osg::StateSet> stateSet;
      if (stateSet_.lock(stateSet))
      {
        // Clear out the shader
        osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::get(stateSet.get());
        if (vp)
        {
          simVis::Shaders package;
          package.unload(vp, package.glowHighlightFragment());
        }
        // Remove the variables
        stateSet->removeUniform("simvis_glowhighlight_color");
        stateSet->removeUniform("simvis_glowhighlight_enabled");
      }
    }
  }
}

void GlowHighlight::createShader_()
{
  // No need to recreate if already done
  if (shaderCreated_)
    return;

  // Create the program
  osg::ref_ptr<osg::StateSet> stateset;
  if (stateSet_.lock(stateset))
  {
    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
    simVis::Shaders package;
    package.load(vp, package.glowHighlightFragment());

    // Set the initial color
    stateset
      ->getOrCreateUniform("simvis_glowhighlight_color", osg::Uniform::FLOAT_VEC4)
      ->set(color_);
    // Set the initial "enabled" flag
    stateset
      ->getOrCreateUniform("simvis_glowhighlight_enabled", osg::Uniform::BOOL)
      ->set(enabled_);
  }
  shaderCreated_ = true;
}

void GlowHighlight::setColor(const simVis::Color& color)
{
  // Save the color, so that even if shader isn't created, we can set the right color on initialization
  color_ = color;

  // Update the uniform variable if the shader was created and is valid
  if (supported_ && shaderCreated_)
  {
    osg::ref_ptr<osg::StateSet> stateSet;
    if (stateSet_.lock(stateSet))
    {
      stateSet
        ->getOrCreateUniform("simvis_glowhighlight_color", osg::Uniform::FLOAT_VEC4)
        ->set(color);
    }
  }
}

simVis::Color GlowHighlight::color() const
{
  return color_;
}

void GlowHighlight::setEnabled(bool enable)
{
  // Add or remove the Update callback conditionally on the enabled_ flag
  if (node_.valid() && enable != enabled_)
  {
    if (enable)
    {
      // Create the shader lazily
      if (!shaderCreated_)
        createShader_();
      // Failure indicates that there were problems creating the shader
      assert(shaderCreated_);
    }

    // Save the flag
    enabled_ = enable;
  }

  // Update the uniform variable, unless the shader hasn't yet been created
  if (supported_ && shaderCreated_)
  {
    osg::ref_ptr<osg::StateSet> stateSet;
    if (stateSet_.lock(stateSet))
    {
      stateSet
        ->getOrCreateUniform("simvis_glowhighlight_enabled", osg::Uniform::BOOL)
        ->set(enable);
    }
  }
}

bool GlowHighlight::enabled() const
{
  return enabled_;
}

}
