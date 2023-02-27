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
#ifndef SIMQT_DIRECTORY_SELECTOR_WIDGET_H
#define SIMQT_DIRECTORY_SELECTOR_WIDGET_H

#include <QWidget>
#include "simCore/Common/Export.h"

class Ui_DirectorySelectorWidget;
class QLabel;

namespace simQt {

/**
 * DirectorySelectorWidget is a widget that shows a text field and button for
 * directory selection.  The text field is grayed out but shows the selected
 * directory.  The button opens a dialog to select a directory.
 */
class SDKQT_EXPORT DirectorySelectorWidget : public QWidget
{
  Q_OBJECT;

  /** Sets/gets the include label in Qt Designer */
  Q_PROPERTY(bool includeLabel READ includeLabel WRITE setIncludeLabel);
  /** Sets/gets the label in Qt Designer */
  Q_PROPERTY(QString label READ label WRITE setLabel);
  /** Sets/gets the title for file browser label in Qt Designer */
  Q_PROPERTY(QString titleForFileBrowser READ browserTitle WRITE setBrowserTitle);
  /** Sets/gets the registry key in Qt Designer */
  Q_PROPERTY(QString registryKey READ registryKey WRITE setRegistryKey);
  /** Sets/gets the default path in Qt Designer */
  Q_PROPERTY(QString defaultPath READ defaultPath WRITE setDefaultPath);

public:
  /** Constructor */
  DirectorySelectorWidget(QWidget* parent=nullptr);
  virtual ~DirectorySelectorWidget();

  /** Sets the registry key for saving/loading files; defaults to SETTINGS/file */
  void setRegistryKey(const QString& regKey);
  /** Retrieves current registry key setting */
  QString registryKey() const;

  /**
   * Sets the default path for the registry entry when loading/saving files.  Only applies if no
   * path is in QSettings for the key.  If this value is non-empty, the QSettings registry key
   * value is initialized with this path.  For more details, see simQt::FileDialog::setRegistryDir().
   * Note that environment variables are supported using $(ENV_VAR) syntax.
   */
  void setDefaultPath(const QString& defaultPath);
  /**
   * Retrieves the currently set default path.  Does not query registry.  Empty value means no
   * explicit default.  May contain environment variables.
   */
  QString defaultPath() const;

  /** If True, include a label to the left */
  void setIncludeLabel(bool value);
  /** If True, include a label to the left */
  bool includeLabel() const;

  /** Set the label to the left of the text field */
  void setLabel(const QString& title);
  /** Get the label to the left of the text field */
  QString label() const;

  /** Sets the title of the dialog box that pops up for the ... button */
  void setBrowserTitle(const QString& title);
  /** Returns currently set title for the dialog box pop up */
  QString browserTitle() const;

  /** Retrieve currently set directory */
  QString directory() const;

public Q_SLOTS:
  /** Sets the directory this selector represents */
  void setDirectory(const QString& dir);

Q_SIGNALS:
  /** Emitted when the directory changes */
  void directoryChanged(const QString& dir);

private Q_SLOTS:
  /** Connects to the ... button */
  void loadButton_();
  /** used during debug to allow developer to type in a file name */
  void editingFinished_();
  void textEdited_();

private:
  Ui_DirectorySelectorWidget* ui_;
  /** location for saving state information */
  QString registryKey_;
  /** Default path string for the registry key (possibly empty) */
  QString defaultPath_;
  /** The optional label to the left */
  QLabel* labelWidget_;
  /** True means the label is included */
  bool includeLabel_;
  /** The Optional text to the left of the text field for the file name */
  QString label_;
  /** The title for the file browser */
  QString browserTitle_;

  /** Re-implement eventFilter() */
  virtual bool eventFilter(QObject* obj, QEvent* evt);

};


}

#endif
