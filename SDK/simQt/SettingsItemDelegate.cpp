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
#include <cassert>
#include <limits>
#include <QApplication>
#include <QLineEdit>
#include <QPainter>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QColorDialog>
#include <QComboBox>
#include <QFileDialog>
#include <limits.h>
#include "simNotify/Notify.h"
#include "simQt/ColorButton.h"
#include "simQt/DirectorySelectorWidget.h"
#include "simQt/FileSelectorWidget.h"
#include "simQt/FontWidget.h"
#include "simQt/SettingsModel.h"
#include "simQt/SettingsItemDelegate.h"

namespace simQt
{

SettingsColorItemDelegate::SettingsColorItemDelegate(QObject* parent)
 : QStyledItemDelegate(parent)
{
}

void SettingsColorItemDelegate::paintItemBackground_(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItemV4 opt = option;
  initStyleOption(&opt, index);
  opt.text.clear();
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
}

void SettingsColorItemDelegate::paintColoredSquare_(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // Calculate the rectangle size for the drawn box
  QRect rect = option.rect.adjusted(4, 2, 0, -2);
  QSize size = sizeHint(option, index);
  rect.setWidth(size.height() - 4);
  QColor qColor = getColor_(index);

  simQt::ColorButton::paintColoredSquare(painter, rect, qColor);
}

QColor SettingsColorItemDelegate::getColor_(const QModelIndex& index, int role) const
{
  const QVariant variantValue = index.model()->data(index, Qt::DisplayRole);
  if (variantValue.isValid() && variantValue.canConvert<QRgb>())
    return QColor::fromRgba(variantValue.value<QRgb>());
  return QColor(0, 0, 0, 0);
}

void SettingsColorItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  painter->save();
  paintItemBackground_(painter, option, index);
  paintColoredSquare_(painter, option, index);
  painter->restore();
}

QWidget* SettingsColorItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QColorDialog* dialog = new QColorDialog(parent);
  dialog->setOptions(QColorDialog::ShowAlphaChannel);
  dialog->setModal(true);
  connect(dialog, SIGNAL(accepted()), this, SLOT(commitAndCloseEditor_()));
  connect(dialog, SIGNAL(rejected()), this, SLOT(cancelEditor_()));
  return dialog;
}

void SettingsColorItemDelegate::commitAndCloseEditor_()
{
  QColorDialog* editor = static_cast<QColorDialog*>(sender());
  emit commitData(editor);
  emit closeEditor(editor);
}

void SettingsColorItemDelegate::cancelEditor_()
{
  QColorDialog* editor = static_cast<QColorDialog*>(sender());
  emit closeEditor(editor);
}

void SettingsColorItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QColorDialog* dialog = static_cast<QColorDialog*>(editor);
  // Set color and title
  dialog->setCurrentColor(getColor_(index, Qt::EditRole));
  QString fqn = index.model()->data(index, SettingsModel::FullyQualifiedNameRole).toString();
  dialog->setWindowTitle(fqn);
}

void SettingsColorItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QColorDialog* dialog = static_cast<QColorDialog*>(editor);
  model->setData(index, dialog->currentColor().rgba(), Qt::EditRole);
}

void SettingsColorItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QColorDialog* dialog = static_cast<QColorDialog*>(editor);
  dialog->move(option.rect.topLeft());
}

/////////////////////////////////////////////////////////////////////////

SettingsDirectorySelectorDelegate::SettingsDirectorySelectorDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

const QString SettingsDirectorySelectorDelegate::tempKeyName_ = "Private/DirectorySelectorDelegate TempDir";

SettingsDirectorySelectorDelegate::~SettingsDirectorySelectorDelegate()
{
  QSettings settings;
  settings.remove(tempKeyName_);
}

QWidget* SettingsDirectorySelectorDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  DirectorySelectorWidget* directorySelector = new DirectorySelectorWidget(parent);
  // Set globally acceptable values for the widget
  directorySelector->setIncludeLabel(false);
  directorySelector->setFocusPolicy(Qt::StrongFocus);
  connect(directorySelector, SIGNAL(directoryChanged(const QString&)), this, SLOT(commitEditor_()));
  return directorySelector;
}

void SettingsDirectorySelectorDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  DirectorySelectorWidget* selector = static_cast<DirectorySelectorWidget*>(editor);

  QString data = index.model()->data(index, Qt::EditRole).toString();
  QSettings settings;
  settings.setValue(tempKeyName_, data);
  QString fqn = index.model()->data(index, SettingsModel::FullyQualifiedNameRole).toString();
  selector->setWindowTitle("Select Directory for " + fqn);
  selector->setRegistryKey(tempKeyName_);
  selector->setDirectory(data);
}

void SettingsDirectorySelectorDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  DirectorySelectorWidget* selector = static_cast<DirectorySelectorWidget*>(editor);
  model->setData(index, selector->directory(), Qt::EditRole);
  QSettings settings;
  settings.remove(tempKeyName_);
}

void SettingsDirectorySelectorDelegate::commitEditor_()
{
  DirectorySelectorWidget* selector = static_cast<DirectorySelectorWidget*>(sender());
  emit commitData(selector);
}

void SettingsDirectorySelectorDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  editor->setGeometry(option.rect);
}

/////////////////////////////////////////////////////////////////////////

SettingsIntegerSpinBoxDelegate::SettingsIntegerSpinBoxDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* SettingsIntegerSpinBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QSpinBox* spinBox = new QSpinBox(parent);
  // Disable keyboard tracking, thereby preventing too many signals while editing text by keyboard
  spinBox->setKeyboardTracking(false);
  return spinBox;
}

void SettingsIntegerSpinBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QSpinBox* spinBox = static_cast<QSpinBox*>(editor);

  // Extract the minimum and maximum from the meta data in the data model
  const QVariant vMetaData = index.model()->data(index, SettingsModel::MetaDataRole);
  int min = std::numeric_limits<int>::min();
  int max = std::numeric_limits<int>::max();
  if (vMetaData.isValid() && vMetaData.canConvert<simQt::Settings::MetaData>())
  {
    Settings::MetaData metaData = vMetaData.value<simQt::Settings::MetaData>();
    if (metaData.minValue().isValid())
      min = metaData.minValue().toInt();
    if (metaData.maxValue().isValid())
      max = metaData.maxValue().toInt();
  }

  // Set the data, minimum, and maximum values
  spinBox->setMinimum(min);
  spinBox->setMaximum(max);
  spinBox->setValue(index.model()->data(index, Qt::EditRole).toInt());
}

void SettingsIntegerSpinBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
  spinBox->interpretText();
  model->setData(index, spinBox->value(), Qt::EditRole);
}

/////////////////////////////////////////////////////////////////////////

SettingsDoubleSpinBoxDelegate::SettingsDoubleSpinBoxDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* SettingsDoubleSpinBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
  // Disable keyboard tracking, thereby preventing too many signals while editing text by keyboard
  spinBox->setKeyboardTracking(false);
  return spinBox;
}

void SettingsDoubleSpinBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);

  // Extract the minimum and maximum from the meta data in the data model
  const QVariant vMetaData = index.model()->data(index, SettingsModel::MetaDataRole);
  double min = -std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::max();
  int numDecimals = 2;
  if (vMetaData.isValid() && vMetaData.canConvert<simQt::Settings::MetaData>())
  {
    Settings::MetaData metaData = vMetaData.value<simQt::Settings::MetaData>();
    if (metaData.minValue().isValid())
      min = metaData.minValue().toDouble();
    if (metaData.maxValue().isValid())
      max = metaData.maxValue().toDouble();
    numDecimals = metaData.numDecimals();
  }

  // Set the data, minimum, and maximum values
  spinBox->setMinimum(min);
  spinBox->setMaximum(max);
  spinBox->setDecimals(numDecimals);
  spinBox->setValue(index.model()->data(index, Qt::EditRole).toDouble());
}

void SettingsDoubleSpinBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
  spinBox->interpretText();
  model->setData(index, spinBox->value(), Qt::EditRole);
}

/////////////////////////////////////////////////////////////////////////

SettingsFileSelectorDelegate::SettingsFileSelectorDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

const QString SettingsFileSelectorDelegate::tempKeyName_ = "Private/FileSelectorDelegate TempFile";

SettingsFileSelectorDelegate::~SettingsFileSelectorDelegate()
{
  QSettings settings;
  settings.remove(tempKeyName_);
}

QWidget* SettingsFileSelectorDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  FileSelectorWidget* fileSelector = new FileSelectorWidget(parent);
  // Set globally acceptable values for the widget
  fileSelector->setIncludeLabel(false);
  fileSelector->setFilterOption(simQt::FileSelectorWidget::CUSTOM_USERDEFINED_FILTER);
  fileSelector->setFileOptions(simQt::FileSelectorWidget::FileLoad);
  fileSelector->setFocusPolicy(Qt::StrongFocus);
  connect(fileSelector, SIGNAL(filenameChanged(const QString&)), this, SLOT(commitEditor_()));
  return fileSelector;
}

void SettingsFileSelectorDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  FileSelectorWidget* fileSelector = static_cast<FileSelectorWidget*>(editor);
  QString filter = "All Files (*)";
  const QVariant vMetaData = index.model()->data(index, SettingsModel::MetaDataRole);
  if (vMetaData.isValid() && vMetaData.canConvert<simQt::Settings::MetaData>())
  {
    Settings::MetaData metaData = vMetaData.value<simQt::Settings::MetaData>();
    // Do not use empty filters
    if (metaData.filenameFilter().isValid() && !metaData.filenameFilter().toString().isEmpty())
      filter = metaData.filenameFilter().toString();
  }

  //--- Initialize the widget
  // pull the name of the setting to modify
  const QString fqn = index.model()->data(index, SettingsModel::FullyQualifiedNameRole).toString();

  // the file selector uses a setting to store the chosen file, we'll need a temp setting to shadow it

  // pull the current filename, and push it into our temp key
  const QString data = index.model()->data(index, Qt::EditRole).toString();
  QSettings settings;
  settings.setValue(tempKeyName_, data);

  // configure the file selector
  fileSelector->setWindowTitle(QString("Select File For ") + fqn);
  fileSelector->setRegistryKey(tempKeyName_);
  fileSelector->setCustomFileFilter(filter);
  fileSelector->setFilename(data);
}

void SettingsFileSelectorDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  FileSelectorWidget* fileSelector = static_cast<FileSelectorWidget*>(editor);
  model->setData(index, fileSelector->filename(), Qt::EditRole);
  QSettings settings;
  settings.remove(tempKeyName_);
}

void SettingsFileSelectorDelegate::commitEditor_()
{
  FileSelectorWidget* editor = static_cast<FileSelectorWidget*>(sender());
  emit commitData(editor);
}

void SettingsFileSelectorDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  editor->setGeometry(option.rect);
}

/////////////////////////////////////////////////////////////////////////
SettingsHexEditValidator::SettingsHexEditValidator(unsigned int min, unsigned int max, QObject* parent)
  :QValidator(parent),
  min_(min),
  max_(max)
{}

SettingsHexEditValidator::~SettingsHexEditValidator() {}
void SettingsHexEditValidator::fixup(QString& input) const
{
  // nop
}

QValidator::State SettingsHexEditValidator::validate(QString& input, int& pos) const
{
  bool ok;
  unsigned int val = input.toUInt(&ok, 16);
  if (!ok)
    return QValidator::Invalid;
  if (val >= min_ && val <= max_)
    return QValidator::Acceptable;
  return QValidator::Invalid;
}

SettingsHexEditDelegate::SettingsHexEditDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* SettingsHexEditDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QLineEdit* lineEdit = new QLineEdit(parent);

  int numChars = maxCharCount_(index);

  // define input mask to ensure only hex values are entered
  QString inputMask;
  QString placeHolder;
  for (int i = 0; i < numChars; ++i)
  {
    inputMask += "h";
    placeHolder += "F";
  }
  lineEdit->setInputMask(inputMask);
  lineEdit->setPlaceholderText(placeHolder);

  // Extract the maximum and minimum from the meta data in the data model
  const QVariant vMetaData = index.model()->data(index, SettingsModel::MetaDataRole);
  unsigned int max = std::numeric_limits<unsigned int>::max();
  unsigned int min = std::numeric_limits<unsigned int>::min();
  if (vMetaData.isValid() && vMetaData.canConvert<simQt::Settings::MetaData>())
  {
    Settings::MetaData metaData = vMetaData.value<simQt::Settings::MetaData>();
    if (metaData.maxValue().isValid())
      max = metaData.maxValue().toUInt();
    if (metaData.minValue().isValid())
      min = metaData.minValue().toUInt();
  }
  // set the validator to apply the min and max
  SettingsHexEditValidator* validator = new SettingsHexEditValidator(min, max, lineEdit);
  lineEdit->setValidator(validator);
  return lineEdit;
}

void SettingsHexEditDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
  // convert the value to a hex string, ensure upper case
  int numChars = maxCharCount_(index);
  lineEdit->setText(QString("%1").arg(index.model()->data(index, Qt::EditRole).toUInt(), numChars, 16, QChar('0')).toUpper());
}

void SettingsHexEditDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
  bool ok;
  // convert the hex string to an unsigned int
  unsigned int value = lineEdit->text().toUInt(&ok, 16);
  if (!ok)
  {
    assert(0); // error converting string to hex, somehow got invalid value into the editor
    return;
  }
  model->setData(index, value, Qt::EditRole);
}


void SettingsHexEditDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItemV4 opt = option;
  initStyleOption(&opt, index);

  // Convert the value to a hex string, converting to upper case and appending '0x'
  int numChars = maxCharCount_(index);
  opt.text = QString("0x%1").arg(QString("%1").arg(index.model()->data(index, Qt::DisplayRole).toUInt(), numChars, 16, QChar('0')).toUpper());

  // Paint with updated text
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
}

int SettingsHexEditDelegate::maxCharCount_(const QModelIndex& index) const
{
  // Extract the maximum from the meta data in the data model
  const QVariant vMetaData = index.model()->data(index, SettingsModel::MetaDataRole);
  unsigned int max = std::numeric_limits<unsigned int>::max();
  if (vMetaData.isValid() && vMetaData.canConvert<simQt::Settings::MetaData>())
  {
    Settings::MetaData metaData = vMetaData.value<simQt::Settings::MetaData>();
    if (metaData.maxValue().isValid())
      max = metaData.maxValue().toUInt();
  }
  // convert max to string for char count
  QString hexString = QString("%1").arg(max, 0, 16);

  return hexString.size();
}

/////////////////////////////////////////////////////////////////////////

SettingsEnumerationDelegate::SettingsEnumerationDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* SettingsEnumerationDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QComboBox* combo = new QComboBox(parent);
  // Set globally acceptable values for the widget
  combo->setEditable(false);
  return combo;
}

void SettingsEnumerationDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QComboBox* combo = static_cast<QComboBox*>(editor);
  combo->clear();

  // Get the value we're supposed to show
  const QVariant editData = index.model()->data(index, Qt::EditRole);
  int valueToShow = editData.toInt();
  int newComboIndex = -1;

  const QVariant vMetaData = index.model()->data(index, SettingsModel::MetaDataRole);
  // Assertion failure means we can't convert, which means the ENUMERATION flag was wrong
  assert(vMetaData.isValid() && vMetaData.canConvert<simQt::Settings::MetaData>());
  if (vMetaData.isValid() && vMetaData.canConvert<simQt::Settings::MetaData>())
  {
    Settings::MetaData metaData = vMetaData.value<simQt::Settings::MetaData>();
    QMapIterator<int, QString> iter(metaData.enumValues());
    while (iter.hasNext())
    {
      iter.next();
      combo->addItem(iter.value(), iter.key());
      if (iter.key() == valueToShow)
        newComboIndex = combo->count() - 1;
    }
  }

  // Set the current value
  if (newComboIndex >= 0)
    combo->setCurrentIndex(newComboIndex);
}

void SettingsEnumerationDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QComboBox* combo = static_cast<QComboBox*>(editor);
  int value = combo->itemData(combo->currentIndex(), Qt::UserRole).toInt();
  model->setData(index, value, Qt::EditRole);
}

void SettingsEnumerationDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  editor->setGeometry(option.rect);
}

void SettingsEnumerationDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItemV4 opt = option;
  initStyleOption(&opt, index);

  // Pull out meta data and display data
  opt.text += " (Unknown Value)";
  const QVariant displayData = index.model()->data(index, Qt::DisplayRole);
  const QVariant vMetaData = index.model()->data(index, SettingsModel::MetaDataRole);
  // Assertion failure means we can't convert, which means the ENUMERATION flag was wrong
  if (displayData.isValid() && vMetaData.isValid() && vMetaData.canConvert<simQt::Settings::MetaData>())
  {
    Settings::MetaData metaData = vMetaData.value<simQt::Settings::MetaData>();
    const QMap<int, QString> enumMap = metaData.enumValues();
    QMap<int, QString>::const_iterator i = enumMap.find(displayData.toInt());
    if (i != enumMap.end())
    {
      opt.text = i.value();
    }
  }

  // Paint with updated text
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
}

/////////////////////////////////////////////////////////////////////////

SettingsFontSelectorDelegate::SettingsFontSelectorDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

SettingsFontSelectorDelegate::~SettingsFontSelectorDelegate()
{
}

QWidget* SettingsFontSelectorDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  FontWidget* fontSelector = new FontWidget(parent);

  // search for font in the SIMDIS_FONTPATH directory
  std::string fontDir;
  const char* tempString = getenv("SIMDIS_FONTPATH");
  if (tempString) // only pass directory if we found one, otherwise it will be empty string
     fontDir = tempString;
  else
  {
    SIM_ERROR << "Could not set font directory.  Check that the environment variable SIMDIS_FONTPATH has been set\n";
  }
  fontSelector->setFontDir(QString::fromStdString(fontDir));
  fontSelector->setFocusPolicy(Qt::StrongFocus);
  fontSelector->setShowFontSize(false); // we don't provide size change option in setting delegate
  fontSelector->setShowFontColor(false); // we don't provide color change option in setting delegate
  connect(fontSelector, SIGNAL(fontFileChanged(const QString&)), this, SLOT(commitEditor_()));
  return fontSelector;
}

void SettingsFontSelectorDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  FontWidget* fontSelector = static_cast<FontWidget*>(editor);

  // get the current font file name from the model
  const QString data = index.model()->data(index, Qt::EditRole).toString();

  // update the font widget
  fontSelector->setFontFile(data);
}

void SettingsFontSelectorDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  // set the font file name in the model
  FontWidget* fontSelector = static_cast<FontWidget*>(editor);
  model->setData(index, fontSelector->fontFile());
}

void SettingsFontSelectorDelegate::commitEditor_()
{
  FontWidget* editor = static_cast<FontWidget*>(sender());
  emit commitData(editor);
}

void SettingsFontSelectorDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  editor->setGeometry(option.rect);
}

/////////////////////////////////////////////////////////////////////////
SettingsItemDelegate::SettingsItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
  // Connect the delegate signals to our signals so the view gets the notices
  connect(&colorDelegate_, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
  connect(&colorDelegate_, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
  connect(&integerDelegate_, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
  connect(&integerDelegate_, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
  connect(&doubleDelegate_, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
  connect(&doubleDelegate_, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
  connect(&filenameDelegate_, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
  connect(&filenameDelegate_, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
  connect(&directoryDelegate_, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
  connect(&directoryDelegate_, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
  connect(&enumerationDelegate_, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
  connect(&enumerationDelegate_, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
  connect(&fontDelegate_, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
  connect(&fontDelegate_, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
  connect(&hexDelegate_, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
  connect(&hexDelegate_, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
}

SettingsItemDelegate::~SettingsItemDelegate()
{
}

void SettingsItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  const QStyledItemDelegate* itemDelegate = findDelegate_(index);
  if (itemDelegate)
    itemDelegate->paint(painter, option, index);
  else
    QStyledItemDelegate::paint(painter, option, index);
}

QWidget* SettingsItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  const QStyledItemDelegate* itemDelegate = findDelegate_(index);
  if (itemDelegate)
    return itemDelegate->createEditor(parent, option, index);
  return QStyledItemDelegate::createEditor(parent, option, index);
}

void SettingsItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  const QStyledItemDelegate* itemDelegate = findDelegate_(index);
  if (itemDelegate)
    itemDelegate->setEditorData(editor, index);
  else
    QStyledItemDelegate::setEditorData(editor, index);
}

void SettingsItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  const QStyledItemDelegate* itemDelegate = findDelegate_(index);
  if (itemDelegate)
    itemDelegate->setModelData(editor, model, index);
  else
    QStyledItemDelegate::setModelData(editor, model, index);
}

void SettingsItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  const QStyledItemDelegate* itemDelegate = findDelegate_(index);
  if (itemDelegate)
    itemDelegate->updateEditorGeometry(editor, option, index);
  else
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

bool SettingsItemDelegate::eventFilter(QObject* object, QEvent* event)
{
  QWidget *editor = qobject_cast<QWidget*>(object);
  if (!editor || !event)
    return false;

  // Do not close the editor if it's a window and it hides.  This prevents a bug
  // where the window gets closed and the data is automatically applied.  This
  // affects delegates where the pop up can be canceled (like Color) -- the cancellation
  // never takes effects, because the dialog hides first.
  if (event->type() == QEvent::Hide && editor->isWindow())
    return false;
  return QStyledItemDelegate::eventFilter(object, event);
}

const QStyledItemDelegate* SettingsItemDelegate::findDelegate_(const QModelIndex& index) const
{
  // Get the QVariant representing the meta data; only continue if the data is valid
  const QVariant qvMetaData = index.model()->data(index, SettingsModel::MetaDataRole);
  if (qvMetaData.isValid() && qvMetaData.canConvert<Settings::MetaData>())
  {
    // Return the appropriate delegate based on metadata type value
    Settings::MetaData metaData = qvMetaData.value<Settings::MetaData>();
    switch (metaData.type())
    {
    case simQt::Settings::COLOR:
      return &colorDelegate_;
    case simQt::Settings::INTEGER:
      return &integerDelegate_;
    case simQt::Settings::DOUBLE:
      return &doubleDelegate_;
    case simQt::Settings::FILENAME:
      return &filenameDelegate_;
    case simQt::Settings::DIRECTORY:
      return &directoryDelegate_;
    case simQt::Settings::ENUMERATION:
      return &enumerationDelegate_;
    case simQt::Settings::FONT:
      return &fontDelegate_;
    case simQt::Settings::HEX:
      return &hexDelegate_;
    default:
      break; // All other cases handled through default painting/editing
    }
  }
  return NULL;
}

}
