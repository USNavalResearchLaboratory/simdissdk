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
#include "osgEarth/VirtualProgram"
#include "osgEarth/ShaderLoader"
#include "simNotify/Notify.h"
#include "simVis/Shaders.h"
#include "simVis/BathymetryGenerator.h"

namespace simVis {

#define LC "simVis::BathymetryGenerator "

static const char* SEA_LEVEL_UNIFORM = "simVis_BathymetryGenerator_seaLevel";
static const char* OFFSET_UNIFORM = "simVis_BathymetryGenerator_offset";

BathymetryGenerator::BathymetryGenerator()
{
  seaLevelUniform_ = new osg::Uniform(SEA_LEVEL_UNIFORM, 0.1f);
  offsetUniform_ = new osg::Uniform(OFFSET_UNIFORM, -75.0f);
}

void BathymetryGenerator::onInstall(osgEarth::TerrainEngineNode* engine)
{
  if (engine)
  {
    osg::StateSet* stateSet = engine->getOrCreateStateSet();
    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateSet);

    // bring in our local shaders:
    simVis::Shaders shaders;
    shaders.load(vp, shaders.bathymetryGeneratorVertex());

    stateSet->addUniform(seaLevelUniform_.get());
    stateSet->addUniform(offsetUniform_.get());
  }
}

void BathymetryGenerator::onUninstall(osgEarth::TerrainEngineNode* engine)
{
  if (engine)
  {
    osg::StateSet* stateSet = engine->getStateSet();
    if (stateSet)
    {
      osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::get(stateSet);

      // remove shader functions:
      simVis::Shaders shaders;
      shaders.unload(vp, shaders.bathymetryGeneratorVertex());

      // no need to uninstall the terrain SDK; it is harmless

      // remove uniforms.
      stateSet->removeUniform(seaLevelUniform_.get());
      stateSet->removeUniform(offsetUniform_.get());
    }
  }
}

void BathymetryGenerator::setSeaLevelElevation(float value)
{
  seaLevelUniform_->set(value);
}

float BathymetryGenerator::getSeaLevelElevation() const
{
  float value;
  seaLevelUniform_->get(value);
  return value;
}

void BathymetryGenerator::setOffset(float value)
{
  offsetUniform_->set(value);
}

float BathymetryGenerator::getOffset() const
{
  float value;
  offsetUniform_->get(value);
  return value;
}

}
