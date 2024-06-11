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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMUTIL_TERRAINTOGGLEEFFECT_H
#define SIMUTIL_TERRAINTOGGLEEFFECT_H

#include "osgEarth/TerrainEffect"
#include "simCore/Common/Common.h"

namespace simUtil {

/**
 * Terrain effect that lets you quickly, globally toggle the display of
 * image and elevation layers on and off.  This can be useful for things
 * like quickly swapping between a colorful map display and a flat one,
 * like in the SIMDIS feature for toggling terrain on and off.
 */
class SDKUTIL_EXPORT TerrainToggleEffect : public osgEarth::TerrainEffect
{
public:
  TerrainToggleEffect();

  /** Enables the effect, thereby overwriting image and flattening elevation data */
  void setEnabled(bool enabled);
  /** Returns whether this effect is enabled. */
  bool isEnabled() const;

  // Override TerrainEffect methods
  virtual void onInstall(osgEarth::TerrainEngineNode* engine);
  virtual void onUninstall(osgEarth::TerrainEngineNode* engine);

protected:
  /** osg::Referenced-derived */
  virtual ~TerrainToggleEffect();

private:
  osg::ref_ptr<osg::Uniform> enabled_;
};

}

#endif /* SIMUTIL_TERRAINTOGGLEEFFECT_H */
