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
#ifndef SIMQT_FILESELECTOR_WIDGET_H
#define SIMQT_FILESELECTOR_WIDGET_H

#include <QWidget>
#include "simCore/Common/Export.h"

class Ui_FileSelectorWidget;
class QLabel;

namespace simQt {

/**
 * FileSelectorWidget is a widget that shows a text field and button for
 * file selection.  The text field is grayed out but shows the selected
 * filename.  The button opens a file dialog to select a file.
 */
class SDKQT_EXPORT FileSelectorWidget : public QWidget
{
  Q_OBJECT;

  /** Sets/gets the include label in Qt Designer */
  Q_PROPERTY(bool includeLabel READ includeLabel WRITE setIncludeLabel);
  /** Sets/gets the label in Qt Designer */
  Q_PROPERTY(QString label READ label WRITE setLabel);
  /** Sets/gets the Save/Load in Qt Designer */
  Q_PROPERTY(FileOption typeOfOperation READ fileOptions WRITE setFileOptions);
  /** Enum for either save or load in Qt Designer */
  Q_ENUMS(FileOption);
  /** Sets/gets the title for file browser in Qt Designer */
  Q_PROPERTY(QString titleForFileBrowser READ browserTitle WRITE setBrowserTitle);
  /** Sets/gets the filter options in Qt Designer */
  Q_PROPERTY(FilterOptions filterForFileBrowser READ filterOption WRITE setFilterOption);
  /** Enum for filter options in Qt Designer */
  Q_ENUMS(FilterOptions);
  /** Sets/gets the custom filter in Qt Designer */
  Q_PROPERTY(QString customFileFilter READ customFileFilter WRITE setCustomFileFilter);
  /** Sets/gets the default file name in Qt Designer */
  Q_PROPERTY(QString defaultFilename READ filename WRITE setFilename);
  /** Sets/gets the registry key in Qt Designer */
  Q_PROPERTY(QString registryKey READ registryKey WRITE setRegistryKey);
  /** Sets/gets the default path in Qt Designer */
  Q_PROPERTY(QString defaultPath READ defaultPath WRITE setDefaultPath);

public:
  /** Constructor */
  FileSelectorWidget(QWidget* parent=NULL);
  virtual ~FileSelectorWidget();

  /** Declare options for the file selector */
  enum FileOption
  {
    /** Load dialog is presented; user must select existing file */
    FileLoad = 0x0,
    /** Save dialog is presented; user prompted to overwrite on selecting existing file */
    FileSave = 0x1
  };

  /// Options for file extension filtering.
  enum FilterOptions
  {
    ALL_SIMDIS_FILE_PATTERNS,
    ALL_SIMDIS_ASCII_FILE_PATTERNS,
    ALL_SIMDIS_TEXTURE_FILE_PATTERNS,
    SIMDIS_ASI_FILE_PATTERNS,
    SIMDIS_FCT_FILE_PATTERNS,
    SIMDIS_ZIP_FILE_PATTERNS,
    SIMDIS_PREFS_FILE_PATTERNS,
    SIMDIS_RULE_FILE_PATTERNS,
    SIMDIS_VIEW_FILE_PATTERNS,
    SIMDIS_BOOKMARK_FILE_PATTERNS,
    SIMDIS_BOOKMARK_SAVE_FILE_PATTERNS,
    SIMDIS_IMAGERY_CONFIG_FILE_PATTERNS,
    SIMDIS_RASTER_DB_FILE_PATTERNS,
    SIMDIS_MODEL_FILE_PATTERNS,
    MEDIA_FILE_PATTERNS,
    SIMDIS_MEDIA_FILE_PATTERNS,
    ANTENNA_FILE_PATTERNS,
    GOG_FILE_PATTERNS,
    RELATIVE_GOG_FILE_PATTERNS,
    GOG_FILE_SAVE_PATTERNS,
    POINT_MAP_FILE_PATTERNS,
    WVS_FILE_PATTERNS,
    RFPROP_CONFIG_FILE_PATTERNS,
    AREPS_CONFIG_FILE_PATTERNS,
    APM_CONFIG_FILE_PATTERNS,
    RCS_CONFIG_FILE_PATTERNS,
    HDF5_FILE_PATTERNS,
    TRACKSTATUS_REPORT_FILE_PATTERNS,
    GDAL_IMAGE_LAYER_FILE_PATTERNS,
    MBTILES_IMAGE_LAYER_FILE_PATTERNS,
    XML_FILE_PATTERNS,
    CUSTOM_USERDEFINED_FILTER
    };

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

  /** Sets the label to the left of the text field */
  void setLabel(const QString& title);
  /** Gets the label to the left of the text field */
  QString label() const;

  /** Sets the title of the dialog box that pops up for the ... button */
  void setBrowserTitle(const QString& title);
  /** Returns currently set title for the dialog box pop up */
  QString browserTitle() const;

  /** Sets filter for the dialog pop up */
  void setFilterOption(FileSelectorWidget::FilterOptions filter);
  /** Gets filter for the dialog pop up */
  FileSelectorWidget::FilterOptions filterOption() const;

  /** Sets file options, such as whether this is for loading or saving files */
  void setFileOptions(const FileSelectorWidget::FileOption& flags);
  /** Gets file options, such as whether this is for loading or saving files */
  FileSelectorWidget::FileOption fileOptions() const;

  /**
   * Sets the custom file filter.  You can use newlines to separate different
   * filters.  Use parentheses to enclose filters.  For example:
   * "Text Files (*.txt)\nAll Files (*)".  This filter will only be used if
   * the Filter Option is set to CUSTOM_USERDEFINED_FILTER.
   */
  void setCustomFileFilter(const QString& filter);
  /** Gets the custom filter */
  QString customFileFilter() const;

  /** Retrieve currently set filename */
  QString filename() const;

public slots:
  /** Sets the filename this selector represents */
  void setFilename(const QString& filename);

signals:
  /** Emitted when the filename changes */
  void filenameChanged(const QString& filename);

private slots:
  /** Connects to the ... button */
  void loadButton_();
  /** used during debug to allow developer to type in a file name */
  void editingFinished_();
  void textEdited_();

private:
  Ui_FileSelectorWidget* ui_;
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
  /** Load or Save  */
  FileSelectorWidget::FileOption flags_;
  /** The file extension */
  FileSelectorWidget::FilterOptions filterOption_;
  /** Custom filter to use if filterOption_ is set to CUSTOM_USERDEFINED_FILTER */
  QString customFileFilter_;

  /** Re-implement eventFilter() */
  virtual bool eventFilter(QObject* obj, QEvent* evt);

  /** Converts the filter option enum to a readable text */
  QString filterOptions2QString_(FileSelectorWidget::FilterOptions option) const;

};


}

#endif /* SIMQT_FILESELECTOR_WIDGET_H */
