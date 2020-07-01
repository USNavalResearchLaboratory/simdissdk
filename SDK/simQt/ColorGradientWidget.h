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
#ifndef SIMQT_COLORGRADIENTWIDGET_H
#define SIMQT_COLORGRADIENTWIDGET_H

#include <memory>
#include <QColor>
#include <QWidget>
#include "simCore/Common/Export.h"
#include "simQt/ColorGradient.h"

class QGroupBox;
class QSortFilterProxyModel;
class QTreeView;
class Ui_ColorGradientWidget;

namespace simQt {

/** Qt widget that enables customization of a multi-stop color gradient */
class SDKQT_EXPORT ColorGradientWidget : public QWidget
{
  Q_OBJECT;

  /** Shows/hides the gradient stop list below the display widget */
  Q_PROPERTY(bool ShowTable READ showTable WRITE setShowTable)
  /** Show/hide alpha in displayed color selectors */
  Q_PROPERTY(bool ShowAlpha READ showAlpha WRITE setShowAlpha)
  /** Show/hide button to spawn help dialog */
  Q_PROPERTY(bool ShowHelp READ showHelp WRITE setShowHelp)

public:
  /** Constructor */
  explicit ColorGradientWidget(QWidget* parent = NULL);
  virtual ~ColorGradientWidget();

  /**
   * Sets the current gradient to the given ColorGradient.
   * Emits gradientChanged() only if changed.
   */
  void setColorGradient(const ColorGradient& gradient);
  ColorGradient getColorGradient() const;

  /** Removes all colors and stops. */
  void clear();

  /** Returns true if the color stops table is displayed */
  bool showTable() const;
  /** Returns true if the color editors will show alpha values */
  bool showAlpha() const;
  /** Returns true if the button to spawn a help dialog is shown */
  bool showHelp() const;

  /** Returns true if the gradient in the widget is currently valid */
  bool gradientIsValid() const;

public slots:
  /** Sets whether to display the color stops table */
  void setShowTable(bool show);
  /** Sets whether the color editors will allow editing of alpha values */
  void setShowAlpha(bool show);
  /** Sets whether the button to spawn a help dialog is shown */
  void setShowHelp(bool show);

signals:
  /** Emitted whenever a change is made to the stored color gradient */
  void gradientChanged(const simQt::ColorGradient& gradient);

private slots:
  /**
   * Trigged by changes to the gradient, emits gradientChanged()
   * with the appropriate ColorGradient object
   */
  void emitGradientChanged_();
  /** Spawns a help dialog explaining the display widget */
  void showHelpDialog_();

private:
  /** Creates or destroys the stops table based on showTable_ flag */
  void showOrHideTable_();

  std::unique_ptr<Ui_ColorGradientWidget> ui_;
  QGroupBox* tableGroup_;
  QTreeView* treeView_;

  class ColorGradientModel;
  ColorGradientModel* model_;
  QSortFilterProxyModel* proxyModel_;

  class GradientDisplayWidget;

  bool showTable_;
  bool showAlpha_;
  bool showHelp_;
};
}

#endif /* SIMQT_COLORGRADIENTWIDGET_H */
