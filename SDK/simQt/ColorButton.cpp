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

#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include "simQt/ColorButton.h"

namespace
{
  /** Blend two colors */
  QColor blendColors(const QColor& src, const QColor& dest)
  {
    double pct = dest.alpha() / 255.0;
    return QColor(static_cast<int>(src.red() + (dest.red() - src.red()) * pct),
      static_cast<int>(src.green() + (dest.green() - src.green()) * pct),
      static_cast<int>(src.blue() + (dest.blue() - src.blue()) * pct),
      255);
  }
}

namespace simQt {

ColorButton::ColorButton(QWidget* parent)
  :QPushButton(parent),
  showAlpha_(true),
  color_(QColor(0,0,0,255))
{
}

ColorButton::~ColorButton()
{}

void ColorButton::paintEvent(QPaintEvent* ev)
{
  QPainter painter(this);
  paintColoredSquare_(&painter);
}

bool ColorButton::showAlpha() const
{
  return showAlpha_;
}

void ColorButton::setShowAlpha(bool value)
{
  showAlpha_ = value;
}

QColor ColorButton::color() const
{
  return color_;
}

void ColorButton::setColor(const QColor& value)
{
  color_ = value;
  // schedule a repaint
  update();
}

void ColorButton::mouseDoubleClickEvent(QMouseEvent* evt)
{
  QPushButton::mouseDoubleClickEvent(evt);
  Q_EMIT doubleClicked(evt);
}

void ColorButton::paintColoredSquare(QPainter* painter, const QRect& rect, const QColor& color, bool showAlpha)
{

  if (showAlpha)
  {
    // create the blended gradient if showing alpha channel
    // Set up the gradient.  A stop at 0.4999/0.5 is possible for a discrete color change
    QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
    QColor whiteBlend = blendColors(Qt::white, color);
    QColor blackBlend = blendColors(Qt::black, color);
    gradient.setColorAt(0.0, whiteBlend);
    gradient.setColorAt(1.0, blackBlend);
    painter->setBrush(gradient);
  }
  else
  {
    // if no alpha, just draw the whole thing solid
    QBrush brush = painter->brush();
    brush.setColor(color);
    brush.setStyle(Qt::SolidPattern);
    painter->setBrush(brush);
  }

  // Note that if Anti-aliasing is disabled, then a regular (not rounded) rect should be drawn.
  painter->setRenderHint(QPainter::Antialiasing);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  painter->setRenderHint(QPainter::HighQualityAntialiasing);
#endif

  // Paints the square
  painter->drawRoundedRect(rect, 2, 2);
}

void ColorButton::paintColoredSquare_(QPainter* painter) const
{
  QStyleOptionButton option;
  initStyleOption(&option);

  // Calculate the rectangle size for the drawn box
  QRect rect = option.rect.adjusted(2, 2, 0, -2);
  rect.setWidth(height() - 4);
  if (isEnabled())
    ColorButton::paintColoredSquare(painter, rect, color_, showAlpha_);
  else
    ColorButton::paintColoredSquare(painter, rect, QColor(0, 0, 0, 0), true);
}

}
