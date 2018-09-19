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

#include <QSpinBox>
#include <QLineEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>

#include "simCore/Calc/Math.h"
#include "simQt/SegmentedTexts.h"
#include "simQt/SegmentedSpinBox.h"


namespace simQt {

  namespace
  {
    // The following value is used as a unique value for calculating the width of the spinner.
    int MIN_VALUE_FOR_CALCULATING_SIZE = -9999;
  }

/** Event filter class to select segments on mouse click */
class SegmentedEventFilter : public QObject
{
public:
  SegmentedEventFilter(QLineEdit& lineEdit, simQt::SegmentedTexts& completeLine, QObject* parent)
    : QObject(parent),
      lineEdit_(lineEdit),
      completeLine_(completeLine)
  {}

  virtual bool eventFilter(QObject* obj, QEvent* e)
  {
    // set selection based on mouse release
    if (e->type() == QEvent::MouseButtonRelease)
    {
      selectPart_(lineEdit_, completeLine_);
      return true;
    }
    return false;
  }

private:
  void selectPart_(QLineEdit& lineEdit, const simQt::SegmentedTexts& completeLine)
  {
    int location = lineEdit.cursorPosition();
    simQt::SegmentedText* part = completeLine.locatePart(static_cast<size_t>(location));
    // If cursor not an editable location move to one
    if (!part->tabStop())
    {
      if (completeLine.getFirstCharacterLocation(part) == 0)
        part = completeLine.nextTabStop(part);
      else
        part = completeLine.previousTabStop(part);
    }
    lineEdit.setSelection(static_cast<int>(completeLine.getFirstCharacterLocation(part)),
      static_cast<int>(part->numberOfCharacters()));
  }

  QLineEdit& lineEdit_;
  simQt::SegmentedTexts& completeLine_;
};

  SegmentedSpinBox::SegmentedSpinBox(QWidget* parent)
    : QSpinBox(parent),
      completeLine_(NULL),
      initialTime_(1970, 0),
      colorCode_(true),
      segmentedEventFilter_(NULL),
      timer_(new QTimer(this)),
      applyInterval_(1000),
      setSinceFocus_(true)
  {
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    // the method sizeHint is suppose to calculate the correct default minimum width, but something is not right.
    // Therefore, set the width manually to a size that should accommodate the largest size with a reasonable font
    setMinimumWidth(175);
    setAlignment(Qt::AlignRight);
    setRange(MIN_VALUE_FOR_CALCULATING_SIZE, 9999);

    setContextMenuPolicy(Qt::CustomContextMenu);

    timer_->setSingleShot(true);
    connect(timer_, SIGNAL(timeout()), this, SLOT(applyTimestamp_()));
  }

  SegmentedSpinBox::~SegmentedSpinBox()
  {
    delete completeLine_;
    delete segmentedEventFilter_;
    delete timer_;
  }

  simCore::TimeStamp SegmentedSpinBox::timeStamp() const
  {
    assert(completeLine_ != NULL);
    // If the text string does not change then return the given time to prevent a truncated time
    if (timeString_ == completeLine_->text())
      return timeStamp_;
    return completeLine_->timeStamp();
  }

  void SegmentedSpinBox::setTimeStamp(const simCore::TimeStamp& value)
  {
    assert(completeLine_ != NULL);
    completeLine_->setTimeStamp(value);
    lineEdit()->setText(completeLine_->text());
    // The text may truncate the value, so keep a copy so that the exact value can be returned if the text does not change
    timeStamp_ = value;
    timeString_ = completeLine_->text();
  }

  void SegmentedSpinBox::setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end)
  {
    assert(completeLine_ != NULL);
    int originalYear;
    simCore::TimeStamp originalStart;
    simCore::TimeStamp originalEnd;
    completeLine_->timeRange(originalYear, originalStart, originalEnd);

    // If the time range has not changed return
    if ((originalYear == scenarioReferenceYear) &&
        (originalStart == start) &&
        (originalEnd == end))
        return;

    QValidator::State originalState = completeLine_->validateText(completeLine_->text());
    completeLine_->setTimeRange(scenarioReferenceYear, start, end);

    // Determine if the text needs to be updated for a color change

    // If no color coding than return
    if (!colorCode_)
      return;

    QString text = completeLine_->text();

    // If the state has not changed return
    if (originalState == completeLine_->validateText(text))
        return;

    // The state has change so call validate which will update the color
    int ignored;
    validate(text, ignored);
  }

  void SegmentedSpinBox::getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const
  {
    completeLine_->getEnforceLimits(limitBeforeStart, limitAfterEnd);
  }

  void SegmentedSpinBox::setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd)
  {
    completeLine_->setEnforceLimits(limitBeforeStart, limitAfterEnd);
  }

  void SegmentedSpinBox::setLine(SegmentedTexts* line)
  {
    delete completeLine_;
    completeLine_ = line;

    if (completeLine_ == NULL)
      return;

    // install a new event filter to capture mouse clicks for selecting the appropriate segment in the spin box
    delete segmentedEventFilter_;
    segmentedEventFilter_ = new SegmentedEventFilter(*lineEdit(), *completeLine_, this);
    // only install the event filter in the QLineEdit component of the QSpinBox
    QObjectList childObjects = children();
    for(int i = 0; i < childObjects.length(); i++)
    {
      QLineEdit *cast = qobject_cast<QLineEdit*>(childObjects[i]);
      if(cast)
        cast->installEventFilter(segmentedEventFilter_);
    }

  }

  bool SegmentedSpinBox::event(QEvent *e)
  {
    if (e->type() == QEvent::KeyPress)
    {
      QKeyEvent *ke = static_cast<QKeyEvent *>(e);
      if (ke->modifiers() & Qt::ShiftModifier)
      {
        if (ke->key() == Qt::Key_Backtab)
        {
          int location = lineEdit()->cursorPosition();
          SegmentedText* part = completeLine_->locatePart(static_cast<size_t>(location));
          SegmentedText* previousTabStop = completeLine_->previousTabStop(part);
          if (previousTabStop != NULL)
          {
            // Did not walk off the front to process the key event
            int pos = static_cast<int>(completeLine_->getFirstCharacterLocation(previousTabStop));
            lineEdit()->setSelection(pos, static_cast<int>(previousTabStop->numberOfCharacters()));
            return true;
          }
        }
      }
      else if (ke->key() == Qt::Key_Tab)
      {
        int location = lineEdit()->cursorPosition();
        SegmentedText* part = completeLine_->locatePart(static_cast<size_t>(location));
        SegmentedText* nextPart = completeLine_->nextTabStop(part);
        if (nextPart != NULL)
        {
          // Did not work off the end to process the key event
          int pos = static_cast<int>(completeLine_->getFirstCharacterLocation(nextPart));
          lineEdit()->setSelection(pos, static_cast<int>(nextPart->numberOfCharacters()));
          return true;
        }
      }
      else if ((ke->key() == Qt::Key_Enter) || (ke->key() == Qt::Key_Return))
      {
        // User says they are done, so force an focusOutEvent
        this->focusNextChild();
        return true;
      }
    }

    // It was either not a tab or back tab or we walked off one of the ends and need the software to go to the next tab stop
    return QSpinBox::event(e);
  }

  void SegmentedSpinBox::applyTimestamp_()
  {
    // If apply was queued and something else triggers an apply first, don't bother applying again
    timer_->stop();

    simCore::TimeStamp currentTime = completeLine_->timeStamp();
    simCore::TimeStamp clampedTime = completeLine_->clampTime(currentTime);
    if (currentTime != clampedTime)
    {
      // Range Limit the value, since the user can type in a value out of range
      completeLine_->setTimeStamp(clampedTime);
    }

    const int selectionStart = lineEdit()->selectionStart();
    const int selectionLength = lineEdit()->selectedText().length();
    const int cursorPosition = lineEdit()->cursorPosition();
    lineEdit()->setText(completeLine_->text());
    lineEdit()->setCursorPosition(simCore::sdkMin(cursorPosition, lineEdit()->text().length()));
    // If there was a selection, restore it after timestamp is updated
    if (selectionStart != -1)
    {
      const int newSelectionStart = simCore::sdkMin(selectionStart, lineEdit()->text().length() - 1);
      if (newSelectionStart != -1)
        lineEdit()->setSelection(newSelectionStart, simCore::sdkMin(selectionLength, lineEdit()->text().length() - newSelectionStart));
    }

    if (initialTime_ != completeLine_->timeStamp())
    {
      completeLine_->valueChanged();
      // If we have focus assume the change was user initiated
      if (setSinceFocus_ == false)
        completeLine_->valueEdited();
    }

    setSinceFocus_ = true;
  }

  void SegmentedSpinBox::queueApplyTimestamp_() const
  {
    // If apply already queued to happen sooner than we would queue it now, do nothing
    if (timer_->isActive() && timer_->remainingTime() < applyInterval_)
      return;

    if (applyInterval_ >= 0)
      timer_->start(applyInterval_);
  }

  int SegmentedSpinBox::applyInterval()
  {
    return applyInterval_;
  }

  void SegmentedSpinBox::setApplyInterval(int milliseconds)
  {
    applyInterval_ = milliseconds;
  }

  void SegmentedSpinBox::focusOutEvent(QFocusEvent* e)
  {
    if (!setSinceFocus_)
      applyTimestamp_();
    QSpinBox::focusOutEvent(e);
  }

  void SegmentedSpinBox::focusInEvent(QFocusEvent* e)
  {
    initialTime_ = completeLine_->timeStamp();
    setSinceFocus_ = false;
    QSpinBox::focusInEvent(e);
  }

  void SegmentedSpinBox::stepBy(int steps)
  {
    int location = lineEdit()->cursorPosition();
    SegmentedText* part = completeLine_->locatePart(static_cast<size_t>(location));
    // If cursor not an editable location move to one
    if (!part->tabStop())
    {
      if (completeLine_->getFirstCharacterLocation(part) == 0)
        part = completeLine_->nextTabStop(part);
      else
        part = completeLine_->previousTabStop(part);
    }

    // Make the change
    if (part != NULL)
    {
      part->stepBy(steps);
    }

    int pos = 0;
    QString text = completeLine_->text();
    // Need to validate before calling stepBy, otherwise the GUI does not update correctly.
    validate(text, pos);
    QSpinBox::stepBy(steps);

    if (part != NULL)
    {
      // select the new segment text
      int startLocation = static_cast<int>(completeLine_->getFirstCharacterLocation(part));
      lineEdit()->setSelection(startLocation, static_cast<int>(part->numberOfCharacters()));
    }
  }

  int SegmentedSpinBox::valueFromText(const QString &text) const
  {
    // return a large non-zero number so that the up/down arrows work
    return 5000;
  }

  QString SegmentedSpinBox::textFromValue(int value) const
  {
    if (value == MIN_VALUE_FOR_CALCULATING_SIZE)
    {
      /*
       * The first call to textFromValue is with the minimum value and is used to calculate
       * the default size of the spinner.  Have all formats return the same text so that the
       * size to the spinner does not change as the user changes formats.
       */
      return "Jan 31 1970 00:00:00.000000";
    }

    // need to trim white space
    if (completeLine_->text() != " ")
    {
      return completeLine_->text();
    }

    return "";
  }

  QValidator::State SegmentedSpinBox::validate(QString& text, int& pos) const
  {
    // Only colorCode if the user wants colorCode and the widget is enabled for editing
    bool colorCodeText = colorCode_ & isEnabled();

    QValidator::State state = completeLine_->validateText(text);
    if (state == QValidator::Acceptable)
    {
      lineEdit()->setStyleSheet("");
      completeLine_->setText(text);
      queueApplyTimestamp_();
    }
    else if (state == QValidator::Intermediate)
    {
      if (colorCodeText)
        lineEdit()->setStyleSheet("QLineEdit {color: blue }");
      else
        lineEdit()->setStyleSheet("");
      completeLine_->setText(text);
      timer_->stop();
    }
    else
    {
      if (colorCodeText)
        lineEdit()->setStyleSheet("QLineEdit {color: red }");
      else
        lineEdit()->setStyleSheet("");
      timer_->stop();
    }

    return QValidator::Acceptable;
  }

  bool SegmentedSpinBox::colorCode() const
  {
    return colorCode_;
  }

  void SegmentedSpinBox::setColorCode(bool value)
  {
    colorCode_ = value;
  }


QSize SegmentedSpinBox::sizeHint() const
{
  /*
   * The Qt4 QAbstractSpinBox::sizeHint limits the text to 18 characters which is too small
   * for our application.  Take the value and re-scale to 27 characters which is needed
   * for the Month Year format.
   * The QAbstractSpinBox caches the size so no need to cache the value at this level
   */
  QSize rv = QSpinBox::sizeHint();

#if(QT_VERSION < QT_VERSION_CHECK(5,0,0))
  rv.setWidth(rv.width() * 27 / 18);  // Change 18 characters to 27 characters as detailed in the comments above
#endif

  return rv;
}


} // namespace

