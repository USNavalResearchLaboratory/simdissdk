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

#ifndef SIMVIS_AREA_HIGHLIGHT_H
#define SIMVIS_AREA_HIGHLIGHT_H

#include "osg/ref_ptr"
#include "osg/Group"
#include "osg/Vec4f"
#include "simCore/Common/Common.h"
#include "simData/DataStore.h"

namespace osg {
  class AutoTransform;
  class Uniform;
}
namespace osgEarth {
  class LineDrawable;
}

namespace simVis
{

/// Interface for a node that can set radius and color, used for highlighting
class SDKVIS_EXPORT HighlightNode : public osg::Group
{
public:
  /** Constructor */
  HighlightNode();
  /** OSG Copy constructor */
  HighlightNode(const HighlightNode& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Apply a color to the highlight */
  virtual void setColor(const osg::Vec4f& rgba) = 0;
  /** Changes the radius of the highlight in meters */
  virtual void setRadius(float radius) = 0;
  /** Set the shape to rotate to screen */
  virtual void setAutoRotate(bool autoRotate) = 0;

  // From osg::Node:
  virtual const char* libraryName() const { return "simVis"; }
  virtual const char* className() const { return "HighlightNode"; }

protected:
  /// osg::Referenced-derived
  virtual ~HighlightNode();
};

/// Attachment node for a circular highlight display.
class SDKVIS_EXPORT AreaHighlightNode : public HighlightNode
{
public:
  /** Declare boilerplate code */
  META_Node(simVis, AreaHighlightNode);
  AreaHighlightNode();
  AreaHighlightNode(const AreaHighlightNode& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  // From HighlightNode:
  virtual void setColor(const osg::Vec4f& rgba);
  virtual void setRadius(float radius);
  virtual void setAutoRotate(bool autoRotate) {}

protected:
  /// osg::Referenced-derived
  virtual ~AreaHighlightNode();

private:
  /// Create the geometry of the highlight
  void init_();

  osg::ref_ptr<osg::Uniform> color_;
  osg::ref_ptr<osg::Uniform> radius_;
};

/// Draws a line around the area using a LineDrawable
class SDKVIS_EXPORT LineDrawableHighlightNode : public HighlightNode
{
public:
  /** Declare boilerplate code */
  META_Node(simVis, LineDrawableHighlightNode);
  LineDrawableHighlightNode();
  LineDrawableHighlightNode(const LineDrawableHighlightNode& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Draws a diamond around the area */
  void makeDiamond();
  /** Draws a square around the area */
  void makeSquare();
  /** Draws a circle around the area */
  void makeCircle();
  /** Draws a squared reticle around the area, like [ ] but with sides gone */
  void makeSquareReticle();
  /** Draws a coffin shape, such as for kill/rebirth functionality */
  void makeCoffin();

  // From HighlightNode:
  virtual void setColor(const osg::Vec4f& rgba);
  virtual void setRadius(float radius);
  virtual void setAutoRotate(bool autoRotate);

protected:
  /// osg::Referenced-derived
  virtual ~LineDrawableHighlightNode();

private:
  /// Create the geometry of the highlight
  void init_();
  /// Reset the number of lines, clear them, and set the mode
  void resetLines_(size_t newLineCount, int glMode);

  osg::ref_ptr<osg::AutoTransform> billboard_;
  std::vector<osg::ref_ptr<osgEarth::LineDrawable> > lines_;
  bool autoRotate_ = true;
};

/// Choose between different highlight nodes based on an enum
class SDKVIS_EXPORT CompositeHighlightNode : public HighlightNode
{
public:
  /** Declare boilerplate code */
  META_Node(simVis, CompositeHighlightNode);
  explicit CompositeHighlightNode(simData::CircleHilightShape shape=simData::CH_PULSING_CIRCLE);
  CompositeHighlightNode(const CompositeHighlightNode& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Sets the shape */
  void setShape(simData::CircleHilightShape shape);

  // From HighlightNode:
  virtual void setColor(const osg::Vec4f& rgba);
  virtual void setRadius(float radius);
  virtual void setAutoRotate(bool autoRotate);

protected:
  /// osg::Referenced-derived
  virtual ~CompositeHighlightNode();

private:
  osg::ref_ptr<HighlightNode> child_;
  simData::CircleHilightShape shape_;
  osg::Vec4f rgba_;
  float radius_;
  bool autoRotate_ = false;
};

} // namespace simVis

#endif // SIMVIS_AREA_HIGHLIGHT_H
