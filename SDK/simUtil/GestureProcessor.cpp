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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simUtil/GestureProcessor.h"

namespace simUtil {

GestureProcessor::GestureProcessor()
{
}

GestureProcessor::~GestureProcessor()
{
}

GestureData GestureProcessor::processTouch(const osgGA::GUIEventAdapter& ea)
{
  // Only process touch events
  if (!ea.isMultiTouchEvent())
    return currentGesture_;

  // Reset on begin of touch
  currentGesture_ = GestureData();
  if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
    currentTouch_.clear();
  lastTouch_ = currentTouch_;

  // Save the new touch points
  currentTouch_.clear();
  currentTouch_.insert(currentTouch_.end(), ea.getTouchData()->begin(), ea.getTouchData()->end());
  if (currentTouch_.empty())
    return currentGesture_;

  currentGesture_ = processTouchVecs_(currentTouch_, lastTouch_);
  return currentGesture_;
}

GestureData GestureProcessor::mostRecentGesture() const
{
  return currentGesture_;
}

GestureData GestureProcessor::processTouchVecs_(const std::vector<TouchPoint>& p0,
  const std::vector<TouchPoint>& p1) const
{
  GestureData rv;

  // Stop processing if fingers do not match
  if (p1.size() < 2 || p0.size() < 2 ||
    p1[0].id != p0[0].id ||
    p1[1].id != p0[1].id)
  {
    return rv;
  }
  // Stop processing if the fingers are on an end event (being lifted)
  if (p1[0].phase == osgGA::GUIEventAdapter::TOUCH_ENDED ||
    p1[1].phase == osgGA::GUIEventAdapter::TOUCH_ENDED ||
    p0[0].phase == osgGA::GUIEventAdapter::TOUCH_ENDED ||
    p0[1].phase == osgGA::GUIEventAdapter::TOUCH_ENDED)
  {
    return rv;
  }

  // Multiple touch types can be detected at once. Start by just gathering data.
  rv.isValid = true;

  // deltaX and deltaY are how far finger 0 and finger 1 moved in X and Y coordinates
  const osg::Vec2f deltaX = {
    p0[0].x - p1[0].x,
    p0[1].x - p1[1].x,
  };
  const osg::Vec2f deltaY = {
    p0[0].y - p1[0].y,
    p0[1].y - p1[1].y,
  };

  // Can calculate the pan difference now
  rv.pan.xy.x() = (deltaX[0] + deltaX[1]) * 0.5;
  rv.pan.xy.y() = (deltaY[0] + deltaY[1]) * 0.5;

  // Determine the direction line segment between the fingers
  const osg::Vec2f oldDirectionVecXy = {
    p1[1].x - p1[0].x,
    p1[1].y - p1[0].y,
  };
  const osg::Vec2f newDirectionVecXy = {
    p0[1].x - p0[0].x,
    p0[1].y - p0[0].y,
  };

  // Calculate the distance change between fingers 0 and 1
  rv.pinch.newDistance = newDirectionVecXy.length();
  rv.pinch.oldDistance = oldDirectionVecXy.length();
  const double deltaDistance = rv.pinch.newDistance - rv.pinch.oldDistance;
  rv.pinch.pixels = deltaDistance;
  rv.pinch.midPointXy.x() = 0.5 * (p0[0].x + p0[1].x);
  rv.pinch.midPointXy.y() = 0.5 * (p0[0].y + p0[1].y);
  if (rv.pinch.oldDistance == 0.)
    rv.pinch.scale = 1.;
  else
    rv.pinch.scale = rv.pinch.newDistance / rv.pinch.oldDistance;

  // Calculate the angle of rotation
  const double angle[2] = {
    atan2(p0[0].y - p0[1].y,
      p0[0].x - p0[1].x),
    atan2(p1[0].y - p1[1].y,
      p1[0].x - p1[1].x)
  };
  rv.twist.angleRad = angle[0] - angle[1];

  return rv;
}

}
