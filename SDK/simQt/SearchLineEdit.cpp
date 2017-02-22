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
#include <QTimer>
#include <QToolButton>
#include <QSize>
#include <QResizeEvent>
#include "simQt/SearchLineEdit.h"
#include "ui_SearchLineEdit.h"

namespace simQt
{

/** Buffer around the button for the clear, in pixels */
static const int ICON_SIZE_BUFFER = 3;

SearchLineEdit::SearchLineEdit(QWidget* parent)
  : QFrame(parent),
    clearButtonEnabled_(true),
    searchTimer_(NULL),
    ui_(new Ui_SearchLineEdit)
{
  ui_->setupUi(this);

  // Configure the timer
  searchTimer_ = new QTimer(this);
  searchTimer_->setSingleShot(true);
  searchTimer_->setInterval(500);
  connect(searchTimer_, SIGNAL(timeout()), this, SLOT(emitSearchRequested_()));

  // Create the clear button.  Not using QLineEdit's clearButtonEnabled() because of lack of Qt4 support
  const int buttonSize = sizeHint().height() - ICON_SIZE_BUFFER;
  clearButton_ = new QToolButton(ui_->searchText);
  clearButton_->setObjectName("searchLineEditClear");
  clearButton_->setIcon(QIcon(":/simQt/images/Close.png"));
  clearButton_->setCursor(Qt::ArrowCursor);
  clearButton_->setStyleSheet("QToolButton { border: none; padding: 2px}");
  clearButton_->setFixedSize(buttonSize, buttonSize);
  clearButton_->setToolTip(tr("Clear"));
  clearButton_->hide();

  // Adjust the padding on the line edit
  ui_->searchText->setStyleSheet(QString("QLineEdit { padding-right: %1px }").arg(buttonSize));

  // Configure the editing signals
  connect(ui_->searchText, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
  connect(ui_->searchText, SIGNAL(returnPressed()), this, SIGNAL(returnPressed()));
  connect(ui_->searchText, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
  connect(ui_->searchText, SIGNAL(textEdited(QString)), this, SIGNAL(textEdited(QString)));
  connect(ui_->searchText, SIGNAL(textChanged(QString)), searchTimer_, SLOT(start()));
  connect(clearButton_, SIGNAL(clicked()), this, SLOT(clear()));
  connect(ui_->searchText, SIGNAL(textChanged(QString)), this, SLOT(showOrHideClearButton_()));

  // Pressing enter should also stop the timer and emit the re-search command
  connect(ui_->searchText, SIGNAL(returnPressed()), searchTimer_, SLOT(stop()));
  connect(ui_->searchText, SIGNAL(returnPressed()), this, SLOT(emitSearchRequested_()));

  // Fix display of clear button (might be on if ui_ sets text, but likely hidden)
  showOrHideClearButton_();
}

SearchLineEdit::~SearchLineEdit()
{
  delete searchTimer_;
  searchTimer_ = NULL;
  delete ui_;
  ui_ = NULL;
}

QString SearchLineEdit::text() const
{
  return ui_->searchText->text();
}

QString SearchLineEdit::placeholderText() const
{
  return ui_->searchText->placeholderText();
}

const QPixmap* SearchLineEdit::searchPixmap() const
{
  return ui_->searchIcon->pixmap();
}

int SearchLineEdit::searchDelayInterval() const
{
  return searchTimer_->interval();
}

bool SearchLineEdit::clearButtonEnabled() const
{
  return clearButtonEnabled_;
}

bool SearchLineEdit::searchIconEnabled() const
{
  return ui_->searchIcon->isVisible();
}

QLineEdit* SearchLineEdit::lineEdit() const
{
  return ui_->searchText;
}

void SearchLineEdit::setText(const QString& text)
{
  ui_->searchText->setText(text);
}

void SearchLineEdit::clear()
{
  if (text().isEmpty())
    return;
  // Clear the text, and immediately send out a refresh on search
  ui_->searchText->clear();
  searchTimer_->stop();
  emit searchRequested("");
}

void SearchLineEdit::selectAll()
{
  ui_->searchText->selectAll();
}

void SearchLineEdit::setPlaceholderText(const QString& text)
{
  ui_->searchText->setPlaceholderText(text);
}

void SearchLineEdit::setSearchPixmap(const QPixmap& pixmap)
{
  ui_->searchIcon->setPixmap(pixmap);
}

void SearchLineEdit::setSearchDelayInterval(int msec)
{
  searchTimer_->setInterval(msec);
}

void SearchLineEdit::setClearButtonEnabled(bool enabled)
{
  if (clearButtonEnabled_ == enabled)
    return;
  clearButtonEnabled_ = enabled;
  showOrHideClearButton_();
}

void SearchLineEdit::showOrHideClearButton_()
{
  // Show or hide the clear button
  if (!clearButtonEnabled_)
    clearButton_->hide();
  else
    clearButton_->setVisible(!text().isEmpty());
}

void SearchLineEdit::resizeEvent(QResizeEvent* evt)
{
  QFrame::resizeEvent(evt);
  const int buttonSize = evt->size().height() - ICON_SIZE_BUFFER;
  if (buttonSize < 1)
    return;
  clearButton_->setFixedSize(buttonSize, buttonSize);
  clearButton_->move(ui_->searchText->width() - buttonSize, 0);
}

void SearchLineEdit::setSearchIconEnabled(bool enabled)
{
  ui_->searchIcon->setVisible(enabled);
}

void SearchLineEdit::emitSearchRequested_()
{
  emit searchRequested(ui_->searchText->text());
}

}
