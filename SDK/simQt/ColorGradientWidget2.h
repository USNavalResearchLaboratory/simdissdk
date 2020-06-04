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
#ifndef SIMQT_ColorGradientWidget2_H
#define SIMQT_ColorGradientWidget2_H

#include <memory>
#include <QColor>
#include <QWidget>
#include "simCore/Common/Export.h"
#include "simQt/ColorGradient.h"

class QSortFilterProxyModel;
class Ui_ColorGradientWidget2;

namespace simQt {

/** Qt widget that enables customization of a multi-stop color gradient */
class SDKQT_EXPORT ColorGradientWidget2 : public QWidget
{
  Q_OBJECT;

public:
  /** Constructor */
  explicit ColorGradientWidget2(QWidget* parent = NULL);
  virtual ~ColorGradientWidget2();

  /**
   * Sets the current gradient to the given ColorGradient.
   * Emits gradientChanged() only if changed.
   */
  void setColorGradient(const ColorGradient& gradient);
  ColorGradient getColorGradient() const;

  /** Removes all colors and stops. */
  void clear();

public slots:
  void showTable(bool show);

signals:
  void gradientChanged();

private:
  std::unique_ptr<Ui_ColorGradientWidget2> ui_;

  class ColorGradientModel;
  ColorGradientModel* model_;
  QSortFilterProxyModel* proxyModel_;

  class GradientDisplayWidget;
};
}

#endif /* SIMQT_ColorGradientWidget2_H */
