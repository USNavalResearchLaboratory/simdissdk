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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_COLORBUTTON_H
#define SIMQT_COLORBUTTON_H

#include <QColor>
#include <QPushButton>
#include "simCore/Common/Export.h"


namespace simQt {

/** Class that creates a colored QPushButton, will apply alpha blended gradient from upper left to lower right if showAlpha is specified */
class SDKQT_EXPORT ColorButton : public QPushButton
{
  Q_OBJECT;

  /** Sets/gets the initial color in Qt Designer */
  Q_PROPERTY(QColor InitialColor READ color WRITE setColor)
  /** Show/hide alpha in Qt Designer */
  Q_PROPERTY(bool ShowAlpha READ showAlpha WRITE setShowAlpha)

public:
  /// Constructor
  ColorButton(QWidget* parent = nullptr);
  virtual ~ColorButton();

  /** returns the current color selection */
  QColor color() const;
  /** returns whether to show alpha channel */
  bool showAlpha() const;

  /**
  * draws a colored rectangle using the provided painter, creating a blend with the alpha channel if indicated
  * @param painter  used to draw the rectangle provided
  * @param rect  rectangle to draw
  * @param color  color to apply to the rectangle
  * @param showAlpha  if true, will apply alpha blending to the color, if false, just paints color directly
  */
  static void paintColoredSquare(QPainter* painter, const QRect& rect, const QColor& color, bool showAlpha = true);

public slots:

  /** Changes the color of the widget */
  void setColor(const QColor& value);
  /** set whether to show alpha channel or not */
  void setShowAlpha(bool value);

signals:
  /** Emitted when double clicked */
  void doubleClicked(QMouseEvent* evt=nullptr);

protected:
  /** Override the paint event to draw the gradient blending alpha if necessary */
  virtual void paintEvent(QPaintEvent* ev);
  /** Override the double click event to send a signal */
  virtual void mouseDoubleClickEvent(QMouseEvent* evt);

private:
 void paintItemBackground_(QPainter* painter) const;
 void paintColoredSquare_(QPainter* painter) const;

 bool showAlpha_; ///< should alpha be applied to the color
 QColor color_; ///< color to paint the button's background
};

}

#endif /* SIMQT_COLORWIDGET_H */
