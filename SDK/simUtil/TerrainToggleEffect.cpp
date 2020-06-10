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
#include "osg/Uniform"
#include "osgEarth/TerrainEngineNode"
#include "osgEarth/VirtualProgram"
#include "simUtil/Shaders.h"
#include "simUtil/TerrainToggleEffect.h"

namespace simUtil {

TerrainToggleEffect::TerrainToggleEffect()
  : enabled_(new osg::Uniform("simutil_terraintoggle_enabled", true))
{
}

TerrainToggleEffect::~TerrainToggleEffect()
{
}

void TerrainToggleEffect::setEnabled(bool enabled)
{
  enabled_->set(enabled);
}

bool TerrainToggleEffect::isEnabled() const
{
  bool value = false;
  enabled_->get(value);
  return value;
}

void TerrainToggleEffect::onInstall(osgEarth::TerrainEngineNode* engine)
{
  if (!engine)
    return;
  // Turn on the shaders and add the uniform
  osg::StateSet* stateset = engine->getOrCreateStateSet();
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
  simUtil::Shaders package;
  package.load(vp, package.terrainToggleEffectVertex());
  package.load(vp, package.terrainToggleEffectFragment());
  stateset->addUniform(enabled_);
}

void TerrainToggleEffect::onUninstall(osgEarth::TerrainEngineNode* engine)
{
  if (!engine)
    return;
  // Turn on the shaders and add the uniform
  osg::StateSet* stateset = engine->getOrCreateStateSet();
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
  simUtil::Shaders package;
  package.unload(vp, package.terrainToggleEffectVertex());
  package.unload(vp, package.terrainToggleEffectFragment());
  stateset->removeUniform(enabled_);
}

}
