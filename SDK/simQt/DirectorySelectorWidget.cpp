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
#include <cassert>
#include <QLabel>
#include "simCore/String/FilePatterns.h"
#include "simQt/FileDialog.h"
#include "simQt/ResourceInitializer.h"
#include "simQt/DirectorySelectorWidget.h"
#include "ui_DirectorySelectorWidget.h"

namespace simQt {

DirectorySelectorWidget::DirectorySelectorWidget(QWidget* parent)
  : QWidget(parent),
    registryKey_("Private/directory"),
    labelWidget_(NULL),
    includeLabel_(false),
    label_(tr("Directory")),
    browserTitle_(tr("Select Directory"))
{
  ResourceInitializer::initialize();  // Needs to be here so that Qt Designer works.

  ui_ = new Ui_DirectorySelectorWidget;
  ui_->setupUi(this);
  connect(ui_->directoryButton, SIGNAL(clicked()), this, SLOT(loadButton_()));
  ui_->directoryButton->setToolTip(tr("Display File Browser to select a directory."));
  connect(ui_->directoryText, SIGNAL(textEdited(const QString&)), this, SLOT(textEdited_()));
  connect(ui_->directoryText, SIGNAL(editingFinished()), this, SLOT(editingFinished_()));

#ifndef NDEBUG
  ui_->directoryText->setReadOnly(false);  // Only allows developers to type in a directory name; users must use the file browser
#else
  ui_->directoryText->setReadOnly(true);  // Normal use - users must use the file browser
#endif
  ui_->directoryText->installEventFilter(this);
}

DirectorySelectorWidget::~DirectorySelectorWidget()
{
  delete ui_;
}

void DirectorySelectorWidget::setRegistryKey(const QString &regKey)
{
  registryKey_ = regKey;
}

QString DirectorySelectorWidget::registryKey() const
{
  return registryKey_;
}

void DirectorySelectorWidget::setDefaultPath(const QString& defaultPath)
{
  defaultPath_ = defaultPath;
}

QString DirectorySelectorWidget::defaultPath() const
{
  return defaultPath_;
}

void DirectorySelectorWidget::setIncludeLabel(bool value)
{
  if (value == true)
  {
    if (labelWidget_ == NULL)
    {
      labelWidget_ = new QLabel(label_);
    }
    ui_->horizontalLayout->insertWidget(0, labelWidget_);
  }
  else
  {
    ui_->horizontalLayout->removeWidget(labelWidget_);
    delete labelWidget_;
    labelWidget_ = NULL;
  }

  includeLabel_ = value;
}

bool DirectorySelectorWidget::includeLabel() const
{
  return includeLabel_;
}

void DirectorySelectorWidget::setLabel(const QString &label)
{
  label_ = label;
  if (labelWidget_)
    labelWidget_->setText(label_);
}

QString DirectorySelectorWidget::label() const
{
  return label_;
}

void DirectorySelectorWidget::setBrowserTitle(const QString &title)
{
  browserTitle_ = title;
}

QString DirectorySelectorWidget::browserTitle() const
{
  return browserTitle_;
}


QString DirectorySelectorWidget::directory() const
{
  return ui_->directoryText->text();
}

void DirectorySelectorWidget::loadButton_()
{
  if (!defaultPath().isEmpty())
    simQt::FileDialog::setRegistryDir(registryKey_, defaultPath());
  QString dir = simQt::FileDialog::findDirectory(this, browserTitle_, registryKey_);

  if (!dir.isEmpty())
    setDirectory(dir);
}

void DirectorySelectorWidget::setDirectory(const QString& dir)
{
  QString osDir = QDir::toNativeSeparators(dir);
  if (osDir != ui_->directoryText->text())
  {
    ui_->directoryText->setText(osDir);
    emit directoryChanged(osDir);
  }
}

bool DirectorySelectorWidget::eventFilter(QObject* obj, QEvent* evt)
{
  if (obj == ui_->directoryText && ui_->directoryText->isEnabled())
  {
    if (evt->type() == QEvent::MouseButtonDblClick)
    {
      loadButton_();
      return true;
    }
  }
  return false;
}

// only used in DEBUG mode
void DirectorySelectorWidget::editingFinished_()
{
  ui_->directoryText->setStyleSheet("QLineEdit {background: palette(base); color: black;}");
  setDirectory(ui_->directoryText->text());
}

// only used in DEBUG mode
void DirectorySelectorWidget::textEdited_()
{
  ui_->directoryText->setStyleSheet("QLineEdit {color: white; background: palette(highlight); }");
}


}
