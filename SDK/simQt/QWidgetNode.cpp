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
#include <cstring>
#include <QLabel>
#include <QPainter>
#include <QWidget>
#include "osg/Texture2D"
#include "simVis/Utils.h"
#include "simQt/QWidgetNode.h"

namespace simQt
{

QImageBasedNode::QImageBasedNode() :
  image_(simVis::makeBrokenImage()),
  vertices_(new osg::Vec3Array)
{
  setName("simQt::QImageBasedNode");
  setUseVertexBufferObjects(true);
  setUseDisplayList(false);
  setDataVariance(osg::Object::DYNAMIC);

  // Create an initial image
  image_->setDataVariance(osg::Object::DYNAMIC);

  // Initialize the quad geometry (initial size) using a triangle strip
  const int width = image_->s();
  const int height = image_->t();
  vertices_->push_back(osg::Vec3(0, 0, 0)); // Vertex 0
  vertices_->push_back(osg::Vec3(width, 0, 0)); // Vertex 1
  vertices_->push_back(osg::Vec3(0, height, 0)); // Vertex 2
  vertices_->push_back(osg::Vec3(width, height, 0)); // Vertex 3
  vertices_->setDataVariance(osg::Node::DYNAMIC);
  setVertexArray(vertices_.get());
  addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, 4));

  auto* c = new osg::Vec4Array(osg::Array::BIND_OVERALL);
  c->push_back(osg::Vec4f(1.f, 1.f, 1.f, 1.f));
  setColorArray(c);

  auto* texcoords = new osg::Vec2Array;
  // Invert texture coordinate Y value instead of flipping the image in memory
  texcoords->push_back(osg::Vec2(0, 1));
  texcoords->push_back(osg::Vec2(1, 1));
  texcoords->push_back(osg::Vec2(0, 0));
  texcoords->push_back(osg::Vec2(1, 0));
  setTexCoordArray(0, texcoords);

  // Set up the texture
  auto* texture = new osg::Texture2D;
  texture->setResizeNonPowerOfTwoHint(false);
  texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
  texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
  texture->setImage(image_.get());
  simVis::fixTextureForGlCoreProfile(texture);

  // Set up the StateSet
  getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
  osgEarth::Registry::shaderGenerator().run(this);
}

QImageBasedNode::QImageBasedNode(const QImageBasedNode& rhs, const osg::CopyOp& copyOp)
  : QImageBasedNode()
{
  setImage_(rhs.qImage_);
}

QImageBasedNode::~QImageBasedNode()
{
  // osg::ref_ptr handles memory management
}

int QImageBasedNode::width() const
{
  return image_->s();
}

int QImageBasedNode::height() const
{
  return image_->t();
}

void QImageBasedNode::setImage_(const QImage& image)
{
  if (image.isNull())
    return;

  qImage_ = image;
  qImageToOsgImage_(qImage_, *image_);

  // Update vertex positions to match image dimensions
  vertices_->at(1).set(image.width(), 0, 0);
  vertices_->at(2).set(0, image.height(), 0);
  vertices_->at(3).set(image.width(), image.height(), 0);
  vertices_->dirty();
}

void QImageBasedNode::qImageToOsgImage_(const QImage& qImage, osg::Image& toImage) const
{
  if (qImage.isNull())
    return;

  // Convert to a known format (RGBA8888) for simplicity
  const QImage convertedImage = qImage.convertToFormat(QImage::Format_RGBA8888);

  const int width = convertedImage.width();
  const int height = convertedImage.height();
  const int bytesPerLine = convertedImage.bytesPerLine();
  const int totalBytes = height * bytesPerLine;

  // Allocate memory for the image data
  unsigned char* imageData = new unsigned char[totalBytes];

  // Copy the image data from the QImage to the newly allocated memory
  std::memcpy(imageData, convertedImage.constBits(), totalBytes);
  toImage.setImage(width, height, 1, // s, t, r
    GL_RGBA8,          // internalTextureformat
    GL_RGBA,           // pixelFormat
    GL_UNSIGNED_BYTE,  // type
    imageData,         // data
    osg::Image::USE_NEW_DELETE);
  toImage.dirty();
}

/////////////////////////////////////////////////////////////////////

QImageNode::QImageNode()
{
  setName("simQt::QImageNode");
}

QImageNode::QImageNode(const QImageNode& rhs, const osg::CopyOp& copyOp)
  : QImageBasedNode(rhs, copyOp)
{
}

QImageNode::~QImageNode()
{
}

void QImageNode::setImage(const QImage& image)
{
  // Direct pass-through to inherited method
  setImage_(image);
}

/////////////////////////////////////////////////////////////////////

QWidgetNode::QWidgetNode()
{
  setName("simQt::QWidgetNode");
}

QWidgetNode::QWidgetNode(const QWidgetNode& rhs, const osg::CopyOp& copyOp)
  : QImageBasedNode(rhs, copyOp)
{
}

QWidgetNode::~QWidgetNode()
{
}

void QWidgetNode::render(QWidget* widget)
{
  if (!widget)
    return;

  QImage image(widget->size(), QImage::Format_RGBA8888);
  image.fill(Qt::transparent);
  QPainter painter(&image);
  widget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
  setImage_(image);
}

/////////////////////////////////////////////////////////////////////

QLabelDropShadowNode::QLabelDropShadowNode()
{
  setName("simQt::QLabelDropShadowNode");
}

QLabelDropShadowNode::QLabelDropShadowNode(const QLabelDropShadowNode& rhs, const osg::CopyOp& copyOp)
  : QImageBasedNode(rhs, copyOp)
{
}

QLabelDropShadowNode::~QLabelDropShadowNode()
{
}

void QLabelDropShadowNode::render(QLabel* label)
{
  if (!label)
    return;

  // Render an empty image if the text is empty or size will be 0. Doing so
  // here avoids QPainter errors with invalid label content below.
  if (label->text().isEmpty() || label->width() <= 0 || label->height() <= 0)
  {
    QImage image({ 1, 1 }, QImage::Format_RGBA8888);
    image.fill(Qt::transparent);
    setImage_(image);
    return;
  }

  constexpr int shadowOffset = 1;
  const auto& labelSize = label->size();
  QImage image(labelSize.width(), labelSize.height(),
    QImage::Format_RGBA8888);
  image.fill(Qt::transparent);

  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);

  const QString oldStyle = label->styleSheet();
  label->setStyleSheet(oldStyle + " ; color: black;");
  label->render(&painter, QPoint(shadowOffset, shadowOffset), QRegion(), QWidget::DrawChildren);
  label->setStyleSheet(oldStyle);
  label->render(&painter, QPoint(0, 0), QRegion(), QWidget::DrawChildren);

  setImage_(image);
}

} // namespace simQt
