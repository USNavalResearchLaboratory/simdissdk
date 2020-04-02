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
#include <limits>
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

int testGetStrippedLine()
{
  int rv = 0;
  std::stringstream ss;
  std::istringstream is;
  std::string out;

  rv += SDK_ASSERT(!simCore::getStrippedLine(ss, out));
  rv += SDK_ASSERT(!simCore::getStrippedLine(is, out));
  ss << " ";
  rv += SDK_ASSERT(!simCore::getStrippedLine(ss, out));
  is = std::istringstream("\0");
  rv += SDK_ASSERT(!simCore::getStrippedLine(is, out));
  ss = std::stringstream("\0");
  rv += SDK_ASSERT(!simCore::getStrippedLine(ss, out));

#ifdef WIN32
  is = std::istringstream(" ");
  rv += SDK_ASSERT(simCore::getStrippedLine(is, out));
  rv += SDK_ASSERT(out.empty());
  ss = std::stringstream(" ");
  rv += SDK_ASSERT(simCore::getStrippedLine(ss, out));
  rv += SDK_ASSERT(out.empty());
  is = std::istringstream(" \n\r\t");
  rv += SDK_ASSERT(simCore::getStrippedLine(is, out));
  rv += SDK_ASSERT(out.empty());

  is = std::istringstream("a \n\r\t");
  rv += SDK_ASSERT(simCore::getStrippedLine(is, out));
  rv += SDK_ASSERT(out == "a");
  is = std::istringstream("a \n\r\ta");
  rv += SDK_ASSERT(simCore::getStrippedLine(is, out));
  rv += SDK_ASSERT(out == "a");
  is = std::istringstream("a \r\ta");
  rv += SDK_ASSERT(simCore::getStrippedLine(is, out));
  rv += SDK_ASSERT(out == "a \r\ta");

  is = std::istringstream("a a");
  rv += SDK_ASSERT(simCore::getStrippedLine(is, out));
  rv += SDK_ASSERT(out == "a a");
  is = std::istringstream("a\ta");
  rv += SDK_ASSERT(simCore::getStrippedLine(is, out));
  rv += SDK_ASSERT(out == "a\ta");
  is = std::istringstream("a \ra");
  rv += SDK_ASSERT(simCore::getStrippedLine(is, out));
  rv += SDK_ASSERT(out == "a \ra");
  is = std::istringstream("a \na");
  rv += SDK_ASSERT(simCore::getStrippedLine(is, out));
  rv += SDK_ASSERT(out == "a");
  is = std::istringstream("a \n\na");
  rv += SDK_ASSERT(simCore::getStrippedLine(is, out));
  rv += SDK_ASSERT(out == "a");
#endif
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

int testHasExtension()
{
  int rv = 0;
  rv += SDK_ASSERT(simCore::hasExtension("", ""));
  rv += SDK_ASSERT(simCore::hasExtension(".", "."));
  rv += SDK_ASSERT(simCore::hasExtension(".t", ".t"));
  rv += SDK_ASSERT(simCore::hasExtension("t.", "."));
  rv += SDK_ASSERT(simCore::hasExtension("t. ", ". "));
  rv += SDK_ASSERT(simCore::hasExtension("t. \n\r\t", ". \n\r\t"));
  rv += SDK_ASSERT(simCore::hasExtension(" \n\r\t.txt", ".tXt"));

  rv += SDK_ASSERT(!simCore::hasExtension(".", ""));
  rv += SDK_ASSERT(!simCore::hasExtension("test.", ""));
  rv += SDK_ASSERT(!simCore::hasExtension("", "."));
  rv += SDK_ASSERT(!simCore::hasExtension("test.txt", ""));
  rv += SDK_ASSERT(!simCore::hasExtension("test.txt", "txt"));
  rv += SDK_ASSERT(!simCore::hasExtension("test.txt", ".tx"));
  rv += SDK_ASSERT(!simCore::hasExtension("test.txt", "..txt"));
  rv += SDK_ASSERT(!simCore::hasExtension("test.txt", ".txt."));
  rv += SDK_ASSERT(!simCore::hasExtension("test.txt", "test.txt"));

  rv += SDK_ASSERT(simCore::hasExtension("tes.t.txt", ".txt"));
  rv += SDK_ASSERT(!simCore::hasExtension("tes.t.txt", ".t.txt"));
  rv += SDK_ASSERT(!simCore::hasExtension("test.txt.", ".txt"));
  rv += SDK_ASSERT(simCore::hasExtension("test.txt.", "."));

  rv += SDK_ASSERT(simCore::hasExtension("test.txt", ".TXT"));
  rv += SDK_ASSERT(simCore::hasExtension("test.txt", ".tXt"));
  rv += SDK_ASSERT(simCore::hasExtension("test.TxT", ".txt"));
  rv += SDK_ASSERT(simCore::hasExtension("test.TxT", ".tXt"));
  rv += SDK_ASSERT(simCore::hasExtension("test.TXT", ".txt"));
  rv += SDK_ASSERT(simCore::hasExtension("test.TXT", ".tXt"));
  rv += SDK_ASSERT(simCore::hasExtension("test.TXT", ".TXT"));

  return rv;
}

int testBuildString()
{
  int rv = 0;

  // field width, precision, and padZero tests
  rv += SDK_ASSERT("123456789" == simCore::buildString("", 123456789.123456789, 0, 0));
  rv += SDK_ASSERT("123456789" == simCore::buildString("", 123456789.123456789, 1, 0));
  rv += SDK_ASSERT(" 123456789" == simCore::buildString("", 123456789.123456789, 10, 0));
  rv += SDK_ASSERT("123456789.1" == simCore::buildString("", 123456789.123456789, 10, 1));
  rv += SDK_ASSERT("0123456789" == simCore::buildString("", 123456789.123456789, 10, 0, "", true));
  rv += SDK_ASSERT("123456789.1" == simCore::buildString("", 123456789.123456789, 10, 1, "", true));
  rv += SDK_ASSERT("123456789.1" == simCore::buildString("", 123456789.123456789, 11, 1, "", true));
  rv += SDK_ASSERT("0123456789.1" == simCore::buildString("", 123456789.123456789, 12, 1, "", true));

  // rounding
#ifdef WIN32
  rv += SDK_ASSERT("1" == simCore::buildString("", 0.5, 1, 0));
#else
  rv += SDK_ASSERT("0" == simCore::buildString("", 0.5, 1, 0));
  rv += SDK_ASSERT("1" == simCore::buildString("", 0.5 + std::numeric_limits<double>::epsilon(), 1, 0));
#endif
  rv += SDK_ASSERT("0" == simCore::buildString("", 0.5 - std::numeric_limits<double>::epsilon(), 1, 0));
  rv += SDK_ASSERT("1.0" == simCore::buildString("", 0.99, 1, 1));

  // precision limits matter
  rv += SDK_ASSERT("abcdefg123456789.1234567910" == simCore::buildString("abcdefg", 123456789.123456789, 10, 10));

  // prefix and suffix
  rv += SDK_ASSERT("abcdefg123456789.1" == simCore::buildString("abcdefg", 123456789.123456789, 1, 1));
  rv += SDK_ASSERT(" \n\r\t123456789.1 \n\r\t" == simCore::buildString(" \n\r\t", 123456789.123456789, 1, 1, " \n\r\t"));
  rv += SDK_ASSERT("abcdefg 123456789" == simCore::buildString("abcdefg", 123456789.123456789, 10, 0));
  rv += SDK_ASSERT("abcdefg0123456789" == simCore::buildString("abcdefg", 123456789.123456789, 10, 0, "", true));

  // scientific notation
  rv += SDK_ASSERT("0" == simCore::buildString("", 0.0, 1, 0, "", false, 1., 1.));
  rv += SDK_ASSERT("0.0" == simCore::buildString("", 0.0, 1, 1, "", false, 1., 1.));
  rv += SDK_ASSERT("1.0e-01" == simCore::buildString("", 0.1, 1, 1, "", false, 1., 1.));

  rv += SDK_ASSERT("1" == simCore::buildString("", 1.0, 1, 0, "", false, 1., 1.));

  // windows and linux have different interpretations of 0 precision in scientific notation
#ifdef WIN32
  rv += SDK_ASSERT("1.000000e+00" == simCore::buildString("", 1.0, 1, 0, "", false, 0., 1.));
  rv += SDK_ASSERT(" 1.000000e+00" == simCore::buildString("", 1.0, 13, 0, "", false, 1. - std::numeric_limits<double>::epsilon(), 1));
  rv += SDK_ASSERT(" 1.000000e+00" == simCore::buildString("", 1.0, 13, 0, "", false, 1., 1. + std::numeric_limits<double>::epsilon()));
#else
  rv += SDK_ASSERT("1e+00" == simCore::buildString("", 1.0, 1, 0, "", false, 0., 1.));
  rv += SDK_ASSERT("        1e+00" == simCore::buildString("", 1.0, 13, 0, "", false, 1. - std::numeric_limits<double>::epsilon(), 1));
  rv += SDK_ASSERT("        1e+00" == simCore::buildString("", 1.0, 13, 0, "", false, 1., 1. + std::numeric_limits<double>::epsilon()));
#endif

  rv += SDK_ASSERT("1.0e+00" == simCore::buildString("", 1.0, 1, 1, "", false, 0., 1.));
  rv += SDK_ASSERT("1.23456789e+00" == simCore::buildString("", 1.23456789, 10, 8, "", false, 1., 1.));
  rv += SDK_ASSERT("1.2345679e+00" == simCore::buildString("", 1.23456789, 10, 7, "", false, 1., 1.));

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
  rv += SDK_ASSERT(testGetStrippedLine() == 0);
  rv += SDK_ASSERT(testGetExtension() == 0);
  rv += SDK_ASSERT(testHasExtension() == 0);
  rv += SDK_ASSERT(testBuildString() == 0);
  std::cout << "simCore StringFormatTest " << ((rv == 0) ? "passed" : "failed") << std::endl;

  return rv;
}
