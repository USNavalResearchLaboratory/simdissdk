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

#include "osg/ref_ptr"
#include "osg/NodeCallback"
#include "osg/Vec2d"
#include "simCore/Common/Common.h"

namespace osgGA { class GUIEventAdapter; }
namespace osgText { class Text; }

namespace simVis {

/**
 * Utilities for dealing with device pixel ratio (DPR) changes. A device pixel ratio of 1.0
 * is 100% scaling (e.g. 1.5 is 150% scaling).
 *
 * This class primarily manages texture resolution for text to ensure crisp rendering
 * on high-DPI displays. Layout scaling (positions and sizes) is handled natively by
 * View HUD orthographic projections using logical coordinates.
 */
class SDKVIS_EXPORT DevicePixelRatioUtils
{
public:
  /** Retrieves the current Device Pixel Ratio (DPR). 1.0 is standard, 2.0 is 4K/Retina. Never 0.0. */
  static double getDpr();

  /** Converts a physical pixel value (from GL/OSG) to a logical layout value. */
  static double toLogical(double physical);
  /** Converts a logical layout value to a physical pixel value. */
  static double toPhysical(double logical);

  /** Extracts the mouse coordinates from an OSG event and converts them to logical space. */
  static osg::Vec2d getLogicalMousePosition(const osgGA::GUIEventAdapter& ea);
};

/**
 * Monitors the device pixel ratio and dynamically updates the font size and resolution
 * when the application is dragged between screens with different DPI settings. This
 * is most useful in osgText::Text using SCREEN_COORDS coordinate system, that is in the
 * 3D scene. Text on SuperHUD does not need this, because they should not be using
 * SCREEN_COORDS and should already be logically scaled.
 */
class DpiTextScalingCallback : public osg::NodeCallback
{
public:
  explicit DpiTextScalingCallback(float logicalFontSize);

  /** Changes the character size */
  void setCharacterSize(float logicalFontSize);

  // From NodeCallback:
  virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) override;

  /** Helper function to install the node callback AND set the logical size at the same time */
  static osg::ref_ptr<DpiTextScalingCallback> install(osgText::Text& textNode, float logicalFontSize);

private:
  /** Internal function to change the physical size based on current DPR */
  void applyScaling_(osgText::Text& textNode) const;

  float logicalFontSize_ = 10.f;
};

}

#endif /* SIMVIS_DEVICEPIXELRATIOUTILS_H */
