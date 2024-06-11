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
#include <cstdlib>
#include <string>
#include "simCore/Common/SDKAssert.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"
#include "simQt/RegExpImpl.h"


namespace
{
std::string NumSeriesToRegExp(const std::string& numSeries)
{
  // Vector containing the separated series of values.
  std::vector<std::string> regVec;
  simCore::stringTokenizer(regVec, numSeries, ",", false);
  std::string regExp = "";

  regExp += "^0*(";
  for (unsigned int i = 0; i < regVec.size(); i++)
  {
    // If there is a '-' in the string, it's a range of values.
    if (regVec[i].find('-') != std::string::npos)
    {
      // minimum and maximum values for the desired range
      std::string min = simCore::StringUtils::trim(simCore::StringUtils::before(regVec[i], '-').c_str());
      std::string max = simCore::StringUtils::trim(simCore::StringUtils::after(regVec[i], '-').c_str());

      // Fill min with leading 0's to match the number of digits in max
      min = std::string(max.size() - min.size(), '0') + min;

      // Index pointing to the one digit that might be something other than fixed (left) or [0-9] (right)
      unsigned int rangeIndex = static_cast<int>(min.size()) - 1;
      // Index prior to which the two numbers are equal
      unsigned int equalIndex = 0;
      while (min[equalIndex] == max[equalIndex])
        ++equalIndex;

      // First pass is getting the minimum range to the same size as the maximum.
      bool firstPass = true;
      bool firstLoop = true;
      std::string currentMax = min;

      // Need equal clause to account for RegExp's like 1-10, which translates to [1-9]|10
      while (min.compare(max) <= 0)
      {
        if (!firstLoop)
        {
          regExp += '|';
        }
        else
          firstLoop = false;

        if (rangeIndex == equalIndex)
          firstPass = false;

        // If the min is fewer digits than the max, the range at the index will be [0-9].
        if (firstPass && rangeIndex != 0)
          currentMax[rangeIndex] = '9';
        else
        {
          // Ignore any leading 0's
          if (rangeIndex == 0)
            while (max[rangeIndex] == '0')
              ++rangeIndex;
          // Pass any portion of the numbers that already match
          while ((rangeIndex < max.size() - 1) && (min[rangeIndex] == max[rangeIndex]))
          {
            ++rangeIndex;
            firstPass = false;
          }
          if (rangeIndex != max.size() - 1)
            currentMax[rangeIndex] = max[rangeIndex] - 1;
          else
            currentMax[rangeIndex] = max[rangeIndex];
        }
        // Any digits after the index will have a range of [0-9].
        for (unsigned int i = rangeIndex + 1; i < currentMax.size(); i++)
          currentMax[i] = '9';

        // Add portion of number that doesn't change for this range.
        bool leadingZeros = true;
        for (unsigned int i = 0; i < rangeIndex; i++)
        {
          // Skip over leading zeros
          if (currentMax[i] != '0' || !leadingZeros)
          {
            regExp += currentMax[i];
            leadingZeros = false;
          }
        }
        // Add the first range. Need to cast the first argument as a string in order to concatenate with +.
        if (min[rangeIndex] != currentMax[rangeIndex])
          regExp += static_cast<std::string>("[") + min[rangeIndex] + '-' + currentMax[rangeIndex] + ']';
        else
          regExp += currentMax[rangeIndex];
        // The rest of the ranges should be [0-9].
        for (unsigned int i = rangeIndex + 1; i < max.size(); i++)
          regExp += "[0-9]";

        // If in the first pass, go backward through the numbers, otherwise go forward
        if (firstPass)
        {
          if (rangeIndex > equalIndex)
            --rangeIndex;
          else // If we've hit the equal rangeIndex, start going forward
          {
            ++rangeIndex;
            firstPass = false;
          }
        }
        else
          ++rangeIndex;

        // This is equivalent to adding 1 to the current minimum.
        size_t j = currentMax.size() - 1;
        while (currentMax[j] == '9')
        {
          currentMax[j] = '0';
          --j;
        }
        ++currentMax[j];

        min = currentMax;
      }
    }
    else
    {
      regExp += simCore::StringUtils::trimLeft(regVec[i], "0 ");
    }
    if (i < regVec.size()-1)
      regExp += '|';
  }
  regExp += ")$";

  return regExp;
}

int RangeMatchTest(const std::string& series)
{
  std::string regExp = NumSeriesToRegExp(series);
  simQt::RegExpImpl numRegExp(regExp);
  // Vector containing the separated series of values.
  std::vector<std::string> rangeVec;
  simCore::stringTokenizer(rangeVec, series, ",", false);
  int rv = 0;

  for (unsigned int i = 0; i < rangeVec.size(); i++)
  {
    int errs = 0;
    // If there is a '-' in the string, it's a range of values.
    if (rangeVec[i].find('-') != std::string::npos)
    {
      long long min = atoi(simCore::StringUtils::trim(simCore::StringUtils::before(rangeVec[i], '-')).c_str());
      long long max = atoi(simCore::StringUtils::trim(simCore::StringUtils::after(rangeVec[i], '-')).c_str());

      // Should not match outside of range, but should match all within range
      errs += SDK_ASSERT(!numRegExp.match(std::to_string(min - 1)));
      for (long long i = min; i <= max; i++)
        errs += SDK_ASSERT(numRegExp.match(std::to_string(i)));
      errs += SDK_ASSERT(!numRegExp.match(std::to_string(max + 1)));

      // leading 0's should not affect matching
      errs += SDK_ASSERT(!numRegExp.match("0000" + std::to_string(max + 1)));
      errs += SDK_ASSERT(!numRegExp.match("0000" + std::to_string(min - 1)));
      errs += SDK_ASSERT(numRegExp.match("0000" + std::to_string(max)));
      errs += SDK_ASSERT(numRegExp.match("0000" + std::to_string(min)));
    }
    else
    {
      long long val = atoi(rangeVec[i].c_str());
      errs += SDK_ASSERT(!numRegExp.match(std::to_string(val - 1)));
      errs += SDK_ASSERT(numRegExp.match(std::to_string(val)));
      errs += SDK_ASSERT(!numRegExp.match(std::to_string(val + 1)));
    }
    if (errs != 0)
    {
      std::cerr << "Match failed." << std::endl;
      std::cerr << "Series: " << series << " (" << rangeVec[i] << ')' << std::endl;
      std::cerr << "RegExp: " << regExp << std::endl;
    }
    rv += errs;
  }
  return rv;
}

}

//--------------------------------------------------------------------
int RangeToRegExpTest(int argc, char *argv[])
{
  int rv = 0;

  // Testing multiple values and ranges in the same expression
  rv += SDK_ASSERT(RangeMatchTest("123-124, 459-512, 7-105, 593") == 0);

  // Testing for a wide range of values
  rv += SDK_ASSERT(RangeMatchTest("27-5021") == 0);

  // Testing for some extra edge cases
  rv += SDK_ASSERT(RangeMatchTest("27-5020") == 0);

  // Testing for handling of large values as well as numbers with equal first few digits
  rv += SDK_ASSERT(RangeMatchTest("505039-506299") == 0);

  // Handling of spaces and leading zeros
  rv += SDK_ASSERT(RangeMatchTest("0072, 1234, 3400 - 3476, 6100 - 6110") == 0);

  // Some more edge cases
  rv += SDK_ASSERT(RangeMatchTest("89-105") == 0);
  rv += SDK_ASSERT(RangeMatchTest("0-51") == 0);
  rv += SDK_ASSERT(RangeMatchTest("27-1121") == 0);
  rv += SDK_ASSERT(RangeMatchTest("400-500") == 0);
  rv += SDK_ASSERT(RangeMatchTest("99-199") == 0);
  rv += SDK_ASSERT(RangeMatchTest("032, 100-110, 450-455") == 0);

  return rv;
}
