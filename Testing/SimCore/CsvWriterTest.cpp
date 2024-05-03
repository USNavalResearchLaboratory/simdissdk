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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <sstream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/String/CsvReader.h"
#include "simCore/String/CsvWriter.h"

namespace {

/**
 * Helper that tests a single vector, checking for expected output and optionally running
 * the output of the writer, through the CSV reader, to confirm bidirectionality.
 * @param vec Vector of tokens to write to default CSV format
 * @param expected String representation of the tokens, expected for CSV-ification. Should end in newline.
 * @param testCsvReader If true, run the output through CSV Reader to confirm that it can read it.
 * @return 0 on success.
 */
int testVector(const std::vector<std::string>& vec, const std::string_view& expected, bool testCsvReader)
{
  std::stringstream ss;
  simCore::CsvWriter writer(ss);
  writer.write(vec);
  int rv = SDK_ASSERT(ss.str() == expected);

  if (testCsvReader)
  {
    simCore::CsvReader reader(ss);
    std::vector<std::string> vec2;
    rv += SDK_ASSERT(reader.readLine(vec2) == 0);
    rv += SDK_ASSERT(vec == vec2);
  }
  return rv;
}

int testBasicWriting()
{
  int rv = 0;
  const auto& v1 = std::vector<std::string>{ "a", "b", "c" };
  rv += SDK_ASSERT(testVector(v1, "a,b,c\n", true) == 0);
  return rv;
}

int testEmpty()
{
  int rv = 0;
  const auto& v1 = std::vector<std::string>{ "a", "", "", "d" };
  rv += SDK_ASSERT(testVector(v1, "a,,,d\n", true) == 0);
  const auto& v2 = std::vector<std::string>{ };
  // No need to test the CSV Reader here, it will generate an error that is not totally wrong
  rv += SDK_ASSERT(testVector(v2, "\n", false) == 0);
  return rv;
}

int testWhitespaceTabs()
{
  int rv = 0;
  const auto& v1 = std::vector<std::string>{ " a a", "b\tb", "   ", "d\t "};
  // TODO: SIM-17007 causes this to fail in CSV Reader
  rv += SDK_ASSERT(testVector(v1, " a a,b\tb,   ,d\t \n", false) == 0);
  return rv;
}

int testMultiline()
{
  int rv = 0;
  // TODO SIM-17008: CSV Reader does not handle newlines correctly
  const auto& v1 = std::vector<std::string>{ "a", "b\nMulti-\nLine test in middle", "c" };
  rv += SDK_ASSERT(testVector(v1, "a,\"b\nMulti-\nLine test in middle\",c\n", false) == 0);
  const auto& v2 = std::vector<std::string>{ "a", "b", "\nMulti-\nLine test at the end" };
  rv += SDK_ASSERT(testVector(v2, "a,b,\"\nMulti-\nLine test at the end\"\n", false) == 0);
  return rv;
}

int testCommas()
{
  int rv = 0;
  const auto& v1 = std::vector<std::string>{ "a,,b", ",", "", ",c," };
  // TODO SIM-17010: CSV Reader testing is false because it doesn't handle tokens in quotes
  rv += SDK_ASSERT(testVector(v1, R"("a,,b",",",,",c,")" + std::string("\n"), false) == 0);
  return rv;
}

int testQuotes()
{
  int rv = 0;
  const auto& v1 = std::vector<std::string>{ "\"", "b\"b", "c\"\"\"cc\"\"", "", "'\",\""};
  // TODO SIM-17013: Internal quote tokens are not handled properly in CSV Reader
  rv += SDK_ASSERT(testVector(v1, R"("""","b""b","c""""""cc""""",,"'"",""")" + std::string("\n"), false) == 0);

  // Repeat test, but with double quotes enabled. Don't bother reading from CSV
  // because even Excel doesn't handle escaped quotes like this.
  std::stringstream ss;
  simCore::CsvWriter writer(ss);
  writer.setDoubleQuote(false);
  writer.write(v1);
  const std::string& expected2 = R"("\"","b\"b","c\"\"\"cc\"\"",,"'\",\"")" + std::string("\n");
  rv += SDK_ASSERT(ss.str() == expected2);

  // Change the escape token to something else
  writer.setEscapeCharacter('+');
  ss.str("");
  writer.write(v1);
  rv += SDK_ASSERT(ss.str() == R"("+"","b+"b","c+"+"+"cc+"+"",,"'+",+"")" + std::string("\n"));

  return rv;
}

int testDegreeSymbol()
{
  int rv = 0;
  // Simple test on degree symbols, which cause problems in XML, but should cause
  // no issues or special behavior in CSV.
  static const std::string DEG_UTF8 = "\xc2\xb0";
  static const std::string DEG_ASCII = "\xb0";
  const auto& v1 = std::vector<std::string>{ "a", DEG_UTF8, DEG_ASCII, "d" };
  rv += SDK_ASSERT(testVector(v1, "a," + DEG_UTF8 + "," + DEG_ASCII + ",d\n", true) == 0);
  return rv;
}

int testDelimiterAndQuote()
{
  int rv = 0;

  std::stringstream ss;
  simCore::CsvWriter writer(ss);
  const auto& v1 = std::vector<std::string>{ "a", "b b", "c" };

  // Random unused character for delimiter
  ss.str("");
  writer.setDelimiter('x');
  writer.write(v1);
  rv += SDK_ASSERT(ss.str() == "axb bxc\n");

  // Alpha character for delim -- tests from here below are a bit wild but stress the system
  ss.str("");
  writer.setDelimiter('a');
  writer.write(v1);
  rv += SDK_ASSERT(ss.str() == "\"a\"ab bac\n");

  // Same, but change quote token
  ss.str("");
  writer.setDelimiter('a');
  writer.setQuoteCharacter('x');
  writer.write(v1);
  rv += SDK_ASSERT(ss.str() == "xaxab bac\n");

  // Same, but change quote token to a used character
  ss.str("");
  writer.setDelimiter('a');
  writer.setQuoteCharacter(' ');
  writer.write(v1);
  rv += SDK_ASSERT(ss.str() == " a a b  b ac\n");

  // Reset quote token and use a different character for delimiter
  ss.str("");
  writer.setDelimiter(' ');
  writer.setQuoteCharacter('\"');
  writer.write(v1);
  rv += SDK_ASSERT(ss.str() == "a \"b b\" c\n");

  return rv;
}

int testEscapeChars()
{
  int rv = 0;

  // Do a test with escape characters mid-token to ensure they're properly encoded
  std::stringstream ss;
  simCore::CsvWriter writer(ss);
  const auto& v1 = std::vector<std::string>{ "a", "\\", "\\\\", "d\\d", "e\\\"e" };

  // Test 1: double quote on; should not need any escapes
  ss.str("");
  writer.setDoubleQuote(true);
  writer.write(v1);
  std::string expected = R"(a,\,\\,d\d,"e\""e")" + std::string("\n");
  rv += SDK_ASSERT(ss.str() == expected);

  // Test 2: double quote off; escapes should be escaped
  ss.str("");
  writer.setDoubleQuote(false);
  writer.write(v1);
  expected = R"(a,\\,\\\\,d\\d,"e\\\"e")" + std::string("\n");
  rv += SDK_ASSERT(ss.str() == expected);

  return rv;
}

}

int CsvWriterTest(int argc, char *argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testBasicWriting() == 0);
  rv += SDK_ASSERT(testEmpty() == 0);
  rv += SDK_ASSERT(testWhitespaceTabs() == 0);
  rv += SDK_ASSERT(testMultiline() == 0);
  rv += SDK_ASSERT(testCommas() == 0);
  rv += SDK_ASSERT(testQuotes() == 0);
  rv += SDK_ASSERT(testDegreeSymbol() == 0);
  rv += SDK_ASSERT(testDelimiterAndQuote() == 0);
  rv += SDK_ASSERT(testEscapeChars() == 0);

  return rv;
}
