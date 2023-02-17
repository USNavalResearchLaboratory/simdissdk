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
#ifndef SIMUTIL_GESTUREPROCESSOR_H
#define SIMUTIL_GESTUREPROCESSOR_H

#include <vector>
#include "osg/Vec2f"
#include "osgGA/GUIEventAdapter"
#include "simCore/Common/Common.h"

namespace simUtil {

/**
 * Represents parameters that are useful for interpreting two-finger pan movements from
 * touch events. Pan values are represented as delta X and Y values.
 */
struct PanData
{
  /** Amount of change in the X and Y coordinate in pixels for two-fingered pan */
  osg::Vec2f xy;
};

/**
 * Represents various parameters that are useful for interpreting pinch movements from
 * touch events. Pinch (and spread) is reported both in terms of pixels and a scale in
 * percentage. The center point is provided for users that want asymmetrical zoom.
 */
struct PinchData
{
  /**
   * Pixel delta (pinchNewDistance - pinchOldDistance); pixels changed between touches.
   * Positive values indicate fingers spreading, typically for zoom-in. Negative values
   * indicate fingers coming closer together, typically for zoom-out.
   */
  float pixels = 0.f;
  /**
   * Pinch scaling; pinch represented as a fraction of new distance over old distance.
   * Value of 1.0 indicates no pinch occurred. Values > 1.0 indicate fingers spreading,
   * typically for a zoom-in. Values from (0.0,1.0) indicate fingers moving closer
   * together, typically for a zoom-out.
   */
  float scale = 1.f;

  /** Distance between fingers for current touch */
  float newDistance = 0.f;
  /** Distance between fingers for previous touch */
  float oldDistance = 0.f;
  /** Main window X and Y coordinates for the midpoint of current pinch. */
  osg::Vec2f midPointXy;
};

/**
 * Represents parameters that are useful for interpreting two-finger twist movements from
 * touch events. Twist is represented as an angle between old and new finger vectors.
 */
struct TwistData
{
  /**
   * Angle change between last set of touch and new set of touch, in radians.
   * Positive values are used for clockwise touch rotations, and negative values
   * are used for counter-clockwise rotations.
   */
  float angleRad = 0.f;
};

/**
 * Represents an amalgamation of all recognized gesture data parameters. Gestures are
 * calculated between two sets of touch positions separated in time, and this data is
 * therefore a delta between those two successive sets of touch points.
 */
struct GestureData
{
  /** Set to true when pan, pinch, and twist contain valid values. */
  bool isValid = false;

  /** Two-fingered pan delta in X and Y coordinates */
  PanData pan;
  /** Pinch (or spread) data in pixels and percentages */
  PinchData pinch;
  /** Twist data in rotation angle */
  TwistData twist;
};

/**
 * Responsible for calculating gestures between successive touch events.
 * Instantiate this, then pass in GUI events from a osgGA::GUIEventHandler.
 * It will in turn provide gesture calculations between successive touch events.
 */
class SDKUTIL_EXPORT GestureProcessor
{
public:
  GestureProcessor();
  virtual ~GestureProcessor();

  /**
   * Processes touch events. Call this at least for all GUI events that involve touch
   * (i.e. passes ea.isMultiTouchEvent() test). It will filter out events it doesn't
   * care about. The method processes touch points and generates a "delta" structure
   * GestureData that reflects how fingers have moved between touches.
   * @param ea Event adapter representing current state.
   * @return Gesture delta from the last touch event. If .isValid is false, then there
   *   was no touch processing performed, otherwise the entries are valid. Gestures
   *   are cumulative in the sense that this is a simple delta from the last gesture
   *   state. Users are expected to apply this delta immediately, with deltas naturally
   *   accumulating to a finished state. All gesture data is calculated at once, and it
   *   is up to the user to filter out gestures as they see appropriate (e.g. ignoring
   *   twist data once a sufficient pan gesture begins).
   */
  GestureData processTouch(const osgGA::GUIEventAdapter& ea);

  /** Retrieve the gesture most recently returned from processTouch(). */
  GestureData mostRecentGesture() const;

private:
  typedef osgGA::GUIEventAdapter::TouchData::TouchPoint TouchPoint;

  /**
   * Calculate the gesture delta between two successive vectors of touch points.
   * @param p0 Current or most recent touch point.
   * @param p1 Historical touch point.
   * @return Gesture relationships between the two points.
   */
  GestureData processTouchVecs_(const std::vector<TouchPoint>& p0,
    const std::vector<TouchPoint>& p1) const;

  GestureData currentGesture_;
  std::vector<TouchPoint> lastTouch_;
  std::vector<TouchPoint> currentTouch_;
};

}

#endif /* SIMUTIL_GESTUREPROCESSOR_H */
