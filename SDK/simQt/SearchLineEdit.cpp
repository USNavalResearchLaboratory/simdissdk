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
#include <QIcon>
#include <QLabel>
#include <QProxyStyle>
#include <QTimer>
#include <QWidgetAction>
#include "SearchLineEdit.h"

#ifdef USE_DEPRECATED_SIMDISSDK_API
#include <QToolButton>
#include <QSize>
#include <QResizeEvent>
#include "ui_SearchLineEditQt4.h"
#endif

namespace simQt
{

/** Converting the QIcon to a QPixmap requires a size. 12x12 is approximately the size of the image, once shrunken to the QLineEdit */
static const QSize ICON_SIZE = QSize(12, 12);
/** Buffer around the button for the clear, in pixels */
static const int ICON_SIZE_BUFFER = 3;

/** Custom ProxyStyle which prevents icons from being colored grey when disabled */
class NoDisabledStyle : public QProxyStyle
{
  public:
    virtual QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap, const QStyleOption* option) const Q_DECL_OVERRIDE
    {
      if (iconMode == QIcon::Disabled || !baseStyle())
        return pixmap;

      return baseStyle()->generatedIconPixmap(iconMode, pixmap, option);
    }
};

SearchLineEdit::SearchLineEdit(QWidget* parent)
  : QLineEdit(parent),
  searchTimer_(NULL),
  iconAction_(NULL),
  iconEnabled_(true)
{
  // Configure the timer
  searchTimer_ = new QTimer(this);
  searchTimer_->setSingleShot(true);
  searchTimer_->setInterval(500);
  connect(this, SIGNAL(textChanged(QString)), searchTimer_, SLOT(start()));
  connect(searchTimer_, SIGNAL(timeout()), this, SLOT(emitSearchRequested_()));

  // Pressing enter should also stop the timer and emit the re-search command
  connect(this, SIGNAL(returnPressed()), searchTimer_, SLOT(stop()));
  connect(this, SIGNAL(returnPressed()), this, SLOT(emitSearchRequested_()));

  // Need negative padding to reduce gap between QWidgetAction and beginning of text
  setStyleSheet(styleSheet() + QString::fromStdString("QLineEdit {padding-left: -10px;}"));

  QIcon icon(":/simQt/images/Search.png");
  iconAction_ = new QWidgetAction(this);
  QLabel* iconLabel = new QLabel(this);
  iconLabel->setPixmap(icon.pixmap(ICON_SIZE));
  // Label needs the custom Style so the icon doesn't turn grey
  iconLabel->setStyle(new NoDisabledStyle);
  iconAction_->setDefaultWidget(iconLabel);
  // Need to hide this action from possible ActionsContextMenus
  iconAction_->setVisible(false);
  addAction(iconAction_, QLineEdit::LeadingPosition);

  setClearButtonEnabled(true);
  setPlaceholderText(tr("Search"));
}

SearchLineEdit::~SearchLineEdit()
{
  delete searchTimer_;
  searchTimer_ = NULL;
  delete iconAction_;
  iconAction_ = NULL;
}

const QPixmap* SearchLineEdit::searchPixmap() const
{
  QLabel* label = dynamic_cast<QLabel*>(iconAction_->defaultWidget());
  // The QWidgetAction should only have QLabel
  assert(label);
  return label->pixmap();
}

int SearchLineEdit::searchDelayInterval() const
{
  return searchTimer_->interval();
}

bool SearchLineEdit::searchIconEnabled() const
{
  return iconEnabled_;
}

void SearchLineEdit::setSearchPixmap(const QPixmap& pixmap)
{
  QLabel* label = dynamic_cast<QLabel*>(iconAction_->defaultWidget());
  // The QWidgetAction should only have QLabel
  assert(label);
  label->setPixmap(pixmap);
}

void SearchLineEdit::setSearchDelayInterval(int msec)
{
  searchTimer_->setInterval(msec);
}

void SearchLineEdit::setSearchIconEnabled(bool enabled)
{
  if (enabled == iconEnabled_)
    return;

  if (enabled)
    addAction(iconAction_, QLineEdit::LeadingPosition);
  else
    removeAction(iconAction_);

}

void SearchLineEdit::emitSearchRequested_()
{
  emit searchRequested(text());
}

#ifdef USE_DEPRECATED_SIMDISSDK_API

SearchLineEditQt4::SearchLineEditQt4(QWidget* parent)
  : QFrame(parent),
  clearButtonEnabled_(true),
  searchTimer_(NULL),
  ui_(new Ui_SearchLineEditQt4)
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
  clearButton_->setObjectName("searchLineEditQt4Clear");
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

SearchLineEditQt4::~SearchLineEditQt4()
{
  delete searchTimer_;
  searchTimer_ = NULL;
  delete ui_;
  ui_ = NULL;
}

QString SearchLineEditQt4::text() const
{
  return ui_->searchText->text();
}

QString SearchLineEditQt4::placeholderText() const
{
  return ui_->searchText->placeholderText();
}

const QPixmap* SearchLineEditQt4::searchPixmap() const
{
  return ui_->searchIcon->pixmap();
}

int SearchLineEditQt4::searchDelayInterval() const
{
  return searchTimer_->interval();
}

bool SearchLineEditQt4::clearButtonEnabled() const
{
  return clearButtonEnabled_;
}

bool SearchLineEditQt4::searchIconEnabled() const
{
  return ui_->searchIcon->isVisible();
}

QLineEdit* SearchLineEditQt4::lineEdit() const
{
  return ui_->searchText;
}

void SearchLineEditQt4::setText(const QString& text)
{
  ui_->searchText->setText(text);
}

void SearchLineEditQt4::clear()
{
  if (text().isEmpty())
    return;
  // Clear the text, and immediately send out a refresh on search
  ui_->searchText->clear();
  searchTimer_->stop();
  emit searchRequested("");
}

void SearchLineEditQt4::selectAll()
{
  ui_->searchText->selectAll();
}

void SearchLineEditQt4::setPlaceholderText(const QString& text)
{
  ui_->searchText->setPlaceholderText(text);
}

void SearchLineEditQt4::setSearchPixmap(const QPixmap& pixmap)
{
  ui_->searchIcon->setPixmap(pixmap);
}

void SearchLineEditQt4::setSearchDelayInterval(int msec)
{
  searchTimer_->setInterval(msec);
}

void SearchLineEditQt4::setClearButtonEnabled(bool enabled)
{
  if (clearButtonEnabled_ == enabled)
    return;
  clearButtonEnabled_ = enabled;
  showOrHideClearButton_();
}

void SearchLineEditQt4::showOrHideClearButton_()
{
  // Show or hide the clear button
  if (!clearButtonEnabled_)
    clearButton_->hide();
  else
    clearButton_->setVisible(!text().isEmpty());
}

void SearchLineEditQt4::resizeEvent(QResizeEvent* evt)
{
  QFrame::resizeEvent(evt);
  const int buttonSize = evt->size().height() - ICON_SIZE_BUFFER;
  if (buttonSize < 1)
    return;
  clearButton_->setFixedSize(buttonSize, buttonSize);
  clearButton_->move(ui_->searchText->width() - buttonSize, 0);
}

void SearchLineEditQt4::setSearchIconEnabled(bool enabled)
{
  ui_->searchIcon->setVisible(enabled);
}

void SearchLineEditQt4::emitSearchRequested_()
{
  emit searchRequested(ui_->searchText->text());
}

#endif /* USE_DEPRECATED_SIMDISSDK_API */


}