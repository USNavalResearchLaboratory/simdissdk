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
#include <QIcon>
#include <QLabel>
#include <QProxyStyle>
#include <QTimer>
#include <QWidgetAction>
#include "SearchLineEdit.h"

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
  searchTimer_(nullptr),
  iconAction_(nullptr),
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
  proxyStyle_ = new NoDisabledStyle;
  iconLabel->setStyle(proxyStyle_);
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
  searchTimer_ = nullptr;
  delete iconAction_;
  iconAction_ = nullptr;
  delete proxyStyle_;
  proxyStyle_ = nullptr;
}

const QPixmap* SearchLineEdit::searchPixmap() const
{
  QLabel* label = dynamic_cast<QLabel*>(iconAction_->defaultWidget());
  // The QWidgetAction should only have QLabel
  assert(label);
  return label ? label->pixmap() : nullptr;
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
  if (label)
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

}
