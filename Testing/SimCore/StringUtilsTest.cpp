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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/TextReplacer.h"

using namespace std;

namespace
{

int testBefore(const std::string& str, const std::string& needle, const std::string& answer, std::string answerLast="")
{
  if (answerLast.empty()) answerLast = answer;
  int rv = 0;
  if (simCore::StringUtils::before(str, needle) != answer)
  {
    cerr << "Error: before(" << str << "," << needle << ") != " << answer << endl;
    cerr << "   " << simCore::StringUtils::before(str, needle) << endl;
    rv++;
  }
  if (simCore::StringUtils::beforeLast(str, needle) != answerLast)
  {
    cerr << "Error: beforeLast(" << str << "," << needle << ") != " << answerLast << endl;
    cerr << "   " << simCore::StringUtils::beforeLast(str, needle) << endl;
    rv++;
  }
  if (needle.length() == 1)
  {
    if (simCore::StringUtils::before(str, needle[0]) != answer)
    {
      cerr << "Error: before char(" << str << "," << needle << ") != " << answer << endl;
      cerr << "   " << simCore::StringUtils::before(str, needle[0]) << endl;
      rv++;
    }
    if (simCore::StringUtils::beforeLast(str, needle[0]) != answerLast)
    {
      cerr << "Error: beforeLast char(" << str << "," << needle << ") != " << answerLast << endl;
      cerr << "   " << simCore::StringUtils::beforeLast(str, needle[0]) << endl;
      rv++;
    }
  }
  return rv;
}

int testAfter(const std::string& str, const std::string& needle, const std::string& answer, std::string answerLast="")
{
  if (answerLast.empty()) answerLast = answer;
  int rv = 0;
  if (simCore::StringUtils::after(str, needle) != answer)
  {
    cerr << "Error: after(" << str << "," << needle << ") != " << answer << endl;
    cerr << "   " << simCore::StringUtils::after(str, needle) << endl;
    rv++;
  }
  if (simCore::StringUtils::afterLast(str, needle) != answerLast)
  {
    cerr << "Error: afterLast(" << str << "," << needle << ") != " << answerLast << endl;
    cerr << "   " << simCore::StringUtils::afterLast(str, needle) << endl;
    rv++;
  }
  if (needle.length() == 1)
  {
    if (simCore::StringUtils::after(str, needle[0]) != answer)
    {
      cerr << "Error: after char(" << str << "," << needle << ") != " << answer << endl;
      cerr << "   " << simCore::StringUtils::after(str, needle[0]) << endl;
      rv++;
    }
    if (simCore::StringUtils::afterLast(str, needle[0]) != answerLast)
    {
      cerr << "Error: afterLast char(" << str << "," << needle << ") != " << answerLast << endl;
      cerr << "   " << simCore::StringUtils::afterLast(str, needle[0]) << endl;
      rv++;
    }
  }
  return rv;
}

int testSubstitute(const std::string& haystack, const std::string& needle, const std::string& repl, const std::string& answer, bool replaceAll=true)
{
  std::string rv = simCore::StringUtils::substitute(haystack, needle, repl, replaceAll);
  if (rv != answer)
  {
    cerr << "Error: substitute(" << haystack << "," << needle << "," << repl << ") != " << answer << endl;
    cerr << "   " << simCore::StringUtils::substitute(haystack, needle, repl, replaceAll) << endl;
    return 1;
  }
  return 0;
}

int testTrim()
{
  int rv = 0;
  // Trim left
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("  43") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("  43\t") == "43\t");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("  43 ") == "43 ");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("43  ") == "43  ");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("4 3") == "4 3");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft(" 4 3 ") == "4 3 ");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("43") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("   ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft(" ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("") == "");

  // Trim right
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("  43") == "  43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("  43\t") == "  43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("  43 ") == "  43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("43  ") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("4 3") == "4 3");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight(" 4 3 ") == " 4 3");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("43") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("   ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight(" ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("") == "");

  // Trim both sides
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43\t") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43 ") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("43  ") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("4 3") == "4 3");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" 4 3 ") == "4 3");
  rv += SDK_ASSERT(simCore::StringUtils::trim("43") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("   ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" ") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("Test\t") == "Test");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" Te st ") == "Te st");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\tTest") == "Test");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\t\rTest\n") == "Test");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\n") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\t") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\r") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\t   \r") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trim("\tTest newline \n in the middle\r") == "Test newline \n in the middle");

  // Irregular whitespace characters
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43", "4") == "  43");
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43\t", "4") == "  43\t");
  rv += SDK_ASSERT(simCore::StringUtils::trim("  43 ", "4") == "  43 ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("43  ", "4") == "3  ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("4 3", "4") == " 3");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" 4 3 ", "4") == " 4 3 ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("43", "4") == "3");
  rv += SDK_ASSERT(simCore::StringUtils::trim("   ", "4") == "   ");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" ", "4") == " ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("", "4") == "");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("43", "4") == "3");
  rv += SDK_ASSERT(simCore::StringUtils::trimLeft("43", "3") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("43", "4") == "43");
  rv += SDK_ASSERT(simCore::StringUtils::trimRight("43", "3") == "4");

  // More than one whitespace, irregular
  rv += SDK_ASSERT(simCore::StringUtils::trim("43  ", "43") == "  ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("4 3", "34") == " ");
  rv += SDK_ASSERT(simCore::StringUtils::trim(" 4 3 ", "43") == " 4 3 ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("aaaaahah mmmmm", "am") == "hah ");
  rv += SDK_ASSERT(simCore::StringUtils::trim("theThe", "the") == "T");
  rv += SDK_ASSERT(simCore::StringUtils::trim("theThe", "het") == "T");
  rv += SDK_ASSERT(simCore::StringUtils::trim("theThe", "eht") == "T");
  rv += SDK_ASSERT(simCore::StringUtils::trim("// Comment line", "/*# ") == "Comment line");
  rv += SDK_ASSERT(simCore::StringUtils::trim("# Comment line", "/*# ") == "Comment line");
  rv += SDK_ASSERT(simCore::StringUtils::trim("/* Comment line */", "/*# ") == "Comment line");
  rv += SDK_ASSERT(simCore::StringUtils::trim("/*   */", "/*# ") == "");
  return rv;
}

int testEscapeAndUnescape(const std::string& source, const std::string& dest)
{
  int rv = 0;

  std::string shouldMatchDest = simCore::StringUtils::addEscapeSlashes(source);
  rv += SDK_ASSERT(dest == shouldMatchDest);
  std::string shouldMatchSource = simCore::StringUtils::removeEscapeSlashes(shouldMatchDest);
  rv += SDK_ASSERT(source == shouldMatchSource);

  return rv;
}

int testEscape()
{
  int rv = 0;

  // Quotes
  rv += SDK_ASSERT(testEscapeAndUnescape("\"Quote to start", "\\\"Quote to start") == 0);                    // "Quote to start      =>  \"Quote to start
  rv += SDK_ASSERT(testEscapeAndUnescape("\"Quotes ev\"erywhere\"", "\\\"Quotes ev\\\"erywhere\\\"") == 0);  // "Quotes ev"erywhere" =>  \"Quotes ev\"erywhere\"

  // Slashes
  rv += SDK_ASSERT(testEscapeAndUnescape("\\Slash to start", "\\\\Slash to start") == 0);                     // \Slash to start       => \\Slash to start
  rv += SDK_ASSERT(testEscapeAndUnescape("\\Slashes ev\\erywhere\\", "\\\\Slashes ev\\\\erywhere\\\\") == 0); // \Slashes ev\erywhere\ => \\Slahes ev\\erywhere\\    eol

  // Both Quotes and Slashes
  // Both \"slashes" and quotes\   =>    Both \\\"slashes\" and quotes\\    eol
  std::string ans = "Both \\\\";
  ans.append("\\\"slashes\\\" and quotes\\\\");
  rv += SDK_ASSERT(testEscapeAndUnescape("Both \\\"slashes\" and quotes\\", ans) == 0);

  // Real use cases
  rv += SDK_ASSERT(testEscapeAndUnescape("^Test \\(GPS\\)", "^Test \\\\(GPS\\\\)") == 0);                     // ^Test \(GPS\)       =>   ^Test \\(GPS\\)
  rv += SDK_ASSERT(testEscapeAndUnescape("^Test \\(GPS\\)\"", "^Test \\\\(GPS\\\\)\\\"") == 0);               // ^Test \(GPS\)"      =>   ^Test \\(GPS\\)\"

  // Test \n
  rv += SDK_ASSERT(testEscapeAndUnescape("\n", "\\0xA") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\nText", "\\0xAText") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("Text\nText", "Text\\0xAText") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("Text\n", "Text\\0xA") == 0);

  rv += SDK_ASSERT(testEscapeAndUnescape("\"\n\"", "\\\"\\0xA\\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\" \n\"", "\\\" \\0xA\\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\"\n \"", "\\\"\\0xA \\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\" \n \"", "\\\" \\0xA \\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\"\nText\"", "\\\"\\0xAText\\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\"Text\nText\"", "\\\"Text\\0xAText\\\"") == 0);
  rv += SDK_ASSERT(testEscapeAndUnescape("\"Text\n\"", "\\\"Text\\0xA\\\"") == 0);

  return rv;
}

int testToNativeSeparators()
{
  int rv = 0;

#ifdef WIN32
  rv += SDK_ASSERT(simCore::toNativeSeparators("./test/file") == ".\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("./test\\file") == ".\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators(".\\test\\file") == ".\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators(".\\test/file") == ".\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\test\\file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:/test/file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/file") == "\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/path/") == "\\test\\path\\");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/path\\\\") == "\\test\\path\\");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:/test/\\/file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test//file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test///file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test////file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test/////file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\\\file") == "c:\\test\\file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\\\\\file") == "c:\\test\\file");
  // UNC addresses should work
  rv += SDK_ASSERT(simCore::toNativeSeparators("\\\\test") == "\\\\test");
  rv += SDK_ASSERT(simCore::toNativeSeparators("\\\\test\\test") == "\\\\test\\test");
  rv += SDK_ASSERT(simCore::toNativeSeparators("\\\\test\\\\test") == "\\\\test\\test");
  rv += SDK_ASSERT(simCore::toNativeSeparators("//test") == "\\\\test");
  rv += SDK_ASSERT(simCore::toNativeSeparators("//test/test") == "\\\\test\\test");
  rv += SDK_ASSERT(simCore::toNativeSeparators("//test//test") == "\\\\test\\test");
#else
  rv += SDK_ASSERT(simCore::toNativeSeparators("./test/file") == "./test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("./test\\file") == "./test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators(".\\test\\file") == "./test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators(".\\test/file") == "./test/file");
  // Note that Linux does not attempt to correct "C:/" and leaves it in
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\test\\file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:/test/file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/file") == "/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/path/") == "/test/path/");
  rv += SDK_ASSERT(simCore::toNativeSeparators("/test/path\\\\") == "/test/path/");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:/test/\\/file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test//file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test///file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test////file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test/////file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\\\file") == "c:/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("c:\\/test\\\\\\\\file") == "c:/test/file");
  // UNC addresses should work
  rv += SDK_ASSERT(simCore::toNativeSeparators("\\\\test") == "//test");
  rv += SDK_ASSERT(simCore::toNativeSeparators("\\\\test\\test") == "//test/test");
  rv += SDK_ASSERT(simCore::toNativeSeparators("\\\\test\\\\test") == "//test/test");
  rv += SDK_ASSERT(simCore::toNativeSeparators("//test") == "//test");
  rv += SDK_ASSERT(simCore::toNativeSeparators("//test/test") == "//test/test");
  rv += SDK_ASSERT(simCore::toNativeSeparators("//test//test") == "//test/test");
#endif
  // URLs should not get mangled in any way
  rv += SDK_ASSERT(simCore::toNativeSeparators("https://simdis.nrl.navy.mil/jira") == "https://simdis.nrl.navy.mil/jira");
  rv += SDK_ASSERT(simCore::toNativeSeparators("https://simdis.nrl.navy.mil\\jira") == "https://simdis.nrl.navy.mil\\jira");
  rv += SDK_ASSERT(simCore::toNativeSeparators("file:///home/test/file") == "file:///home/test/file");
  rv += SDK_ASSERT(simCore::toNativeSeparators("file:///home/test\\file") == "file:///home/test\\file");
  return rv;
}

int testBeforeAfter()
{
  int rv = 0;
  // Standard case
  rv += SDK_ASSERT(testBefore("foobar=baz", "=", "foobar") == 0);
  rv += SDK_ASSERT(testAfter("foobar=baz", "=", "baz") == 0);
  // Bound 0
  rv += SDK_ASSERT(testBefore("=baz", "=", "") == 0);
  rv += SDK_ASSERT(testAfter("=baz", "=", "baz") == 0);
  // Bound last
  rv += SDK_ASSERT(testBefore("foobar=", "=", "foobar") == 0);
  rv += SDK_ASSERT(testAfter("foobar=", "=", "") == 0);
  // Bound outside
  rv += SDK_ASSERT(testBefore("foobar", "=", "foobar") == 0);
  rv += SDK_ASSERT(testAfter("foobar", "=", "") == 0);
  // Bound double
  rv += SDK_ASSERT(testBefore("foobar=baz=zoo", "=", "foobar", "foobar=baz") == 0);
  rv += SDK_ASSERT(testAfter("foobar=baz=zoo", "=", "baz=zoo", "zoo") == 0);
  // Multi-char delimiter
  rv += SDK_ASSERT(testBefore("foobar:;:baz:;:zoo", ":;:", "foobar", "foobar:;:baz") == 0);
  rv += SDK_ASSERT(testAfter("foobar:;:baz:;:zoo", ":;:", "baz:;:zoo", "zoo") == 0);
  return rv;
}

int testSubstitute()
{
  int rv = 0;
  // Simple substitute
  rv += SDK_ASSERT(testSubstitute("foobar", "bar", "baz", "foobaz") == 0);
  // Double substitute
  rv += SDK_ASSERT(testSubstitute("barfoobar", "bar", "baz", "bazfoobaz") == 0);
  // Substitute with needle in replacement pattern
  rv += SDK_ASSERT(testSubstitute("barbara", "bar", "xxxxbary", "xxxxbaryxxxxbarya") == 0);
  // Single substitution
  rv += SDK_ASSERT(testSubstitute("barbara", "bar", "zoo", "zoobara", false) == 0);
  return rv;
}

int checkStrings(const std::string& expected, const std::string& str)
{
  if (expected != str)
  {
    std::cerr << "Strings do not match: >" << str << "< -- expected: >" << expected << "<" << std::endl;
    return 1;
  }
  return 0;
}

int checkStrings2(const std::string& option1, const std::string& option2, const std::string& str)
{
  if (option1 == str || option2 == str)
    return 0;
  std::cerr << "Strings do not match: >" << str << "< -- expected: >" << option1 << "< or >" << option2 << "<" << std::endl;
  return 1;
}

int buildFormatStrTest()
{
  int rv = 0;
  // Scientific tests -- different build systems give different e+00 or e+000 results
  rv += SDK_ASSERT(0 == checkStrings2("1.52e+025", "1.52e+25", simCore::buildString("", 1.52103484e25, 0, 2, "", false)));
  rv += SDK_ASSERT(0 == checkStrings2("-1.52e+025", "-1.52e+25", simCore::buildString("", -1.52103484e25, 0, 2, "", false)));
  rv += SDK_ASSERT(0 == checkStrings2("1.52e-025", "1.52e-25", simCore::buildString("", 1.52103484e-25, 0, 2, "", false)));
  rv += SDK_ASSERT(0 == checkStrings2("-1.52e-025", "-1.52e-25", simCore::buildString("", -1.52103484e-25, 0, 2, "", false)));
  // Regular tests
  rv += SDK_ASSERT(0 == checkStrings("0", simCore::buildString("", 0.0, 0, 0, "", false)));
  rv += SDK_ASSERT(0 == checkStrings("15.21", simCore::buildString("", 1.52103484e1, 0, 2, "", false)));
  rv += SDK_ASSERT(0 == checkStrings("-15.21", simCore::buildString("", -1.52103484e1, 0, 2, "", false)));
  // NaN and inf tests
  rv += SDK_ASSERT(0 == checkStrings("NaN", simCore::buildString("", std::numeric_limits<double>::quiet_NaN(), 0, 2, "", false)));
  rv += SDK_ASSERT(0 == checkStrings("inf", simCore::buildString("", std::numeric_limits<double>::infinity(), 0, 2, "", false)));
  return rv;
}

// Create an undefined-variables helper; it effectively replaces '%' with '-'
class TestUndefinedHelper : public simCore::TextReplacer::UndefinedVariableHandler
{
public:
  // Add extra percent signs so that incoming value's percent signs get interpreted as text, not variable markers.
  virtual std::string getText(const std::string& varName) const { return "-%" + varName + "%-"; }
};

int testTextReplacer()
{
  /** Create a custom replaceable that returns any text desired */
  class CustomReplaceable : public simCore::TextReplacer::Replaceable
  {
  public:
    CustomReplaceable(const std::string& varName, const std::string& textValue)
      : varName_(varName),
        textValue_(textValue)
    {
    }
    virtual std::string getText() const { return textValue_; }
    virtual std::string getVariableName() const { return varName_; }
  private:
    std::string varName_;
    std::string textValue_;
  };

  simCore::TextReplacer replacer;
  int rv = 0;
  // Variable does not exist; it does not get replaced (by default, from default unknown-handler)
  rv += SDK_ASSERT(replacer.format("test %VAR% 123") == "test %VAR% 123");
  // Built-in support for %%
  rv += SDK_ASSERT(replacer.format("test %% 123") == "test % 123");
  // Non-matching %
  rv += SDK_ASSERT(replacer.format("test % 123") == "test % 123");
  // Non-matching %, with %% next to it
  rv += SDK_ASSERT(replacer.format("test %%% 123") == "test %% 123");

  // Test a bunch of failures on addReplaceable
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(nullptr) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%VAR", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%VAR%", "foo")) == 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("VAR%", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("V%AR", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("VAR", "foo")) == 0);
  // Adding a new one should not leak, and should succeed
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("VAR", "foo")) == 0);
  rv += SDK_ASSERT(replacer.format("test %VAR% 123") == "test foo 123");
  // Remove it and make sure that the replacement is gone
  rv += SDK_ASSERT(replacer.deleteReplaceable("") != 0);
  rv += SDK_ASSERT(replacer.deleteReplaceable("DOESNOTEXIST") != 0);
  rv += SDK_ASSERT(replacer.deleteReplaceable("VAR") == 0);
  // Should not be able to delete twice
  rv += SDK_ASSERT(replacer.deleteReplaceable("VAR") != 0);

  // Test the deleteReplaceable() with pointers
  rv += SDK_ASSERT(replacer.deleteReplaceable(nullptr) != 0);
  auto* fooVariable = new CustomReplaceable("VAR", "foo");
  rv += SDK_ASSERT(replacer.addReplaceable(fooVariable) == 0);
  rv += SDK_ASSERT(replacer.deleteReplaceable(nullptr) != 0);
  rv += SDK_ASSERT(replacer.deleteReplaceable(fooVariable) == 0);
  // Should not stay in the list after last command
  rv += SDK_ASSERT(replacer.deleteReplaceable("VAR") != 0);

  // The memory in fooVariable is considered deleted and is now invalid
  fooVariable = nullptr;
  rv += SDK_ASSERT(replacer.format("test %VAR% 123") == "test %VAR% 123");
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("VAR", "baz")) == 0);
  rv += SDK_ASSERT(replacer.format("test %VAR% 123") == "test baz 123");
  // Should be able to overwrite existing "VAR" string
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("VAR", "baz")) == 0);

  // Make sure it can replace more than once
  rv += SDK_ASSERT(replacer.format("test %VAR%%VAR% 123") == "test bazbaz 123");
  rv += SDK_ASSERT(replacer.format("test %VAR% %VAR% 123") == "test baz baz 123");
  // Ensure %% resolves to %
  rv += SDK_ASSERT(replacer.format("test %VAR% %% %VAR% 123") == "test baz % baz 123");
  // Mess up the percents
  rv += SDK_ASSERT(replacer.format("test %VAR%%%%VAR% 123") == "test baz%baz 123");
  rv += SDK_ASSERT(replacer.format("test %VAR%%%VAR% 123") == "test baz%VAR% 123");

  // Beginning of string
  rv += SDK_ASSERT(replacer.format("%VAR% %% 123") == "baz % 123");
  rv += SDK_ASSERT(replacer.format("%% %VAR% 123") == "% baz 123");
  // End of string
  rv += SDK_ASSERT(replacer.format("123 %VAR% %%") == "123 baz %");
  rv += SDK_ASSERT(replacer.format("123 %% %VAR%") == "123 % baz");
  // Both
  rv += SDK_ASSERT(replacer.format("%VAR% %VAR%") == "baz baz");
  rv += SDK_ASSERT(replacer.format("%% %%") == "% %");

  // Check that we can add the another variable with a different name
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("TEST", "baz2")) == 0);
  rv += SDK_ASSERT(replacer.format("%TEST% %VAR%") == "baz2 baz");
  rv += SDK_ASSERT(replacer.format("%TEST2% %TEST%") == "%TEST2% baz2");

  replacer.setUndefinedVariableHandler(std::make_shared<TestUndefinedHelper>());
  auto str = replacer.format("test %VAR% %VAR% %NOTHING% 123");
  rv += SDK_ASSERT(replacer.format("test %VAR% %VAR% %NOTHING% 123") == "test baz baz -%NOTHING%- 123");
  rv += SDK_ASSERT(replacer.format("test %VAR% %% %VAR% %NOTHING% 123") == "test baz % baz -%NOTHING%- 123");

  // Clear out the handler and retest
  replacer.setUndefinedVariableHandler(std::shared_ptr<simCore::TextReplacer::UndefinedVariableHandler>());
  rv += SDK_ASSERT(replacer.format("test %VAR% %VAR% %NOTHING% 123") == "test baz baz  123");
  rv += SDK_ASSERT(replacer.format("test %VAR% %% %VAR% %NOTHING% 123") == "test baz % baz  123");

  // Test getVariableName() with funky inputs.  Start with known good, then divert into bad territory
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%VAR%", "foo")) == 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("VAR", "foo")) == 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%VAR", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("VAR%", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%V%AR%", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("V%AR%", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%V%AR", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("V%AR", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("V%%AR", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%%", "foo")) != 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%", "foo")) != 0);

  // Handle embedded variables
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("EMBED1", "embed1")) == 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%EMBED2%", "embed2")) == 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%VAR%", "A:%EMBED2% B:%EMBED1%")) == 0);
  rv += SDK_ASSERT(replacer.format("> %VAR% <") == "> A:embed2 B:embed1 <");
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%VAR%", "A:%EMBED2%%% B:%EMBED1%")) == 0);
  rv += SDK_ASSERT(replacer.format("> %VAR% <") == "> A:embed2% B:embed1 <");

  // Make sure embedded variables can't cause infinite recursion
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%VAR%", "%EMBED2%")) == 0);
  rv += SDK_ASSERT(replacer.addReplaceable(new CustomReplaceable("%EMBED2%", "%VAR%")) == 0);
  const std::string recursed = replacer.format("%VAR%");
  rv += SDK_ASSERT(recursed == "%VAR%" || recursed == "%EMBED2%");

  return rv;
}

int testRemoveTrailingZeros()
{
  int rv = 0;

  rv += SDK_ASSERT(simCore::removeTrailingZeros("100.000", false) == "100");
  rv += SDK_ASSERT(simCore::removeTrailingZeros("100.000", true) == "100.");
  rv += SDK_ASSERT(simCore::removeTrailingZeros("100.0001", false) == "100.0001");
  rv += SDK_ASSERT(simCore::removeTrailingZeros("100.000100", false) == "100.0001");
  rv += SDK_ASSERT(simCore::removeTrailingZeros(".000", false) == "");
  rv += SDK_ASSERT(simCore::removeTrailingZeros(".000", true) == ".");
  rv += SDK_ASSERT(simCore::removeTrailingZeros("100.", false) == "100");
  rv += SDK_ASSERT(simCore::removeTrailingZeros("100.", true) == "100.");
  rv += SDK_ASSERT(simCore::removeTrailingZeros("100", false) == "100");
  rv += SDK_ASSERT(simCore::removeTrailingZeros("not a number", false) == "not a number");
  rv += SDK_ASSERT(simCore::removeTrailingZeros("not a number", true) == "not a number");

  return rv;
}

}

int StringUtilsTest(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  int rv = 0;

  rv += SDK_ASSERT(testBeforeAfter() == 0);
  rv += SDK_ASSERT(testSubstitute() == 0);

  // Test trimming methods (trimLeft, trimRight, trim)
  rv += SDK_ASSERT(testTrim() == 0);

  // Test adding escape slashes
  rv += SDK_ASSERT(testEscape() == 0);

  // Test the to-native-path code
  rv += SDK_ASSERT(testToNativeSeparators() == 0);

  // buildString() testing
  rv += SDK_ASSERT(buildFormatStrTest() == 0);

  // TextReplacer testing
  rv += SDK_ASSERT(testTextReplacer() == 0);

  rv += SDK_ASSERT(testRemoveTrailingZeros() == 0);

  std::cout << "simCore StringUtilsTest " << ((rv == 0) ? "passed" : "failed") << std::endl;

  return rv;
}
