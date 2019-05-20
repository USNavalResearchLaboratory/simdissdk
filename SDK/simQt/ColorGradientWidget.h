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

#include <QWidget>
#include "simCore/Common/Export.h"

class Ui_ColorGradientWidget;
class QLabel;

namespace simQt {

class ColorButton;

/** Qt widget that enables customization of a multi-stop color gradient */
class SDKQT_EXPORT ColorGradientWidget : public QWidget
{
  Q_OBJECT;

public:
  /** Constructor */
  explicit ColorGradientWidget(QWidget* parent = NULL);
  virtual ~ColorGradientWidget();

private:
  Ui_ColorGradientWidget* ui_;
};

}

#endif /* SIMQT_COLORGRADIENTWIDGET_H */
