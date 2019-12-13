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
#ifndef SIMUTIL_SHADERS_H
#define SIMUTIL_SHADERS_H

#include "osgEarth/ShaderLoader"

namespace simUtil {

/** Note: Not exported. */
class Shaders : public osgEarth::Util::ShaderPackage
{
public:
  Shaders();

  /** Name of combined shader for map scale flat coloring */
  std::string mapScale() const;
  /** Name of the vertex shader for terrain toggle effect */
  std::string terrainToggleEffectVertex() const;
  /** Name of the fragment shader for terrain toggle effect */
  std::string terrainToggleEffectFragment() const;
};

}

#endif /* SIMUTIL_SHADERS_H */
