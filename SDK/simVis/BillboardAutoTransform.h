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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#ifndef SIMVIS_BILLBOARD_AUTO_TRANSFORM
#define SIMVIS_BILLBOARD_AUTO_TRANSFORM

#include <osg/AutoTransform>
#include <osg/Node>
#include <osg/observer_ptr>
#include <osg/Viewport>
#include "simCore/Common/Common.h"

namespace simVis
{

/**
 * An AutoTransform variant for orienting billboard icons.  Adapted with permission from the
 * deprecated osgEarth::PixelAutoTransform.
 */
class SDKVIS_EXPORT BillboardAutoTransform : public osg::AutoTransform
{
public:
  BillboardAutoTransform();

  /**
  * Forces a recalculation of the autoscale on the next traversal
  * (this usually doesn't happen unless the camera moves)
  */
  void dirty();

  /** Set up the transform to orient the node based on a 2D screen-space rotation. */
  void setRotateInScreenSpace(bool value);
  /** Returns true if the transform is orienting the node based on a 2D screen-space rotation. */
  bool getRotateInScreenSpace() const;

  /** Sets the value of the 2D rotation in radians, used if rotate-in-screen-space is true. */
  void setScreenSpaceRotation(double radians);
  /** Retrieves the value of the 2D rotation in radians, if rotate-in-screen-space is true. */
  double getScreenSpaceRotation() const;

  /** Rotate the node to face the screen appropriately on cull traversal */
  virtual void accept(osg::NodeVisitor& nv);

private:
  bool   dirty_;
  bool   rotateInScreenSpace_;
  double screenSpaceRotationRadians_;
};
}

#endif
