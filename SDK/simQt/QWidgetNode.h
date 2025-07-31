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
#ifndef SIMQT_QWIDGETNODE_H
#define SIMQT_QWIDGETNODE_H

#include <QImage>
#include "osg/ref_ptr"
#include "osg/Geometry"
#include "simCore/Common/Export.h"

class QLabel;
class QWidget;
namespace osg {
  class Image;
}

namespace simQt {

/**
 * Base class for an osg::Geometry implementation that shows a QImage-based
 * display. This might be an actual QImage, or a QImage from a QWidget. The
 * Geometry starts at (0,0) and has a size matching the image's width and height.
 */
class SDKQT_EXPORT QImageBasedNode : public osg::Geometry
{
public:
  /** Width of the image-based content in pixels */
  int width() const;
  /** Height of the image-based content in pixels */
  int height() const;

protected:
  META_Node(simQt, QImageBasedNode);
  /** Not intended to be instantiated directly; use derived instances. */
  QImageBasedNode();
  QImageBasedNode(const QImageBasedNode& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Updates the graphics to show the given image */
  void setImage_(const QImage& image);

  /** Converts a QImage into an osg::Image */
  void qImageToOsgImage_(const QImage& qImage, osg::Image& toImage) const;

  /** From osg::Referenced */
  virtual ~QImageBasedNode();

private:
  QImage qImage_;
  osg::ref_ptr<osg::Image> image_;
  osg::ref_ptr<osg::Vec3Array> vertices_;
};

/**
 * Node representation of an image. Handles conversion from a QImage into an osg::Image
 * and associates the texture with the class's geometry.
 */
class SDKQT_EXPORT QImageNode : public QImageBasedNode
{
public:
  META_Node(simQt, QImageNode);
  QImageNode();
  QImageNode(const QImageNode& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Updates the graphics to show the given image */
  void setImage(const QImage& image);

protected:
  /** From osg::Referenced */
  virtual ~QImageNode();
};

/** QWidget-based implementation. This is display only and does not process mouse or keyboard interaction. */
class SDKQT_EXPORT QWidgetNode : public QImageBasedNode
{
public:
  META_Node(simQt, QWidgetNode);
  QWidgetNode();
  QWidgetNode(const QWidgetNode& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Updates the graphics to show the given widget */
  void render(QWidget* widget);

protected:
  /** From osg::Referenced */
  virtual ~QWidgetNode();
};

/** QLabel-based implementation renders the label twice for a drop-shadow effect. */
class SDKQT_EXPORT QLabelDropShadowNode : public QImageBasedNode
{
public:
  META_Node(simQt, QLabelDropShadowNode);
  QLabelDropShadowNode();
  QLabelDropShadowNode(const QLabelDropShadowNode& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Updates the graphics to show the given label */
  void render(QLabel* label);

protected:
  /** From osg::Referenced */
  virtual ~QLabelDropShadowNode();
};

}

#endif /* SIMQT_QWIDGETNODE_H */
