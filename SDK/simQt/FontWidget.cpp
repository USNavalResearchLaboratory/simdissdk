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
 *               EW Modeling and Simulation, Code 5770
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * For more information please send email to simdis@enews.nrl.navy.mil
 *
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * U.S. Naval Research Laboratory.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 *
 */
#include <cassert>
#include <QDir>
#include <QRawFont>
#include <QComboBox>
#include <QSpinBox>
#include "simNotify/Notify.h"
#include "simQt/QtFormatting.h"
#include "simQt/ColorWidget.h"
#include "ui_FontWidget.h"
#include "FontWidget.h"

namespace simQt {

FontWidget::FontWidget(QWidget* parent)
  : QWidget(parent),
    fontDir_(new QDir()),
    ui_(nullptr),
    useFriendlyFontName_(true)
{

  ui_ = new Ui_FontWidget();
  ui_->setupUi(this);
  connect(ui_->fontNameComboBox, SIGNAL(currentIndexChanged(QString)),
    this, SLOT(fontNameChanged_(QString)));
  connect(ui_->fontSizeSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(fontSizeChanged(int)));
  connect(ui_->fontColorWidget, SIGNAL(colorChanged(QColor)), this, SIGNAL(fontColorChanged(QColor)));

  // set tool tips
  ui_->fontNameComboBox->setToolTip(simQt::formatTooltip(tr("Fonts"), tr("Provides a list of available system fonts.")));
  ui_->fontSizeSpinBox->setToolTip(simQt::formatTooltip(tr("Size"), tr("Controls the size of the selected font.")));
  ui_->fontColorWidget->setToolTip(simQt::formatTooltip(tr("Color"), tr("Controls the color of the selected font.")));
}

FontWidget::~FontWidget()
{
  delete fontDir_;
  delete ui_;
}

QString FontWidget::fontFile()
{
  QHash<QString, QFileInfo>::const_iterator iter = fontFiles_.find(ui_->fontNameComboBox->currentText());
  if (iter != fontFiles_.end())
    return iter->fileName();
  else
    assert(0);
  return "";
}

QString FontWidget::fontFullPathFile()
{
  QHash<QString, QFileInfo>::const_iterator iter = fontFiles_.find(ui_->fontNameComboBox->currentText());
  if (iter != fontFiles_.end())
    return iter->absoluteFilePath();
  else
    assert(0);
  return "";
}

QColor FontWidget::fontColor() const
{
  return ui_->fontColorWidget->color();
}

int FontWidget::fontSize() const
{
  return ui_->fontSizeSpinBox->value();
}

bool FontWidget::showFontColor() const
{
  return ui_->fontColorWidget->isVisible();
}

bool FontWidget::showFontSize() const
{
  return ui_->fontSizeSpinBox->isVisible();
}

void FontWidget::setFontDir(const QString& fontDir)
{
  fontDir_->setPath(fontDir);
  ui_->fontNameComboBox->clear();
  fontFiles_.clear();
  // search for all the installed fonts
  QStringList fontFilters;
  fontFilters << "*.ttf"; // only look at .ttf files
  QFileInfoList fonts = fontDir_->entryInfoList(fontFilters, QDir::Files);

#ifdef WIN32
  // On some Windows systems, specific WinAPI calls can fail when trying to
  // use SIMDIS fonts. Test to make sure we are able to use SIMDIS fonts. SDK-119
  const QString& absPath = fonts.front().absoluteFilePath();
  if (AddFontResourceEx(absPath.toStdString().c_str(), FR_PRIVATE, 0) == 0)
    useFriendlyFontName_ = false;
  // Make sure to remove added font resource
  RemoveFontResourceEx(absPath.toStdString().c_str(), FR_PRIVATE, 0);
#endif

  for (auto it = fonts.begin(); it != fonts.end(); ++it)
  {
    const QFileInfo& fontFile = *it;
    // Only use getFriendlyFontName_() and QRawFont when we're able
    // to use SIMDIS fonts. Otherwise, just use the font file name. SDK-119
    QString fontName = fontFile.fileName();
    if (useFriendlyFontName_)
      fontName = getFriendlyFontName_(fontFile.absoluteFilePath());
    if (fontName.isEmpty())
      continue;
    // keep our font names in the hash table, don't want duplicates
    if (fontFiles_.find(fontName) == fontFiles_.end())
    {
      fontFiles_.insert(fontName, fontFile);
      ui_->fontNameComboBox->addItem(fontName);
    }
  }
}

void FontWidget::setFontNameEnabled(bool enabled)
{
  ui_->fontNameComboBox->setEnabled(enabled);
}

bool FontWidget::customFontsAvailable() const
{
  return useFriendlyFontName_;
}

void FontWidget::setFontFile(const QString& fontFile)
{
  // use QRawFont to convert the file name to a font family name
  QString fontFullPath = fontDir_->absoluteFilePath(fontFile);
  if (fontFullPath.isEmpty() || fontFile.isEmpty())
    return;
  // Only use getFriendlyFontName_() and QRawFont when we're able
  // to use SIMDIS fonts. Otherwise, just use the font file name. SDK-119
  QString fontName = fontFile;
  if (useFriendlyFontName_)
    fontName = getFriendlyFontName_(fontFullPath);
  int index = ui_->fontNameComboBox->findText(fontName, Qt::MatchFixedString);
  if (index >= 0) // only change if valid font was found
    ui_->fontNameComboBox->setCurrentIndex(index);
  else
  {
    SIM_WARN << "WARNING: FontWidget: could not find font file: " << fontFile.toStdString() << "\n";
  }
}

void FontWidget::setFontColor(const QColor& fontColor)
{
  ui_->fontColorWidget->setColor(fontColor);
}

void FontWidget::setFontSize(int fontSize)
{
  // Check for equality to avoid trampling user edits with programmatic updates
  if (ui_->fontSizeSpinBox->value() != fontSize)
    ui_->fontSizeSpinBox->setValue(fontSize);
}

void FontWidget::setShowFontColor(bool showColor)
{
  if (showColor)
    ui_->fontColorWidget->show();
  else
    ui_->fontColorWidget->hide();
}

void FontWidget::setShowFontSize(bool showSize)
{
  if (showSize)
    ui_->fontSizeSpinBox->show();
  else
    ui_->fontSizeSpinBox->hide();
}


void FontWidget::fontNameChanged_(const QString& fontName)
{
  // we should have an entry in our hash table for this value, otherwise there is a problem
  QHash<QString, QFileInfo>::const_iterator iter = fontFiles_.find(fontName);
  if (iter != fontFiles_.end())
    emit(fontFileChanged(iter->fileName()));
  else
    assert(0);
}

QString FontWidget::getFriendlyFontName_(const QString& absolutePath) const
{
  // use QRawFont to convert the file name to a font family name
  QRawFont rawFont(absolutePath, 14.0);
  if (!rawFont.isValid())
    return QString();

  // Pull out the weight string
  QString weightString;
  switch (rawFont.weight())
  {
  case QFont::Light:
    weightString = " Light";
    break;
  case QFont::DemiBold:
    weightString = " DemiBold";
    break;
  case QFont::Bold:
    weightString = " Bold";
    break;
  case QFont::Black:
    weightString = " Black";
    break;
  case QFont::Normal:
    break;
  }

  // Pull out the style (italic) string
  QString slantString;
  switch (rawFont.style())
  {
  case QFont::StyleItalic:
    slantString = " Italic";
    break;
  case QFont::StyleOblique:
    slantString = " Oblique";
  case QFont::StyleNormal:
    break;
  }
  return QString("%1%2%3").arg(rawFont.familyName(), weightString, slantString);
}

}
