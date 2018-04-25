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
#ifndef SIMVIS_BOXGRAPHIC_H
#define SIMVIS_BOXGRAPHIC_H

#include "osg/ref_ptr"
#include "osg/Vec4"
#include "osg/Group"
#include "simCore/Common/Common.h"

namespace osgEarth { class LineDrawable; }

namespace simVis {

/** Draws a simple box graphic. */
class SDKVIS_EXPORT BoxGraphic : public osg::Group
{
public:
  /**
  * Build the graphic using the specified parameters. Will remove any prior instance of the graphics.
  * @param x starting object space coordinate x value
  * @param y starting sobject space coordinate x value
  * @param width in pixels
  * @param height in pixels
  * @param lineWidth in pixels
  * @param stipple value for line style
  * @param color line color
  */
  BoxGraphic(double x = 0., double y = 0., double width = 0., double height = 0.,
    float lineWidth = 2., unsigned short stipple = 0x9999, const osg::Vec4& color = osg::Vec4(1.0, 1.0, 1.0, 1.0));

  BoxGraphic(const BoxGraphic& rhs);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "BoxGraphic"; }

  /** Get starting object space coordinate x value, in pixels */
  double x() const;

  /** Get starting object space coordinate x value, in pixels */
  double y() const;

  /** Get total width, in pixels */
  double width() const;

  /** Get total height, in pixels */
  double height() const;

  /** Get line width, in pixels */
  float lineWidth() const;

  /** Retrieve the stipple factor */
  unsigned int stippleFactor() const;

  /** Get stipple pattern value */
  unsigned short stipplePattern() const;

  /** Get color vector, value ranges 0.0-1.0 (R, G, B, A)*/
  osg::Vec4 color() const;

  /**
  * Set the starting screen XY and the size, in pixels
  * @param x starting object space coordinate x value, in pixels
  * @param y starting object space coordinate x value, in pixels
  * @param width of the box, in pixels
  * @param height of the box, in pixels
  */
  void setGeometry(double x, double y, double width, double height);

  /**
  * Sets the width of the line.
  * @param lineWidth Width to which to set the line.
  */
  void setLineWidth(float lineWidth);

  /**
  * Sets the stipple factor.
  * @param factor Stipple factor for the line.
  */
  void setStippleFactor(unsigned int factor);

  /**
  * Sets the stipple pattern in OpenGL format.
  * @param stipple Stipple pattern for the line.
  */
  void setStipplePattern(unsigned short stipple);

  /**
  * Sets the box's line color.
  * @param color vector, value ranges 0.0-1.0 (R,G,B,A).
  */
  void setColor(const osg::Vec4& color);

protected:
  /// osg::Referenced-derived
  virtual ~BoxGraphic();

private:
  /// create the line geometry
  void create_();

  /// starting screen coordinate x value, in pixels
  double x_;
  /// starting screen coordinate y value, in pixels
  double y_;
  /// width in pixels
  double width_;
  /// height in pixels
  double height_;
  /// line width in pixels
  float lineWidth_;
  /// stipple factor value
  unsigned int stippleFactor_;
  /// stipple pattern value
  unsigned short stipplePattern_;
  /// color vector, value ranges 0.0-1.0 (R,G,B,A)
  osg::Vec4 color_;
  /// geometry used to draw the box
  osg::ref_ptr<osgEarth::LineDrawable> geom_;

};

}

#endif /* SIMVIS_BOXGRAPHIC_H */
