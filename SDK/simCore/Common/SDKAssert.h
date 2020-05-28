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
#ifndef SIMCORE_COMMON_SDKASSERT_H
#define SIMCORE_COMMON_SDKASSERT_H

#include <iostream>
#include <string>
#define SDK_ASSERT(COND) SDK_ASSERT_TEST(COND, __LINE__, #COND)

/**
* @brief Assertion test function that prints the line number and text if the condition fails
*
* Useful for testing assertions, printing the line and information for the assertion failure.
* This is particularly useful for applications in the Testing/ directory.
* @param[in ] condition Boolean value to test
* @param[in ] lineNum Line number where condition failed
* @param[in ] text Information describing the assertion condition
* @return 0:success, 1:failure
*/
inline int SDK_ASSERT_TEST(bool condition, int lineNum, const std::string& text)
{
  if (!condition)
  {
    std::cerr << "Line " << lineNum << " Failed: " << text << std::endl;
    return 1;
  }
  return 0;
}

#endif /* SIMCORE_COMMON_SDKASSERT_H */
