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
#include <cassert>
#include <QLabel>
#include "simCore/String/FilePatterns.h"
#include "simQt/QtFormatting.h"
#include "simQt/FileDialog.h"
#include "simQt/ResourceInitializer.h"
#include "simQt/FileSelectorWidget.h"
#include "ui_FileSelectorWidget.h"

namespace simQt {

FileSelectorWidget::FileSelectorWidget(QWidget* parent)
  : QWidget(parent),
    registryKey_("Private/file"),
    label_(tr("File")),
    browserTitle_(tr("Load Data File")),
    customFileFilter_(tr("All Files (*)"))
{
  ResourceInitializer::initialize();  // Needs to be here so that Qt Designer works.

  ui_ = new Ui_FileSelectorWidget;
  ui_->setupUi(this);
  connect(ui_->fileButton, &QPushButton::clicked, this, &FileSelectorWidget::loadButton_);
  connect(ui_->clearButton, &QPushButton::clicked, this, [this](int idx) { setFilename_("", true); });
  ui_->fileButton->setToolTip(tr("Display File Browser to select file to load."));
  ui_->clearButton->setToolTip(tr("Clear current filename."));
  ui_->clearButton->setVisible(showClearButton_);
  connect(ui_->fileText, &QLineEdit::textEdited, this, &FileSelectorWidget::textEdited_);
  connect(ui_->fileText, &QLineEdit::editingFinished, this, &FileSelectorWidget::editingFinished_);
#ifndef NDEBUG
  // Developers are allowed to type by default, in debug mode; users by default must use the file browser
  ui_->fileText->setReadOnly(false);
#else
  ui_->fileText->setReadOnly(true);
#endif
  ui_->fileText->installEventFilter(this);

  // set tool tips
  setFileOptions(flags_);
}

FileSelectorWidget::~FileSelectorWidget()
{
  delete ui_;
}


bool FileSelectorWidget::iconBeforeText() const
{
  return iconBeforeText_;
}

void FileSelectorWidget::setIconBeforeText(bool alignLeft)
{
  if (iconBeforeText_ == alignLeft)
    return;
  iconBeforeText_ = alignLeft;
  QPushButton* fileButton = ui_->fileButton;
  ui_->horizontalLayout->removeWidget(fileButton);
  if (labelWidget_ != nullptr)
    ui_->horizontalLayout->removeWidget(labelWidget_);

  if (alignLeft)
  {
    ui_->horizontalLayout->insertWidget(0, fileButton);
    if (labelWidget_ != nullptr)
      ui_->horizontalLayout->addWidget(labelWidget_);
  }
  else
  {
    if (labelWidget_ != nullptr)
      ui_->horizontalLayout->insertWidget(0, labelWidget_);
    ui_->horizontalLayout->addWidget(fileButton);
  }

}

void FileSelectorWidget::setRegistryKey(const QString &regKey)
{
  registryKey_ = regKey;
}

QString FileSelectorWidget::registryKey() const
{
  return registryKey_;
}

void FileSelectorWidget::setDefaultPath(const QString& defaultPath)
{
  defaultPath_ = defaultPath;
}

QString FileSelectorWidget::defaultPath() const
{
  return defaultPath_;
}

void FileSelectorWidget::setIncludeLabel(bool value)
{
  if (value == true)
  {
    if (labelWidget_ == nullptr)
      labelWidget_ = new QLabel(label_);

    if (iconBeforeText_)
      ui_->horizontalLayout->addWidget(labelWidget_);
    else
      ui_->horizontalLayout->insertWidget(0, labelWidget_);
  }
  else if (labelWidget_)
  {
    ui_->horizontalLayout->removeWidget(labelWidget_);
    delete labelWidget_;
    labelWidget_ = nullptr;
  }

  includeLabel_ = value;
}

bool FileSelectorWidget::includeLabel() const
{
  return includeLabel_;
}

void FileSelectorWidget::setLabel(const QString &label)
{
  label_ = label;
  if (labelWidget_)
    labelWidget_->setText(label_);
}

QString FileSelectorWidget::label() const
{
  return label_;
}

void FileSelectorWidget::setBrowserTitle(const QString &title)
{
  browserTitle_ = title;
}

QString FileSelectorWidget::browserTitle() const
{
  return browserTitle_;
}

void FileSelectorWidget::setFileOptions(const FileSelectorWidget::FileOption &flags)
{
  flags_ = flags;
  if (flags_ == FileSelectorWidget::FileLoad)
  {
    ui_->fileText->setToolTip(simQt::formatTooltip(tr("Open"), tr("Enter a file name to open.<p>Double clicking the text field displays a file browser to select a file to open.")));
    ui_->fileButton->setToolTip(simQt::formatTooltip(tr("Open"), tr("Displays a file browser to select a file to open.")));
  }
  else
  {
    ui_->fileText->setToolTip(simQt::formatTooltip(tr("Save"), tr("Enter a file name to save.<p>Double clicking the text field displays a file browser to specify a file name to save.")));
    ui_->fileButton->setToolTip(simQt::formatTooltip(tr("Save"), tr("Displays a file browser to specify a file name to save.")));
  }
}

FileSelectorWidget::FileOption FileSelectorWidget::fileOptions() const
{
  return flags_;
}

QString FileSelectorWidget::filename() const
{
  return filename_;
}

void FileSelectorWidget::loadButton_()
{
  if (!defaultPath().isEmpty())
    simQt::FileDialog::setRegistryDir(registryKey_, defaultPath());
  QString file;
  if (fileOptions() & FileSave)
  {
    file = simQt::FileDialog::saveFile(this, browserTitle_, registryKey_, filterOptions2QString_(filterOption_));
  }
  else
  {
    file = simQt::FileDialog::loadFile(this, browserTitle_, registryKey_, filterOptions2QString_(filterOption_));
  }
  if (!file.isEmpty())
    setFilename_(file, true);
}

void FileSelectorWidget::setFilename(const QString& filename)
{
  setFilename_(filename, false);
}

void FileSelectorWidget::setFilename_(const QString& filename, bool canEmitFileSelected)
{
  const QString& osFilename = QDir::toNativeSeparators(filename);
  if (osFilename == filename_)
    return;
  filename_ = osFilename;
  ui_->fileText->setText(filename_);
  ui_->clearButton->setVisible(showClearButton_ && !filename_.isEmpty());
  Q_EMIT filenameChanged(filename_);
  if (canEmitFileSelected)
    Q_EMIT fileSelected(filename_);
}

bool FileSelectorWidget::isValid() const
{
  return isValid_;
}

void FileSelectorWidget::setValid(bool valid)
{
  if (isValid_ == valid)
    return;

  isValid_ = valid;

  if (isValid_)
    ui_->fileText->setStyleSheet("");
  else
    ui_->fileText->setStyleSheet("QLineEdit {color: red}");
}

bool FileSelectorWidget::eventFilter(QObject* obj, QEvent* evt)
{
  if (obj == ui_->fileText)
  {
    if (evt->type() == QEvent::MouseButtonDblClick && isEnabled())
    {
      loadButton_();
      return true;
    }
  }
  return false;
}

void FileSelectorWidget::setFilterOption(FileSelectorWidget::FilterOptions filter)
{
  filterOption_ = filter;
}

/** Currently set filter for the dialog pop up */
FileSelectorWidget::FilterOptions FileSelectorWidget::filterOption() const
{
  return filterOption_;
}

// only used in DEBUG mode
void FileSelectorWidget::editingFinished_()
{
  ui_->fileText->setStyleSheet("QLineEdit {background: palette(base); color: black;}");
  setFilename_(ui_->fileText->text(), true);
}

// only used in DEBUG mode
void FileSelectorWidget::textEdited_()
{
  ui_->fileText->setStyleSheet("QLineEdit {color: white; background: palette(highlight); }");
}

void FileSelectorWidget::setCustomFileFilter(const QString& filter)
{
  customFileFilter_ = filter;
}

QString FileSelectorWidget::customFileFilter() const
{
  return customFileFilter_;
}

QString FileSelectorWidget::filterOptions2QString_(FileSelectorWidget::FilterOptions option) const
{
  switch (option)
  {
  case FileSelectorWidget::ALL_SIMDIS_FILE_PATTERNS:
    return QString::fromStdString(simCore::ALL_SIMDIS_FILE_PATTERNS);
  case FileSelectorWidget::ALL_SIMDIS_ASCII_FILE_PATTERNS:
    return QString::fromStdString(simCore::ALL_SIMDIS_ASCII_FILE_PATTERNS);
  case FileSelectorWidget::ALL_SIMDIS_TEXTURE_FILE_PATTERNS:
    return QString::fromStdString(simCore::ALL_SIMDIS_TEXTURE_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_ASI_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_ASI_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_FCT_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_FCT_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_ZIP_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_ZIP_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_PREFS_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_PREFS_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_RULE_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_RULE_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_VIEW_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_VIEW_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_BOOKMARK_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_BOOKMARK_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_BOOKMARK_SAVE_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_BOOKMARK_SAVE_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_IMAGERY_CONFIG_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_IMAGERY_CONFIG_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_RASTER_DB_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_RASTER_DB_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_MODEL_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_MODEL_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_IMAGE_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_IMAGE_FILE_PATTERNS);
  case FileSelectorWidget::MEDIA_FILE_PATTERNS:
    return QString::fromStdString(simCore::MEDIA_FILE_PATTERNS);
  case FileSelectorWidget::SIMDIS_MEDIA_FILE_PATTERNS:
    return QString::fromStdString(simCore::SIMDIS_MEDIA_FILE_PATTERNS);
  case FileSelectorWidget::ANTENNA_FILE_PATTERNS:
    return QString::fromStdString(simCore::ANTENNA_FILE_PATTERNS);
  case FileSelectorWidget::GOG_FILE_PATTERNS:
    return QString::fromStdString(simCore::GOG_FILE_PATTERNS);
  case FileSelectorWidget::RELATIVE_GOG_FILE_PATTERNS:
    return QString::fromStdString(simCore::RELATIVE_GOG_FILE_PATTERNS);
  case FileSelectorWidget::GOG_FILE_SAVE_PATTERNS:
    return QString::fromStdString(simCore::GOG_FILE_SAVE_PATTERNS);
  case FileSelectorWidget::POINT_MAP_FILE_PATTERNS:
    return QString::fromStdString(simCore::POINT_MAP_FILE_PATTERNS);
  case FileSelectorWidget::WVS_FILE_PATTERNS:
    return QString::fromStdString(simCore::WVS_FILE_PATTERNS);
  case FileSelectorWidget::RFPROP_CONFIG_FILE_PATTERNS:
    return QString::fromStdString(simCore::RFPROP_CONFIG_FILE_PATTERNS);
  case FileSelectorWidget::AREPS_CONFIG_FILE_PATTERNS:
    return QString::fromStdString(simCore::AREPS_CONFIG_FILE_PATTERNS);
  case FileSelectorWidget::APM_CONFIG_FILE_PATTERNS:
    return QString::fromStdString(simCore::APM_CONFIG_FILE_PATTERNS);
  case FileSelectorWidget::RCS_CONFIG_FILE_PATTERNS:
    return QString::fromStdString(simCore::RCS_CONFIG_FILE_PATTERNS);
  case FileSelectorWidget::HDF5_FILE_PATTERNS:
    return QString::fromStdString(simCore::HDF5_FILE_PATTERNS);
  case FileSelectorWidget::TRACKSTATUS_REPORT_FILE_PATTERNS:
    return QString::fromStdString(simCore::TRACKSTATUS_REPORT_FILE_PATTERNS);
  case FileSelectorWidget::GDAL_IMAGE_LAYER_FILE_PATTERNS:
    return QString::fromStdString(simCore::GDAL_IMAGE_LAYER_FILE_PATTERNS);
  case FileSelectorWidget::MBTILES_IMAGE_LAYER_FILE_PATTERNS:
    return QString::fromStdString(simCore::MBTILES_IMAGE_LAYER_FILE_PATTERNS);
  case FileSelectorWidget::XML_FILE_PATTERNS:
    return QString::fromStdString(simCore::XML_FILE_PATTERNS);
  case FileSelectorWidget::CUSTOM_USERDEFINED_FILTER:
    return customFileFilter().replace("\\n", "\n");
  }

  assert(0);
  return tr("All Files (*)");
}

bool FileSelectorWidget::readOnlyLineEdit() const
{
  return ui_->fileText->isReadOnly();
}

void FileSelectorWidget::setReadOnlyLineEdit(bool readOnly)
{
  ui_->fileText->setReadOnly(readOnly);
}

void FileSelectorWidget::setShowClearFilename(bool showClearFilenameControl)
{
  showClearButton_ = showClearFilenameControl;
  // only show the clear button if there is something to clear
  ui_->clearButton->setVisible(showClearButton_ && !filename_.isEmpty());
}

bool FileSelectorWidget::showClearFilename() const
{
  return showClearButton_;
}

}
