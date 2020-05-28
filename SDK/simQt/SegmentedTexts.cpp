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
#include <math.h>
#include <limits>
#include <QDateTime>

#include "simCore/Calc/Math.h"
#include "simCore/Time/Exception.h"
#include "simCore/Time/Utils.h"
#include "simQt/SegmentedTexts.h"

namespace simQt {
// Precision limit is 6 -- bad precision past 6
static const unsigned int MAX_PRECISION = 6;

  SegmentedTexts::SegmentedTexts()
    : scenarioReferenceYear_(1970),
      precision_(3),
      limitBeforeStart_(true),
      limitAfterEnd_(true)
  {
  }

  SegmentedTexts::~SegmentedTexts()
  {
    clearParts();
  }

  void SegmentedTexts::clearParts()
  {
    for (auto it = segments_.begin(); it != segments_.end(); ++it)
      delete *it;
    segments_.clear();
  }

  unsigned int SegmentedTexts::precision() const
  {
    return precision_;
  }

  void SegmentedTexts::setPrecision(unsigned int digits)
  {
    digits = simCore::sdkMin(MAX_PRECISION, digits);
    if (digits == precision_)
      return;

    precision_ = digits;
    adjustTimeRange_();
    makeSegments_();
  }

  void SegmentedTexts::addPart(SegmentedText* part)
  {
    segments_.push_back(part);
  }

  SegmentedText* SegmentedTexts::locatePart(size_t pos) const
  {
    size_t current = 0;
    if (pos == 0)
      return segments_.front();

    for (auto it = segments_.begin(); it != segments_.end(); ++it)
    {
      auto part = *it;
      current += part->numberOfCharacters();
      // If at the end of a part, but it is a tabStop, then the cursor is in this part
      if ((current == pos) && part->tabStop())
        return part;
      if (current > pos)
        return part;
    }

    return segments_.back();
  }

  size_t SegmentedTexts::getFirstCharacterLocation(const SegmentedText* inputPart) const
  {
    size_t current = 0;

    for (auto it = segments_.begin(); it != segments_.end(); ++it)
    {
      if (inputPart == *it)
        return current;

      current += (*it)->numberOfCharacters();
    }

    assert(false);
    return current;
  }

  SegmentedText* SegmentedTexts::nextTabStop(const SegmentedText* inputPart) const
  {
    bool pending = false;
    for (auto it = segments_.begin(); it != segments_.end(); ++it)
    {
      auto part = *it;
      if (inputPart == part)
        pending = true;
      else if (pending)
      {
        if (part->tabStop())
          return part;
      }
    }

    // Walked off the end which is OK
    return NULL;
  }

  SegmentedText* SegmentedTexts::previousTabStop(const SegmentedText* inputPart) const
  {
    SegmentedText* lastStop = NULL;
    for (auto it = segments_.begin(); it != segments_.end(); ++it)
    {
      auto part = *it;
      if (part == inputPart)
        return lastStop;  // Return the previous stop, if any

      if (part->tabStop())
        lastStop = part;
    }

    // Did not find the inputPart, so something is wrong
    assert(false);
    return NULL;
  }

  QString SegmentedTexts::text() const
  {
    // Put all the parts together to form a complete line
    QString rv;
    for (auto it = segments_.begin(); it != segments_.end(); ++it)
      rv += (*it)->text();

    return rv;
  }

  QValidator::State SegmentedTexts::setText(const QString& text)
  {
    size_t startLocation = 0;
    for (auto it = segments_.begin(); it != segments_.end(); ++it)
    {
      QValidator::State state;
      startLocation = (*it)->setText(text, startLocation, state);
      if (state != QValidator::Acceptable)
      {
        assert(false);  // should not happen, since the line should be validated before setting
        return state;
      }
    }

    return QValidator::Acceptable;
  }

  QValidator::State SegmentedTexts::validateText(const QString& text) const
  {
    size_t startLocation = 0;
    QValidator::State lastState = QValidator::Acceptable;
    for (auto it = segments_.begin(); it != segments_.end(); ++it)
    {
      QValidator::State state;
      startLocation = (*it)->validateText(text, startLocation, state);

      if (state == QValidator::Invalid)
        return state;  // give up on the first error

      if (state != QValidator::Acceptable)
        lastState = state;
    }

    return lastState;
  }

  void SegmentedTexts::setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end)
  {
    scenarioReferenceYear_ = scenarioReferenceYear;
    start_ = start;
    end_ = end;
    adjustTimeRange_();
  }

  void SegmentedTexts::adjustTimeRange_()
  {
    adjustedStart_ = start_;
    if (adjustedStart_.secondsSinceRefYear() != simCore::ZERO_SECONDS)
    {
      const simCore::Seconds& secondsStart = adjustedStart_.secondsSinceRefYear();
      // adjust the fraction representation to contain number of digits corresponding to precision
      const int fraction = fractionFromField_(fractionToField_(secondsStart), precision_);
      adjustedStart_.setTime(adjustedStart_.referenceYear(), simCore::Seconds(secondsStart.getSeconds(), fraction));
    }

    adjustedEnd_ = end_;
    if (adjustedEnd_.secondsSinceRefYear() != simCore::ZERO_SECONDS)
    {
      const simCore::Seconds& secondsEnd = adjustedEnd_.secondsSinceRefYear();
      const int scale = (precision_ >= 9) ? 1 : static_cast<int>(pow(10.0, 9 - precision_));
      // convert time in number of nanoseconds to number of time units in the specified precision, using ceiling to round up
      const int timeUnits = std::ceil(static_cast<double>(secondsEnd.getFractionLong()) / scale);
      // convert the number of time units in the specified precision back to a number of nanoseconds (that is now ceilinged to the desired precision)
      const int fraction = fractionFromField_(timeUnits, precision_);
      adjustedEnd_.setTime(adjustedEnd_.referenceYear(), simCore::Seconds(secondsEnd.getSeconds(), fraction));
    }
  }

  void SegmentedTexts::timeRange(int& scenarioReferenceYear, simCore::TimeStamp& start, simCore::TimeStamp& end)
  {
    scenarioReferenceYear = scenarioReferenceYear_;
    start = start_;
    end = end_;
  }

  void SegmentedTexts::getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const
  {
    limitBeforeStart = limitBeforeStart_;
    limitAfterEnd = limitAfterEnd_;
  }

  void SegmentedTexts::setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd)
  {
    limitBeforeStart_ = limitBeforeStart;
    limitAfterEnd_ = limitAfterEnd;
  }

  simCore::TimeStamp SegmentedTexts::clampTime(const simCore::TimeStamp& value)
  {
    if (limitBeforeStart_ && (value < adjustedStart_))
      return adjustedStart_;

    if (limitAfterEnd_ && (value > adjustedEnd_))
      return adjustedEnd_;

    return value;
  }

  bool SegmentedTexts::inRange_(const simCore::TimeStamp& current, bool limitBeforeStart, bool limitAfterEnd) const
  {
    if (limitBeforeStart && (current < adjustedStart_))
    {
      return false;
    }

    if (limitAfterEnd && (current > adjustedEnd_))
    {
      return false;
    }

    return true;
  }

  NumberText* SegmentedTexts::createFactionOfSeconds_(int precision)
  {
    const double scale = std::pow(10.0, precision_);
    const int maxValue = static_cast<int>(scale) - 1;
    return new NumberText(this, 0, maxValue, precision, true, 1.0/scale, true);
  }

#ifdef USE_DEPRECATED_SIMDISSDK_API
  NumberText* SegmentedTexts::updateFactionOfSeconds_(int precision)
  {
    if (precision < 1)
      precision = 1;
    else if (precision > static_cast<int>(MAX_PRECISION))
      precision = static_cast<int>(MAX_PRECISION);

    delete segments_.back();
    segments_.pop_back();
    NumberText* fraction = createFactionOfSeconds_(precision_);
    addPart(fraction);
    return fraction;
  }
#endif /* USE_DEPRECATED_SIMDISSDK_API */

  int SegmentedTexts::fractionToField_(const simCore::Seconds& secondsRounded) const
  {
    // precision limit of 9 is due to simCore::Seconds implementation, and is independent of SegmentedText MAX_PRECISION
    // scale factor to convert from # nanoseconds to field value in specified precision
    const int scale = (precision_ >= 9) ? 1 : static_cast<int>(pow(10.0, 9 - precision_));
    assert(scale > 0);
    return secondsRounded.getFractionLong() / scale;
  }

  int SegmentedTexts::fractionFromField_(int fractionFieldValue, int precision) const
  {
    // precision limit of 9 is due to simCore::Seconds implementation, and is independent of SegmentedText MAX_PRECISION
    // scale factor to convert from field value in specified precision to # nanoseconds
    const int scale = (precision >= 9) ? 1 : static_cast<int>(pow(10.0, 9 - precision));
    assert(scale > 0);
    return fractionFieldValue * scale;
  }

  //------------------------------------------------------------------------------------------------------------------

  SeparatorText::SeparatorText(QString separator, bool optional)
    : SegmentedText(false),
      separator_(separator),
      optional_(optional)
  {
  }

  SeparatorText::~SeparatorText()
  {
  }

  size_t SeparatorText::numberOfCharacters() const
  {
    return static_cast<size_t>(separator_.size());
  }

  size_t SeparatorText::spaceLeft() const
  {
    // Always full
    return 0;
  }

  void SeparatorText::stepBy(int amount)
  {
    // Static value; nothing to do
  }

  QString SeparatorText::text() const
  {
    return separator_;
  }

  size_t SeparatorText::setText(const QString& text, size_t startLocation, QValidator::State& state)
  {
    return validateText(text, startLocation, state);
  }

  size_t SeparatorText::validateText(const QString& text, size_t startLocation, QValidator::State& state) const
  {
    if (static_cast<int>(startLocation) >= text.size())
    {
      if (optional_)
        state = QValidator::Acceptable;
      else
        state = QValidator::Invalid;

      return startLocation;
    }

    QString part = text.mid(static_cast<int>(startLocation), separator_.size());

    if (part == separator_)
      state = QValidator::Acceptable;
    else
      state = QValidator::Invalid;

    return startLocation + part.size();
  }

  //------------------------------------------------------------------------------------------------------------------

  NumberText::NumberText(SegmentedTexts* line, int minValue, int maxValue, size_t maxDigits, bool leadingZeros, double timeScaleFactor, bool optional)
    : SegmentedText(true),
      line_(line),
      minValue_(minValue),
      maxValue_(maxValue),
      maxDigits_(maxDigits),
      leadingZeros_(leadingZeros),
      timeScaleFactor_(timeScaleFactor),
      optional_(optional)
  {
    text_ = toString_(minValue);
  }

  NumberText::~NumberText()
  {
  }

  int NumberText::value() const
  {
    return text_.toInt();
  }

  void NumberText::setValue(int value)
  {
    text_ = toString_(value);
  }

  size_t NumberText::numberOfCharacters() const
  {
    return static_cast<size_t>(text_.size());
  }

  size_t NumberText::spaceLeft() const
  {
    return maxDigits_ - static_cast<size_t>(text_.size());
  }

  void NumberText::stepBy(int amount)
  {
    // The timeScaleFactor is 1 for seconds, 60 for minutes, 3600 for hours, 0.1 for tenths, etc.
    double modfIntPart;
    const double timeScaleFactorFractionPart = std::modf(timeScaleFactor_, &modfIntPart);
    const int timeScaleFactorIntPart = static_cast<int>(modfIntPart);
    const int timeScaleNanoSeconds = timeScaleFactorFractionPart * 1e09;

    // Adjust the time by the timeScaleFactor
    const simCore::Seconds adjustment(amount * timeScaleFactorIntPart, amount * timeScaleNanoSeconds);
    const simCore::TimeStamp& adjusted = line_->timeStamp() + adjustment;

    // then limit the change by the time range
    line_->setTimeStamp(line_->clampTime(adjusted));
  }

  QString NumberText::text() const
  {
    return text_;
  }

  size_t NumberText::setText(const QString& text, size_t startLocation, QValidator::State& state)
  {
    size_t endLocation = validateText(text, startLocation, state);
    if (state != QValidator::Invalid)
      text_ = text.mid(static_cast<int>(startLocation), static_cast<int>(endLocation-startLocation));

    return endLocation;
  }

  size_t NumberText::validateText(const QString& text, size_t startLocation, QValidator::State& state) const
  {
    int count = static_cast<int>(startLocation);

    if (count >= text.size())
    {
      if (optional_)
        state = QValidator::Acceptable;
      else
        state = QValidator::Invalid;

      return count;
    }

    while ((count < text.size()) && (text[count] >= '0') && (text[count] <= '9') && ((static_cast<size_t>(count)-startLocation) < maxDigits_))
      count++;

    if (count != static_cast<int>(startLocation))
    {
      QString part = text.mid(static_cast<int>(startLocation), static_cast<int>(count-startLocation));
      int value = part.toInt();
      if ((value >= minValue_) && (value <= maxValue_))
        state = QValidator::Acceptable;
      else
        state = QValidator::Invalid;
    }
    else
      state = QValidator::Invalid;

    return count;
  }

  QString NumberText::toString_(int value) const
  {
    if (leadingZeros_)
      return QString("%1").arg(value, static_cast<int>(maxDigits_), 10, QChar('0'));

    return QString("%1").arg(value);
  }

  //--------------------------------------------------------------------------
  MonthText::MonthText(SegmentedTexts* parentLine)
  : SegmentedText(true),
    line_(parentLine),
    currentMonth_(0)
  {
  }

  int MonthText::intValue() const
  {
    return currentMonth_;
  }

  void MonthText::setIntValue(int monthNum)
  {
    // this should only be driven by simCore time functions
    assert(0 <= monthNum && monthNum < 12);
    currentMonth_ = monthNum;
  }

  size_t MonthText::numberOfCharacters() const
  {
    return ABBR_LENGTH;
  }

  size_t MonthText::spaceLeft() const
  {
    return 0;
  }

  void MonthText::stepBy(int amount)
  {
    // while decreasing month
    while (amount < 0)
    {
      stepDn_();
      ++amount; // back toward 0
    }

    // while increasing month
    while (amount > 0)
    {
      stepUp_();
      --amount; // down toward 0
    }
  }

  void MonthText::stepUp_()
  {
    const simCore::TimeStamp& currentTimeStamp = line_->timeStamp();
    const simCore::Seconds& currentTime = currentTimeStamp.secondsSinceRefYear();
    // add one month (in seconds)(Jan 29 will become Mar 1)
    const simCore::Seconds adjustment(86400 * simCore::daysPerMonth(currentTimeStamp.referenceYear(), currentMonth_), 0);
    line_->setTimeStamp(line_->clampTime(currentTimeStamp + adjustment));
  }

  void MonthText::stepDn_()
  {
    const simCore::TimeStamp& currentTimeStamp = line_->timeStamp();
    const simCore::Seconds& currentTime = currentTimeStamp.secondsSinceRefYear();
    const int refYear = currentTimeStamp.referenceYear();
    // subtracting a month is a little harder (adjust month id for underflow)
    // (Mar 29 will become Mar 1)
    const int prevMonth = (currentMonth_ == 0) ? 11 : currentMonth_ - 1;
    const int yearToUse = (currentMonth_ == 0) ? refYear - 1 : refYear;
    const simCore::Seconds adjustment(86400 * simCore::daysPerMonth(yearToUse, prevMonth), 0);
    line_->setTimeStamp(line_->clampTime(currentTimeStamp - adjustment));
  }

  QString MonthText::text() const
  {
    return simCore::MonthDayTimeFormatter::monthIntToString(currentMonth_).c_str();
  }

  size_t MonthText::setText(const QString& line, size_t startLocation, QValidator::State& state)
  {
    const size_t endLocation = validateText(line, startLocation, state);
    if (state != QValidator::Invalid)
      currentMonth_ = simCore::MonthDayTimeFormatter::monthStringToInt(line.toStdString().substr(startLocation, ABBR_LENGTH));

    return endLocation;
  }

  size_t MonthText::validateText(const QString& line, size_t startLocation, QValidator::State& state) const
  {
    // check if the text is a valid month name
    if (simCore::MonthDayTimeFormatter::monthStringToInt(line.toStdString().substr(startLocation, ABBR_LENGTH)) == -1)
    {
      state = QValidator::Invalid;
      return startLocation;
    }

    state = QValidator::Acceptable;
    return startLocation + ABBR_LENGTH;
  }

  //--------------------------------------------------------------------------
  SecondsTexts::SecondsTexts()
    : SegmentedTexts()
  {
    makeSegments_();
  }

  SecondsTexts::~SecondsTexts()
  {
    // No need to delete the pointers since they are also in segments_ of SegmentedTexts
  }

  void SecondsTexts::makeSegments_()
  {
    clearParts();

    // almost 70 years
    seconds_ = new NumberText(this, 0, std::numeric_limits<int>::max(), 10, false, 1.0, false);
    if (precision_ != 0)
      fraction_ = createFactionOfSeconds_(precision_);
    else
      fraction_ = NULL;

    addPart(seconds_);
    if (fraction_ != NULL)
    {
      addPart(new SeparatorText(".", true));
      addPart(fraction_);
    }
  }

  simCore::TimeStamp SecondsTexts::timeStamp() const
  {
    const int seconds = seconds_->value();
    // Need to scale based on the number of digits after the decimal point.
    const int fraction = (fraction_ == NULL) ? 0 : fractionFromField_(fraction_->value(), fraction_->text().size());
    return simCore::TimeStamp(scenarioReferenceYear_, simCore::Seconds(seconds, fraction));
  }

  void SecondsTexts::setTimeStamp(const simCore::TimeStamp& value)
  {
    if (!inRange_(value, limitBeforeStart_, limitAfterEnd_))
      return;

    // use TimeStamp to renormalize time after rounding
    const simCore::TimeStamp stamp(value.referenceYear(), value.secondsSinceRefYear().rounded(precision_));

    // SecondsTexts fields are always relative to scenarioReferenceYear_: they do not reset to 0 if year rolls over.
    const simCore::Seconds& secondsSinceScenarioRefYear = stamp.secondsSinceRefYear(scenarioReferenceYear_);
    seconds_->setValue(static_cast<int>(secondsSinceScenarioRefYear.getSeconds()));
    if (fraction_ != NULL)
      fraction_->setValue(fractionToField_(secondsSinceScenarioRefYear));
  }

  QValidator::State SecondsTexts::validateText(const QString& text) const
  {
    const QValidator::State lastState = SegmentedTexts::validateText(text);
    if (lastState != QValidator::Acceptable)
      return lastState;

    SecondsTexts temp;
    temp.setTimeRange(scenarioReferenceYear_, simCore::MIN_TIME_STAMP, simCore::TimeStamp(2070, simCore::ZERO_SECONDS));
    temp.setEnforceLimits(limitBeforeStart_, limitAfterEnd_);
    temp.setText(text);
    if (!inRange_(temp.timeStamp(), true, true))  // Always color code base on the limits
      return QValidator::Intermediate;

    return lastState;
  }

  void SecondsTexts::valueEdited()
  {
    emit timeEdited(timeStamp());
  }

  void SecondsTexts::valueChanged()
  {
    emit timeChanged(timeStamp());
  }

  //--------------------------------------------------------------------------
  MinutesTexts::MinutesTexts()
    : SegmentedTexts()
  {
    makeSegments_();
  }

  MinutesTexts::~MinutesTexts()
  {
    // No need to delete the pointers since they are also in segments_ of SegmentedTexts
  }

  void MinutesTexts::makeSegments_()
  {
    clearParts();

    // 70 years
    minutes_ = new NumberText(this, 0, 36792000, 8, false, 60.0, false);  // No leading zeros
    seconds_ = new NumberText(this, 0, 59, 2, true, 1.0, false);
    if (precision_ != 0)
      fraction_ = createFactionOfSeconds_(precision_);
    else
      fraction_ = NULL;

    addPart(minutes_);
    addPart(new SeparatorText(":", false));
    addPart(seconds_);
    if (fraction_ != NULL)
    {
      addPart(new SeparatorText(".", true));
      addPart(fraction_);
    }
  }

  simCore::TimeStamp MinutesTexts::timeStamp() const
  {
    const int seconds = (minutes_->value() * 60) + seconds_->value();
    // Need to scale based on the number of digits after the decimal point.
    const int fraction = (fraction_ == NULL) ? 0 : fractionFromField_(fraction_->value(), fraction_->text().size());
    return simCore::TimeStamp(scenarioReferenceYear_, simCore::Seconds(seconds, fraction));
  }

  void MinutesTexts::setTimeStamp(const simCore::TimeStamp& value)
  {
    if (!inRange_(value, limitBeforeStart_, limitAfterEnd_))
      return;

    // use TimeStamp to renormalize time after rounding
    const simCore::TimeStamp stamp(value.referenceYear(), value.secondsSinceRefYear().rounded(precision_));

    // MinutesTexts fields are always relative to scenarioReferenceYear_: they do not reset to 0 if year rolls over.
    const simCore::Seconds& secondsSinceScenarioRefYear = stamp.secondsSinceRefYear(scenarioReferenceYear_);
    const int64_t secondsSinceRefYear = secondsSinceScenarioRefYear.getSeconds();
    const int minutes = static_cast<int>(secondsSinceRefYear / simCore::SECPERMIN);
    minutes_->setValue(minutes);
    seconds_->setValue(static_cast<int>(secondsSinceRefYear - (minutes*simCore::SECPERMIN)));
    if (fraction_ != NULL)
      fraction_->setValue(fractionToField_(secondsSinceScenarioRefYear));
  }

  QValidator::State MinutesTexts::validateText(const QString& text) const
  {
    QValidator::State lastState = SegmentedTexts::validateText(text);
    if (lastState == QValidator::Acceptable)
    {
      MinutesTexts temp;
      temp.setTimeRange(scenarioReferenceYear_, simCore::MIN_TIME_STAMP, simCore::TimeStamp(2070, simCore::ZERO_SECONDS));
      temp.setEnforceLimits(limitBeforeStart_, limitAfterEnd_);
      temp.setText(text);
      if (!inRange_(temp.timeStamp(), true, true))  // Always color code base on the limits
        return QValidator::Intermediate;
    }

    return lastState;
  }

  void MinutesTexts::valueEdited()
  {
    emit timeEdited(timeStamp());
  }

  void MinutesTexts::valueChanged()
  {
    emit timeChanged(timeStamp());
  }

  //------------------------------------------------------------------------------------------------------------------

  HoursTexts::HoursTexts()
    : SegmentedTexts()
  {
    makeSegments_();
  }

  HoursTexts::~HoursTexts()
  {
    // No need to delete the pointers since they are also in segments_ of SegmentedTexts
  }

  void HoursTexts::makeSegments_()
  {
    clearParts();

    // 70 years
    hours_ = new NumberText(this, 0, 613200, 6, false, 60.0*60.0, false);  // No leading zeros
    minutes_ = new NumberText(this, 0, 59, 2, true, 60.0, false);
    seconds_ = new NumberText(this, 0, 59, 2, true, 1.0, false);
    if (precision_ != 0)
      fraction_ = createFactionOfSeconds_(precision_);
    else
      fraction_ = NULL;

    addPart(hours_);
    addPart(new SeparatorText(":", false));
    addPart(minutes_);
    addPart(new SeparatorText(":", false));
    addPart(seconds_);
    if (fraction_ != NULL)
    {
      addPart(new SeparatorText(".", true));
      addPart(fraction_);
    }
  }

  simCore::TimeStamp HoursTexts::timeStamp() const
  {
    const int64_t seconds = (((hours_->value() * 60) + minutes_->value()) * 60) + seconds_->value();
    // Need to scale based on the number of digits after the decimal point.
    const int fraction = (fraction_ == NULL) ? 0 : fractionFromField_(fraction_->value(), fraction_->text().size());
    return simCore::TimeStamp(scenarioReferenceYear_, simCore::Seconds(seconds, fraction));
  }

  void HoursTexts::setTimeStamp(const simCore::TimeStamp& value)
  {
    if (!inRange_(value, limitBeforeStart_, limitAfterEnd_))
      return;

    // use TimeStamp to renormalize time after rounding
    const simCore::TimeStamp stamp(value.referenceYear(), value.secondsSinceRefYear().rounded(precision_));

    // HoursTexts fields are always relative to scenarioReferenceYear_: they do not reset to 0 if year rolls over.
    const simCore::Seconds& secondsSinceScenarioRefYear = stamp.secondsSinceRefYear(scenarioReferenceYear_);
    int64_t secondsSinceRefYear = secondsSinceScenarioRefYear.getSeconds();
    const int hours = static_cast<int>(secondsSinceRefYear / simCore::SECPERHOUR);
    secondsSinceRefYear -= (hours*simCore::SECPERHOUR);
    const int minutes = static_cast<int>(secondsSinceRefYear / simCore::SECPERMIN);
    secondsSinceRefYear -= (minutes*simCore::SECPERMIN);

    hours_->setValue(hours);
    minutes_->setValue(minutes);
    seconds_->setValue(static_cast<int>(secondsSinceRefYear));
    if (fraction_ != NULL)
      fraction_->setValue(fractionToField_(secondsSinceScenarioRefYear));
  }

  QValidator::State HoursTexts::validateText(const QString& text) const
  {
    QValidator::State lastState = SegmentedTexts::validateText(text);
    if (lastState == QValidator::Acceptable)
    {
      HoursTexts temp;
      temp.setTimeRange(scenarioReferenceYear_, simCore::MIN_TIME_STAMP, simCore::TimeStamp(2070, simCore::ZERO_SECONDS));
      temp.setEnforceLimits(limitBeforeStart_, limitAfterEnd_);
      temp.setText(text);
      if (!inRange_(temp.timeStamp(), true, true))  // Always color code base on the limits
        return QValidator::Intermediate;
    }

    return lastState;
  }

  void HoursTexts::valueEdited()
  {
    emit timeEdited(timeStamp());
  }

  void HoursTexts::valueChanged()
  {
    emit timeChanged(timeStamp());
  }

  //------------------------------------------------------------------------------------------------------------------

  OrdinalTexts::OrdinalTexts()
    : SegmentedTexts(),
      zone_(simCore::TIMEZONE_UTC)
  {
    makeSegments_();
  }

  OrdinalTexts::~OrdinalTexts()
  {
    // No need to delete the pointers since they are also in segments_ of SegmentedTexts
  }

  void OrdinalTexts::makeSegments_()
  {
    clearParts();

    days_ = new NumberText(this, 1, 366, 3, true, 24.0*60.0 * 60, false);
    // If the user increments the year the code will add 365*24*60*60 seconds to the current value.
    // If the time change crosses Feb 29th, the year will change by one but the day of year will also
    // change by one.   I do not think it is worth fixing.
    years_ = new NumberText(this, 1970, 2046, 4, false, 365.0*24.0*60.0*60.0, false);
    hours_ = new NumberText(this, 0, 23, 2, true, 60.0*60.0, false);
    minutes_ = new NumberText(this, 0, 59, 2, true, 60.0, false);
    seconds_ = new NumberText(this, 0, 59, 2, true, 1.0, false);
    if (precision_ != 0)
      fraction_ = createFactionOfSeconds_(precision_);
    else
      fraction_ = NULL;

    addPart(days_);
    addPart(new SeparatorText(" ", false));
    addPart(years_);
    addPart(new SeparatorText(" ", false));
    addPart(hours_);
    addPart(new SeparatorText(":", false));
    addPart(minutes_);
    addPart(new SeparatorText(":", false));
    addPart(seconds_);
    if (fraction_ != NULL)
    {
      addPart(new SeparatorText(".", true));
      addPart(fraction_);
    }
  }

  simCore::TimeStamp OrdinalTexts::timeStamp() const
  {
    int64_t seconds = (days_->value()-1) * 24;
    seconds += hours_->value();
    seconds *= 60;
    seconds += minutes_->value();
    seconds *= 60;
    seconds += seconds_->value();
    // Need to scale based on the number of digits after the decimal point.
    const int fraction = (fraction_ == NULL) ? 0 : fractionFromField_(fraction_->value(), fraction_->text().size());
    simCore::TimeStamp stamp(years_->value(), simCore::Seconds(seconds, fraction));

    // Remove the timezone offset that was introduced by setTimestamp()
    if (zone_ == simCore::TIMEZONE_LOCAL)
    {
      // Define a UTC datetime with the timestamp
      const tm& timeComponents = simCore::getTimeStruct(stamp);
      QDateTime dateTime(QDate(1900 + timeComponents.tm_year, 1 + timeComponents.tm_mon, timeComponents.tm_mday), QTime(timeComponents.tm_hour, timeComponents.tm_min, timeComponents.tm_sec), Qt::UTC);
      // Change it to local time so that Qt figures out the local time offset from UTC time
      dateTime.setTimeSpec(Qt::LocalTime);
      stamp -= simCore::Seconds(dateTime.offsetFromUtc(), 0);
    }
    return stamp;
  }

  void OrdinalTexts::setTimeStamp(const simCore::TimeStamp& value)
  {
    if (!inRange_(value, limitBeforeStart_, limitAfterEnd_))
      return;

    simCore::TimeStamp stamp = value;
    if (zone_ == simCore::TIMEZONE_LOCAL)
    {
      // Define a UTC datetime with the timestamp
      const tm& timeComponents = simCore::getTimeStruct(value);
      QDateTime dateTime(QDate(1900 + timeComponents.tm_year, 1 + timeComponents.tm_mon, timeComponents.tm_mday), QTime(timeComponents.tm_hour, timeComponents.tm_min, timeComponents.tm_sec), Qt::UTC);
      // Change it to local time so that Qt figures out the local time offset from UTC time
      dateTime.setTimeSpec(Qt::LocalTime);
      stamp += simCore::Seconds(dateTime.offsetFromUtc(), 0);
    }

    // rounding the seconds can increase the refyear, need to reset the timestamp with rounded time to ensure no artifacts.
    stamp.setTime(stamp.referenceYear(), stamp.secondsSinceRefYear().rounded(precision_));
    unsigned int dayOfYear = 0; // [0,365] 365 possible in leap year
    unsigned int hour = 0; // hours since midnight (0..23)
    unsigned int min = 0;
    unsigned int sec = 0;
    stamp.getTimeComponents(dayOfYear, hour, min, sec);
    years_->setValue(stamp.referenceYear());
    days_->setValue(static_cast<int>(dayOfYear + 1));  // User interface see 1 to 365/366, but internal is 0 to 364/365
    hours_->setValue(static_cast<int>(hour));
    minutes_->setValue(static_cast<int>(min));
    seconds_->setValue(static_cast<int>(sec));
    if (fraction_ != NULL)
      fraction_->setValue(fractionToField_(stamp.secondsSinceRefYear()));
  }

  QValidator::State OrdinalTexts::validateText(const QString& text) const
  {
    QValidator::State lastState = SegmentedTexts::validateText(text);
    if (lastState == QValidator::Acceptable)
    {
      OrdinalTexts temp;
      temp.setTimeRange(scenarioReferenceYear_, simCore::MIN_TIME_STAMP, simCore::TimeStamp(2070, simCore::ZERO_SECONDS));
      temp.setEnforceLimits(limitBeforeStart_, limitAfterEnd_);
      temp.setTimeZone(zone_);
      temp.setText(text);
      if (!inRange_(temp.timeStamp(), true, true))  // Always color code base on the limits
        return QValidator::Intermediate;
    }

    return lastState;
  }

  void OrdinalTexts::valueEdited()
  {
    emit timeEdited(timeStamp());
  }

  void OrdinalTexts::valueChanged()
  {
    emit timeChanged(timeStamp());
  }

  void OrdinalTexts::setTimeZone(simCore::TimeZone zone)
  {
    if (zone == zone_)
      return;

    // Timestamp() is no longer correct after this line.  If timestamp must stay consistent after this call, caller must save it and restore it after calling
    zone_ = zone;
  }

  simCore::TimeZone OrdinalTexts::timeZone() const
  {
    return zone_;
  }

  //--------------------------------------------------------------------------
  MonthDayYearTexts::MonthDayYearTexts()
    : SegmentedTexts(),
      zone_(simCore::TIMEZONE_UTC)
  {
    makeSegments_();
  }

  MonthDayYearTexts::~MonthDayYearTexts()
  {
    // no need to delete
  }

  void MonthDayYearTexts::makeSegments_()
  {
    clearParts();

    month_ = new MonthText(this);
    days_ = new NumberText(this, 1, 31, 2, false, 24.0*60.0 * 60, false);
    // If the user increments the year the code will add 365*24*60*60 seconds to the current value.
    // If the time change crosses Feb 29th, the year will change by one but the day of year will also
    // change by one.   I do not think it is worth fixing.
    years_ = new NumberText(this, 1970, 2046, 4, false, 365.0*24.0*60.0*60.0, false);
    hours_ = new NumberText(this, 0, 23, 2, true, 60.0*60.0, false);
    minutes_ = new NumberText(this, 0, 59, 2, true, 60.0, false);
    seconds_ = new NumberText(this, 0, 59, 2, true, 1.0, false);
    if (precision_ != 0)
      fraction_ = createFactionOfSeconds_(precision_);
    else
      fraction_ = NULL;

    addPart(month_);
    addPart(new SeparatorText(" ", false));
    addPart(days_);
    addPart(new SeparatorText(" ", false));
    addPart(years_);
    addPart(new SeparatorText(" ", false));
    addPart(hours_);
    addPart(new SeparatorText(":", false));
    addPart(minutes_);
    addPart(new SeparatorText(":", false));
    addPart(seconds_);
    if (fraction_ != NULL)
    {
      addPart(new SeparatorText(".", true));
      addPart(fraction_);
    }
  }

  simCore::TimeStamp MonthDayYearTexts::timeStamp() const
  {
    const int yearDay = simCore::getYearDay(month_->intValue(), days_->value(), years_->value());
    const int64_t secondsIntoYear = yearDay*simCore::SECPERDAY + hours_->value()*simCore::SECPERHOUR + minutes_->value()*simCore::SECPERMIN + seconds_->value();

    const int fraction = (fraction_ == NULL) ? 0 : fractionFromField_(fraction_->value(), fraction_->text().size());
    simCore::TimeStamp stamp(years_->value(), simCore::Seconds(secondsIntoYear, fraction));

    if (zone_ == simCore::TIMEZONE_LOCAL)
    {
      // Define a UTC datetime with the timestamp
      const tm& timeComponents = simCore::getTimeStruct(stamp);
      QDateTime dateTime(QDate(1900 + timeComponents.tm_year, 1 + timeComponents.tm_mon, timeComponents.tm_mday), QTime(timeComponents.tm_hour, timeComponents.tm_min, timeComponents.tm_sec), Qt::UTC);
      // Change it to local time so that Qt figures out the local time offset from UTC time
      dateTime.setTimeSpec(Qt::LocalTime);
      stamp -= simCore::Seconds(dateTime.offsetFromUtc(), 0);
    }
    return stamp;
  }

  void MonthDayYearTexts::setTimeStamp(const simCore::TimeStamp& value)
  {
    if (!inRange_(value, limitBeforeStart_, limitAfterEnd_))
      return;

    simCore::TimeStamp stamp = value;
    if (zone_ == simCore::TIMEZONE_LOCAL)
    {
      // Define a UTC datetime with the timestamp
      const tm& timeComponents = simCore::getTimeStruct(value);
      QDateTime dateTime(QDate(1900 + timeComponents.tm_year, 1 + timeComponents.tm_mon, timeComponents.tm_mday), QTime(timeComponents.tm_hour, timeComponents.tm_min, timeComponents.tm_sec), Qt::UTC);
      // Change it to local time so that Qt figures out the local time offset from UTC time
      dateTime.setTimeSpec(Qt::LocalTime);
      stamp += simCore::Seconds(dateTime.offsetFromUtc(), 0);
    }

    try
    {
      // rounding the seconds can increase the refyear, need to reset the timestamp with rounded time to ensure no artifacts.
      stamp.setTime(stamp.referenceYear(), stamp.secondsSinceRefYear().rounded(precision_));
      unsigned int dayOfYear = 0; // [0,365] 365 possible in leap year
      unsigned int hour = 0; // hours since midnight (0..23)
      unsigned int min = 0;
      unsigned int  sec = 0;
      stamp.getTimeComponents(dayOfYear, hour, min, sec);

      int month = 0; // 0 to 11
      int dayInMonth = 0; // 1 to 31
      simCore::getMonthAndDayOfMonth(month, dayInMonth, stamp.referenceYear(), dayOfYear);

      seconds_->setValue(static_cast<int>(sec));
      minutes_->setValue(static_cast<int>(min));
      hours_->setValue(static_cast<int>(hour));
      days_->setValue(dayInMonth);
      years_->setValue(stamp.referenceYear());
      month_->setIntValue(month);
      if (fraction_ != NULL)
        fraction_->setValue(fractionToField_(stamp.secondsSinceRefYear()));
    }
    catch (const simCore::TimeException &)
    {
      // on exception, don't set anything
    }
  }

  void MonthDayYearTexts::valueEdited()
  {
    emit timeEdited(timeStamp());
  }

  void MonthDayYearTexts::valueChanged()
  {
    emit timeChanged(timeStamp());
  }

  QValidator::State MonthDayYearTexts::validateText(const QString& text) const
  {
    QValidator::State lastState = SegmentedTexts::validateText(text);
    if (lastState == QValidator::Acceptable)
    {
      MonthDayYearTexts temp;
      temp.setTimeRange(scenarioReferenceYear_, simCore::MIN_TIME_STAMP, simCore::TimeStamp(2070, simCore::ZERO_SECONDS));
      temp.setEnforceLimits(limitBeforeStart_, limitAfterEnd_);
      temp.setTimeZone(zone_);
      temp.setText(text);
      if (!inRange_(temp.timeStamp(), true, true))  // Always color code base on the limits
        return QValidator::Intermediate;
    }

    return lastState;
  }

  void MonthDayYearTexts::setTimeZone(simCore::TimeZone zone)
  {
    if (zone == zone_)
      return;

    // Timestamp() is no longer correct after this line.  If timestamp must stay consistent after this call, caller must save it and restore it after calling
    zone_ = zone;
  }

  simCore::TimeZone MonthDayYearTexts::timeZone() const
  {
    return zone_;
  }
}

