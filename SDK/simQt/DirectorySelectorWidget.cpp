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
#include <cassert>
#include <QDesktopServices>
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
    labelWidget_(nullptr),
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
    if (labelWidget_ == nullptr)
    {
      labelWidget_ = new QLabel(label_);
    }
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

void DirectorySelectorWidget::setShowOpenDirectoryButton(bool show)
{
  if (showOpenDirectoryButton_ == show)
    return;

  showOpenDirectoryButton_ = show;

  if (showOpenDirectoryButton_)
  {
    assert(openDirectoryButton_ == nullptr); // Button should be NULL
    openDirectoryButton_ = new QPushButton;
    openDirectoryButton_->setIcon(QIcon(":/simQt/images/Folder 1 Forward.png"));
    ui_->horizontalLayout->addWidget(openDirectoryButton_);
    connect(openDirectoryButton_, SIGNAL(clicked(bool)), this, SLOT(openDirectory_()));
  }
  else
  {
    assert(openDirectoryButton_ != nullptr); // Button shouldn't be NULL
    if (openDirectoryButton_)
      ui_->horizontalLayout->removeWidget(openDirectoryButton_);
    delete openDirectoryButton_;
    openDirectoryButton_ = nullptr;
  }
}

bool DirectorySelectorWidget::showOpenDirectoryButton() const
{
  return showOpenDirectoryButton_;
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
    Q_EMIT directoryChanged(osDir);
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

void DirectorySelectorWidget::openDirectory_()
{
  QDesktopServices::openUrl(QUrl::fromLocalFile(ui_->directoryText->text()));
}


}
