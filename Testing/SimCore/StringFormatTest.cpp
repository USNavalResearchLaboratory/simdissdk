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
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simCore/String/Format.h"

namespace
{
// This is a simple class that supports a stream operator
class Simple
{
public:
  explicit Simple(char c) : c_(c) {}
  char c_;
};

std::ostream& operator <<(std::ostream& out, const Simple& s)
{
  out << s.c_;
  return out;
}

int testJoin()
{
  int rv = 0;

  std::vector<double> empty;
  rv += SDK_ASSERT(simCore::join(empty, "") == "");

  std::vector<double> iParams = {1, 2, 3, 4};
  rv += SDK_ASSERT(simCore::join(iParams, "") == "1234");
  rv += SDK_ASSERT(simCore::join(iParams, "+") == "1+2+3+4");
  rv += SDK_ASSERT(simCore::join(iParams, " ") == "1 2 3 4");
  rv += SDK_ASSERT(simCore::join(iParams, "\"") == "1\"2\"3\"4");
  rv += SDK_ASSERT(simCore::join(iParams, "\\") == "1\\2\\3\\4");

  std::vector<char> cParams = {'a', '`', '\\', '"'};
  rv += SDK_ASSERT(simCore::join(cParams, "") == "a`\\\"");
  rv += SDK_ASSERT(simCore::join(cParams, "\\") == "a\\`\\\\\\\"");
  rv += SDK_ASSERT(simCore::join(cParams, "\\") == "a\\`\\\\\\\"");

  std::vector<double> dParams = {1., 2., 3., 4.};
  rv += SDK_ASSERT(simCore::join(dParams, "") == "1234");

  std::vector<float> fParams = {1.1f, 2.2f, 3.3f, 4.4f};
  rv += SDK_ASSERT(simCore::join(fParams, "") == "1.12.23.34.4");
  rv += SDK_ASSERT(simCore::join(fParams, ".") == "1.1.2.2.3.3.4.4");

  std::vector<Simple> simpleVector;
  rv += SDK_ASSERT(simCore::join(simpleVector, ".") == "");
  simpleVector.push_back(Simple('a'));
  simpleVector.push_back(Simple('b'));
  simpleVector.push_back(Simple('c'));
  simpleVector.push_back(Simple('d'));
  rv += SDK_ASSERT(simCore::join(simpleVector, ".") == "a.b.c.d");

  return rv;
}

int testCaseCompare()
{
  int rv = 0;
  rv += SDK_ASSERT(simCore::caseCompare("", "") == 0);
  rv += SDK_ASSERT(simCore::caseCompare("a", "") > 0);
  rv += SDK_ASSERT(simCore::caseCompare("", "B") < 0);
  rv += SDK_ASSERT(simCore::caseCompare("a", "A") == 0);
  rv += SDK_ASSERT(simCore::caseCompare("a", "B") < 0);
  rv += SDK_ASSERT(simCore::caseCompare("qB", "Qa") > 0);
  rv += SDK_ASSERT(simCore::caseCompare("a", ".") > 0);
  return rv;
}

int testLowerCase()
{
  int rv = 0;
  const std::string upper("A.B/C`D");
  const std::string lower("a.b/c`d");
  rv += SDK_ASSERT(upper != simCore::lowerCase(upper));
  rv += SDK_ASSERT(lower == simCore::lowerCase(upper));
  rv += SDK_ASSERT(lower == simCore::lowerCase(lower));
  rv += SDK_ASSERT("\\" == simCore::lowerCase("\\"));
  return rv;
}

int testUpperCase()
{
  int rv = 0;
  const std::string upper("A.B/C`D");
  const std::string lower("a.b/c`d");
  rv += SDK_ASSERT(lower != simCore::upperCase(lower));
  rv += SDK_ASSERT(upper == simCore::upperCase(lower));
  rv += SDK_ASSERT(upper == simCore::upperCase(upper));
  rv += SDK_ASSERT("\\" == simCore::upperCase("\\"));
  return rv;
}

int testStringCaseFind()
{
  int rv = 0;
  const std::string upper("A.B/C`D");
  const std::string lower("a.b/c`d");
  rv += SDK_ASSERT(0 == simCore::stringCaseFind(upper, lower));
  rv += SDK_ASSERT(0 == simCore::stringCaseFind(upper+lower, lower));
  rv += SDK_ASSERT(0 == simCore::stringCaseFind(lower+upper, lower));
  rv += SDK_ASSERT(std::string::npos == simCore::stringCaseFind("", "a"));
  rv += SDK_ASSERT(0 == simCore::stringCaseFind("", ""));
  rv += SDK_ASSERT(0 == simCore::stringCaseFind("a", ""));
  rv += SDK_ASSERT(std::string::npos == simCore::stringCaseFind("ab", "abb"));
  rv += SDK_ASSERT(0 == simCore::stringCaseFind("abb", "ABB"));
  rv += SDK_ASSERT(1 == simCore::stringCaseFind("aabb", "ABB"));
  rv += SDK_ASSERT(1 == simCore::stringCaseFind("aa.bb", "A.BB"));

  return rv;
}

int testGetExtension()
{
  int rv = 0;
  rv += SDK_ASSERT(simCore::getExtension("test.txt") == ".txt");
  rv += SDK_ASSERT(simCore::getExtension("test.TXT") == ".txt");
  rv += SDK_ASSERT(simCore::getExtension("test.txt", true) == ".txt");
  rv += SDK_ASSERT(simCore::getExtension("test.TXT", true) == ".txt");
  rv += SDK_ASSERT(simCore::getExtension("test.txt", false) == ".txt");
  rv += SDK_ASSERT(simCore::getExtension("test.TXT", false) == ".TXT");
  rv += SDK_ASSERT(simCore::getExtension("test.") == ".");
  rv += SDK_ASSERT(simCore::getExtension("test") == "");
  rv += SDK_ASSERT(simCore::getExtension("") == "");
  rv += SDK_ASSERT(simCore::getExtension("test.foo.bar") == ".bar");
  rv += SDK_ASSERT(simCore::getExtension("test.a") == ".a");
  rv += SDK_ASSERT(simCore::getExtension("test.ab,cd!ef") == ".ab,cd!ef");
  rv += SDK_ASSERT(simCore::getExtension("test.AbCdEfGhI") == ".abcdefghi");
  rv += SDK_ASSERT(simCore::getExtension("test.AbCdEfGhI", false) == ".AbCdEfGhI");
  return rv;
}

}

int StringFormatTest(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  int rv = 0;
  rv += SDK_ASSERT(testJoin() == 0);
  rv += SDK_ASSERT(testCaseCompare() == 0);
  rv += SDK_ASSERT(testLowerCase() == 0);
  rv += SDK_ASSERT(testUpperCase() == 0);
  rv += SDK_ASSERT(testStringCaseFind() == 0);

  rv += SDK_ASSERT(testGetExtension() == 0);

  std::cout << "simCore StringFormatTest " << ((rv == 0) ? "passed" : "failed") << std::endl;

  return rv;
}
