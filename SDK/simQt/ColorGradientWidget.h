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
#ifndef SIMQT_COLORGRADIENTWIDGET_H
#define SIMQT_COLORGRADIENTWIDGET_H

#include <memory>
#include <QColor>
#include <QWidget>
#include "simCore/Common/Export.h"
#include "simQt/ColorGradient.h"

class Ui_ColorGradientWidget;

namespace simQt {

/** Qt widget that enables customization of a multi-stop color gradient */
class SDKQT_EXPORT ColorGradientWidget : public QWidget
{
  Q_OBJECT;

public:
  /** Constructor */
  explicit ColorGradientWidget(QWidget* parent = NULL);
  virtual ~ColorGradientWidget();

  void setColorGradient(const ColorGradient& gradient);
  ColorGradient getColorGradient() const;

signals:
  /** Emitted when any change to the color gradient is made */
  void gradientChanged(const simQt::ColorGradient& gradient);

private slots:
  /** Updates GUI to the values of the newly-selected gradient stop */
  void setSelectedGradientIndex_(int index);
  /** Updates the stored stop position and spinner value based on current slider position */
  void storeGradientSliderPosition_(int sliderPos);
  /** Updates the stored stop position and slider position based on current spinner value */
  void storeGradientSpinnerPosition_(double spinPos);
  /** Updates the stored stop color */
  void storeGradientColor_(const QColor& color);
  /** Creates a new stop */
  void createColor_();
  /** Deletes the current stop */
  void deleteColor_();

private:
  /** Updates the color widget to show the current gradient and emits gradientChanged(). */
  void applyGradient_();
  /** Updates the enabled state of the editing widgets based on the current stops size */
  void updateEnables_();

  Ui_ColorGradientWidget* ui_;
  std::vector<std::pair<double, QColor> > stops_;
};
}

#endif /* SIMQT_COLORGRADIENTWIDGET_H */
