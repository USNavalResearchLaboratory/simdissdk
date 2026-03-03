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
#ifndef SIMVIS_DEVICEPIXELRATIOUTILS_H
#define SIMVIS_DEVICEPIXELRATIOUTILS_H

#include "simCore/Common/Common.h"

namespace osg {
  class Node;
  class Vec3f;
}
namespace osgText { class TextBase; }

namespace simVis {

/**
 * Utilities for dealing with device pixel ratio (DPR) changes. A device pixel ratio of 1.0
 * is 100% scaling (e.g. 1.5 is 150% scaling). SIMDIS SDK applies scaling based on a 100%
 * display, and will up-scale as needed. For example, a line of width 4 on a 100% display
 * should have an actual line width of 6 on a 150% display. This class accomplishes that by
 * detecting an intended width of 4, and multiplying that by the DPR to get a stored width
 * of 6.
 *
 * This class relies on User Data Values (osg::Node::getUserValue) to get and store values.
 * It assumes any value supplied by an end user is at 100% scaling and will upscale as needed
 * based on the current DPR.
 *
 * In the future, DPR might apply directly in shaders, eliminating the need for some or all
 * of these functions.
 */
class SDKVIS_EXPORT DevicePixelRatioUtils
{
public:
  /** Marks a node as device-pixel-ratio disabled; no scaling is performed. */
  static void setDprDisabled(osg::Node& node);
  /** Returns true if the DPR calculations are disabled for this node. */
  static bool isDprDisabled(osg::Node& node);

  // Clean wrappers for commonly scaled values; accepts values at 100% scaling.
  static void setTextCharacterSize(osgText::TextBase& text, float characterSize);
  static void setTextPosition(osgText::TextBase& text, const osg::Vec3f& position);

  /** Extracts the current DPR from osgEarth's registry, and applies it recursively to the node tree and all children */
  static void updateScenePixelRatio(osg::Node& rootNode);
};

}

#endif /* SIMVIS_DEVICEPIXELRATIOUTILS_H */
