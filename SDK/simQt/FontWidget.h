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
#ifndef SIMQT_FONTWIDGET_H
#define SIMQT_FONTWIDGET_H

#include <QHash>
#include <QString>
#include <QFileInfo>
#include <QWidget>
#include "simCore/Common/Export.h"

class QDir;
class Ui_FontWidget;

namespace simQt {

/**
 * FontWidget is a font selector widget that searches the path provided for the font file names
 * and provides a combo box filled with all the valid fonts on that path, displaying the font family name.
 * Also provides widgets for font color and size.  There is the option to hide both the size and color widgets.
 * Emits a signal with the font file name (without the full path) when the combo box selection is changed,
 * or when size or color widgets are changed.  The user can query for the current selected font file name, size
 * or color directly.
 */
class SDKQT_EXPORT FontWidget : public QWidget  // QDESIGNER_WIDGET_EXPORT
{
  Q_OBJECT

  /** Sets/gets the showing of the font size in Qt Designer */
  Q_PROPERTY(bool showFontSize READ showFontSize WRITE setShowFontSize)
  /** Sets/gets the showing of the font color in Qt Designer */
  Q_PROPERTY(bool showFontColor READ showFontColor WRITE setShowFontColor)
  /** Sets/gets the font size in Qt Designer */
  Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize)
  /** Sets/gets the font color in Qt Designer */
  Q_PROPERTY(QColor fontColor READ fontColor WRITE setFontColor)

public:
  /**
   * Constructor
   * @param parent  parent widget
   */
  explicit FontWidget(QWidget* parent = nullptr);

  /** Destructor */
  virtual ~FontWidget();

  /**
  * Get the currently selected font file name (not full path)
  * @return The current font's file name (e.g. arialbd.ttf)
  */
  QString fontFile();

  /**
  * Get the currently selected font file name, full path
  * @return The current font's full path file name
  */
  QString fontFullPathFile();

  /**
  * Returns the current font color selected
  * @return font color
  */
  QColor fontColor() const;

  /**
  * Returns the current font size selected
  * @return font size
  */
  int fontSize() const;

  /**
  * Returns if the font color widget is visible
  * @return True if font color user editing field is shown
  */
  bool showFontColor() const;

  /**
  * Returns if the font size widget is visible
  * @return True if font size user editing field is shown
  */
  bool showFontSize() const;

  /**
  * Set the font directory to rebuild the combo box with all font files found in the provided directory
  * @param fontDir  new font directory location
  */
  void setFontDir(const QString& fontDir);

  /**
  * Set the enabled state of the font name combo box. Defaults to enabled
  * @param enabled  new enabled state for the font name combo box
  */
  void setFontNameEnabled(bool enabled);

  /**
  * Returns if custom font files are available on the current system with the configured font directory
  * @return True if custom font files are available on the current system with the configured font directory
  */
  bool customFontsAvailable() const;

public Q_SLOTS:

  /**
   * Set the current font name based on the provided font file name.  Searches in the fontDir_ to insure this font exists.
   * fontFile should be simple file name, not full path
   * @param fontFile font file name
   */
  void setFontFile(const QString& fontFile);

  /**
   * Set the current font color in the widget
   * @param fontColor Color for the font
   */
  void setFontColor(const QColor& fontColor);

  /**
   * Set the current font size in the widget
   * @param fontSize Point size of the new font
   */
  void setFontSize(int fontSize);

  /**
   * Hide or show the font color widget
   * @param showColor True if the color editing should be shown
   */
  void setShowFontColor(bool showColor);

  /**
   * Hide or show the font size widget
   * @param showSize True if the font size editing should be shown
   */
  void setShowFontSize(bool showSize);

Q_SIGNALS:

  /**
   * Emitted when the combo box is changed, sends out the new font file name for the selected font
   * @param fontFile  selected font file name
   */
  void fontFileChanged(const QString& fontFile);

  /**
   * Emitted when the font size is changed
   * @param fontSize
   */
  void fontSizeChanged(int fontSize);

  /**
   * Emitted when the font color is changed
   * @param fontColor
   */
  void fontColorChanged(const QColor& fontColor);

private Q_SLOTS:
  /** Called when the font name combo box is changed */
  void fontNameChanged_(const QString& fontName);

private:
  /** Given an absolute path of a font, return a description of the font. */
  QString getFriendlyFontName_(const QString& absolutePath) const;

  /// manages the font directory
  QDir* fontDir_;
  /// Font widget gui
  Ui_FontWidget* ui_;
  /// holds a map of the font family name to their file info, so insure no duplicates
  QHash<QString, QFileInfo> fontFiles_;
  /// If true, use the friendly font name of the font file, generated by getFriendlyFontName_()
  bool useFriendlyFontName_;
};

}

#endif /* SIMQT_FONTWIDGET_H */
