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
#pragma once

#include "osg/Geometry"
#include "osgEarth/Draggers"
#include "simCore/Common/Export.h"

namespace osg { class Image; }

namespace simUtil {

/**
 * A dragger that uses a 2D image icon, billboards to the screen, and tints
 * its color when hovered. Similar to osgEarth::SphereDragger but with 2D icons.
 */
class SDKUTIL_EXPORT IconDragger : public osgEarth::Dragger
{
public:
  IconDragger(osgEarth::MapNode* mapNode, osg::Image* image);
  virtual ~IconDragger() { }

  /** Change the underlying image */
  void setImage(osg::Image* image);

  /** Color override when not picked */
  const osg::Vec4f& getColor() const;
  void setColor(const osg::Vec4f& color);

  /** Color override when mouse is hovering or picked through clicking */
  const osg::Vec4f& getPickColor() const;
  void setPickColor(const osg::Vec4f& pickColor);

  /** On-screen size (square) in pixels */
  float getSize() const;
  void setSize(float size);

  // From Dragger:
  virtual void enter() override;
  virtual void leave() override;

private:
  void updateColor_();

  osg::ref_ptr<osg::Geometry> geometry_;
  osg::ref_ptr<osg::Texture2D> texture_;

  osg::Vec4f pickColor_ = osg::Vec4f(1.f, 1.f, 0.f, 1.f); // yellow
  osg::Vec4f color_ = osg::Vec4f(1.f, 1.f, 1.f, 1.f); // white
  float size_ = 25.f;
};
}
