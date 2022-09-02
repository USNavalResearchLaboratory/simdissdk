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
#ifndef SIMQT_COLORGRADIENTWIDGET_H
#define SIMQT_COLORGRADIENTWIDGET_H

#include <memory>
#include <QColor>
#include <QWidget>
#include "simCore/Common/Export.h"
#include "simQt/ColorGradient.h"

class QGroupBox;
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

  /** Retrieves the minimum user value (typically 0.f, for percentage) */
  Q_PROPERTY(double MinimumUserValue READ minimumUserValue WRITE setMinimumUserValue)
  /** Retrieves the maximum user display value (typically 100.f, for percentage) */
  Q_PROPERTY(double MaximumUserValue READ maximumUserValue WRITE setMaximumUserValue)

  /** Changes the value suffix, used in tooltip and model values */
  Q_PROPERTY(QString ValueSuffix READ valueSuffix WRITE setValueSuffix)
  /** If true, suffix is shown in the table's header */
  Q_PROPERTY(bool SuffixInTableHeader READ suffixInTableHeader WRITE setSuffixInTableHeader)
  /** If true, suffix is shown for each item in the table */
  Q_PROPERTY(bool SuffixInTableItems READ suffixInTableItems WRITE setSuffixInTableItems)

public:
  /** Constructor */
  explicit ColorGradientWidget(QWidget* parent = nullptr);
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
  /** Retrieves the minimum user value (typically 0.f, for percentage) */
  double minimumUserValue() const;
  /** Retrieves the maximum user display value (typically 100.f, for percentage) */
  double maximumUserValue() const;
  /** Value suffix for data values */
  QString valueSuffix() const;
  /** If true, suffix is shown in the table's header */
  bool suffixInTableHeader() const;
  /** If true, suffix is shown for each item in the table */
  bool suffixInTableItems() const;

  /** Returns true if the gradient in the widget is currently valid */
  bool gradientIsValid() const;

  /** Returns true if the gradient has changed since last call to setColorGradient() */
  bool hasChanges() const;

public slots:
  /** Sets whether to display the color stops table */
  void setShowTable(bool show);
  /** Sets whether the color editors will allow editing of alpha values */
  void setShowAlpha(bool show);
  /** Sets whether the button to spawn a help dialog is shown */
  void setShowHelp(bool show);
  /** Changes the minimum user value */
  void setMinimumUserValue(double val);
  /** Changes the maximum user value */
  void setMaximumUserValue(double val);
  /** Changes the value suffix, used in tooltip and model values */
  void setValueSuffix(const QString& suffix);
  /** If true, suffix is shown in the table's header */
  void setSuffixInTableHeader(bool val);
  /** If true, suffix is shown for each item in the table */
  void setSuffixInTableItems(bool val);

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

  /** Sets the gradient to default */
  void setGradientDefault_();
  /** Sets the gradient to darker */
  void setGradientDarker_();
  /** Sets the gradient to greyscale */
  void setGradientGreyscale_();
  /** Sets the gradient to doppler */
  void setGradientDoppler_();

private:
  /** Creates or destroys the stops table based on showTable_ flag */
  void showOrHideTable_();
  /** Updates the model and display widget with updated min/max values */
  void updateMinMaxUserValues_();

  std::unique_ptr<Ui_ColorGradientWidget> ui_;
  QGroupBox* tableGroup_;
  QTreeView* treeView_;

  class ColorGradientModel;
  ColorGradientModel* model_;

  class GradientDisplayWidget;
  GradientDisplayWidget* display_;

  bool showTable_;
  bool showAlpha_;
  bool showHelp_;

  double minUserValue_;
  double maxUserValue_;
  QString valueSuffix_;

  /** Tracks whether the gradient has changed since last call to setColorGradient() */
  bool hasChanges_ = false;
};
}

#endif /* SIMQT_COLORGRADIENTWIDGET_H */
