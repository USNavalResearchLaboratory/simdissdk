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
#include <sstream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/String/CsvReader.h"

namespace {

int testCsvReadLine()
{
  int rv = 0;

  std::istringstream stream("one,two,three\nfour,five,six");

  simCore::CsvReader reader(stream);
  std::vector<std::string> tokens;

  // Test basic stream
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "one");
  rv += SDK_ASSERT(tokens[1] == "two");
  rv += SDK_ASSERT(tokens[2] == "three");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "four");
  rv += SDK_ASSERT(tokens[1] == "five");
  rv += SDK_ASSERT(tokens[2] == "six");
  rv += SDK_ASSERT(reader.readLine(tokens) == 1);

  // Test rows of differing lengths
  stream.clear();
  stream.str("one,two\nthree,four,five\nsix,seven");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 2);
  rv += SDK_ASSERT(tokens[0] == "one");
  rv += SDK_ASSERT(tokens[1] == "two");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "three");
  rv += SDK_ASSERT(tokens[1] == "four");
  rv += SDK_ASSERT(tokens[2] == "five");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 2);
  rv += SDK_ASSERT(tokens[0] == "six");
  rv += SDK_ASSERT(tokens[1] == "seven");
  rv += SDK_ASSERT(reader.readLine(tokens) == 1);

  // Test stream with empty lines
  stream.clear();
  stream.str("one,two\n\nthree,four,five\n\nsix,seven");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 2);
  rv += SDK_ASSERT(tokens[0] == "one");
  rv += SDK_ASSERT(tokens[1] == "two");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "three");
  rv += SDK_ASSERT(tokens[1] == "four");
  rv += SDK_ASSERT(tokens[2] == "five");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 2);
  rv += SDK_ASSERT(tokens[0] == "six");
  rv += SDK_ASSERT(tokens[1] == "seven");
  rv += SDK_ASSERT(reader.readLine(tokens) == 1);

  // Test basic stream with odd whitespace thrown in
  stream.clear();
  stream.str("one  , two,thr  ee\n four ,   five,six");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "one  ");
  rv += SDK_ASSERT(tokens[1] == " two");
  rv += SDK_ASSERT(tokens[2] == "thr  ee");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == " four ");
  rv += SDK_ASSERT(tokens[1] == "   five");
  rv += SDK_ASSERT(tokens[2] == "six");
  rv += SDK_ASSERT(reader.readLine(tokens) == 1);

  return rv;
}

int testCsvReadLineTrimmed()
{
  int rv = 0;

  // Same leading and trailing whitespace test cases from testCsvreadLine1(), but using readLineTrimmed
  std::istringstream stream("one  , two,thr  ee\n four ,   five,six");

  simCore::CsvReader reader(stream);
  std::vector<std::string> tokens;

  // Test basic stream
  rv += SDK_ASSERT(reader.readLineTrimmed(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "one");
  rv += SDK_ASSERT(tokens[1] == "two");
  rv += SDK_ASSERT(tokens[2] == "thr  ee");
  rv += SDK_ASSERT(reader.readLineTrimmed(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "four");
  rv += SDK_ASSERT(tokens[1] == "five");
  rv += SDK_ASSERT(tokens[2] == "six");
  rv += SDK_ASSERT(reader.readLineTrimmed(tokens) == 1);

  return rv;
}

int testCsvWithComments()
{
  int rv = 0;

  std::istringstream stream("#column 1, column 2, column 3\none,two,three\nfour,five,six");

  simCore::CsvReader reader(stream);
  std::vector<std::string> tokens;

  // Test comments
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "one");
  rv += SDK_ASSERT(tokens[1] == "two");
  rv += SDK_ASSERT(tokens[2] == "three");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "four");
  rv += SDK_ASSERT(tokens[1] == "five");
  rv += SDK_ASSERT(tokens[2] == "six");
  rv += SDK_ASSERT(reader.readLine(tokens) == 1);

  // Test changing the comment char
  reader.setCommentChar('$');
  stream.clear();
  stream.str("$column 1, column 2, column 3\none,two,three\nfour,five,six");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "one");
  rv += SDK_ASSERT(tokens[1] == "two");
  rv += SDK_ASSERT(tokens[2] == "three");
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "four");
  rv += SDK_ASSERT(tokens[1] == "five");
  rv += SDK_ASSERT(tokens[2] == "six");
  rv += SDK_ASSERT(reader.readLine(tokens) == 1);

  return rv;
}


int testCsvLineNumber()
{
  int rv = 0;

  std::istringstream stream("#col 1, col 2, col3\none,two\n\n\nthree,four,five\nsix,seven");

  simCore::CsvReader reader(stream);
  std::vector<std::string> tokens;

  rv += SDK_ASSERT(reader.lineNumber() == 0);
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  // Skips comment line
  rv += SDK_ASSERT(reader.lineNumber() == 2);
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  // Skips empty lines
  rv += SDK_ASSERT(reader.lineNumber() == 5);
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(reader.lineNumber() == 6);
  rv += SDK_ASSERT(reader.readLine(tokens) == 1);

  return rv;
}

int testReadEmptyLines()
{
  int rv = 0;

  std::istringstream stream("\n#col 1, col 2, col3\none,two\n\nthree,four,five\n\nsix,seven");

  simCore::CsvReader reader(stream);
  std::vector<std::string> tokens;

  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(reader.lineNumber() == 3);
  rv += SDK_ASSERT(tokens.size() == 2); // [one, two]

  // Read line skipping empty lines, will skip line 4
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(reader.lineNumber() == 5);
  rv += SDK_ASSERT(tokens.size() == 3); // [three, four, five]

  // Read line without skipping empty lines
  rv += SDK_ASSERT(reader.readLine(tokens, false) == 0);
  rv += SDK_ASSERT(reader.lineNumber() == 6);
  rv += SDK_ASSERT(tokens.empty()); // empty line

  rv += SDK_ASSERT(reader.readLine(tokens, false) == 0);
  rv += SDK_ASSERT(reader.lineNumber() == 7);
  rv += SDK_ASSERT(tokens.size() == 2); // [six, seven]
  rv += SDK_ASSERT(reader.readLine(tokens, false) == 1);

  return rv;
}

int testDelimiterInToken()
{
  // This is 4 tokens:
  //   a,,b
  //   ,
  //   empty
  //   ,c,
  std::istringstream is(R"("a,,b",",",,",c,")");

  simCore::CsvReader reader(is);

  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 4);
  rv += SDK_ASSERT(tokens[0] == "a,,b");
  rv += SDK_ASSERT(tokens[1] == ",");
  rv += SDK_ASSERT(tokens[2].empty());
  rv += SDK_ASSERT(tokens[3] == ",c,");
  return rv;
}

int testEmptyLineInQuotedToken()
{
  // This is 3 tokens:
  //   a
  //   \nb\n\nb\n
  //   c
  std::istringstream is("a,\"\nb\n\nb\n\",c");
  simCore::CsvReader reader(is);
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == "a");
  rv += SDK_ASSERT(tokens[1] == "\nb\n\nb\n");
  rv += SDK_ASSERT(tokens[2] == "c");
  return rv;
}

int testQuotedComment()
{
  // This is 2 tokens:
  //   a#
  //   b
  std::istringstream is(R"("a#",b#,c)");
  simCore::CsvReader reader(is);
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 2);
  rv += SDK_ASSERT(tokens[0] == "a#");
  rv += SDK_ASSERT(tokens[1] == "b");
  return rv;
}

int testSymmetricWhitespace()
{
  std::istringstream is(" a a,b\tb,   ,d\t ");
  simCore::CsvReader reader(is);
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 4);
  rv += SDK_ASSERT(tokens[0] == " a a");
  rv += SDK_ASSERT(tokens[1] == "b\tb");
  rv += SDK_ASSERT(tokens[2] == "   ");
  rv += SDK_ASSERT(tokens[3] == "d\t ");
  return rv;
}

int testLotsOfQuotes()
{
  std::istringstream is(R"("""","b""b","c""""""cc""""",,"'"",""")");
  simCore::CsvReader reader(is);
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 5);
  rv += SDK_ASSERT(tokens[0] == R"(")");
  rv += SDK_ASSERT(tokens[1] == R"(b"b)");
  rv += SDK_ASSERT(tokens[2] == R"(c"""cc"")");
  rv += SDK_ASSERT(tokens[3].empty());
  rv += SDK_ASSERT(tokens[4] == R"('",")");
  return rv;
}

int testNullQuotes1()
{
  // Case with must-be-user-generated file, where quotes are not doubled or embedded
  std::istringstream is(R"(file,with " the,quote",embedded)");
  simCore::CsvReader reader(is);
  reader.setQuoteChar('\0');
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 4);
  rv += SDK_ASSERT(tokens[0] == R"(file)");
  rv += SDK_ASSERT(tokens[1] == R"(with " the)");
  rv += SDK_ASSERT(tokens[2] == R"(quote")");
  rv += SDK_ASSERT(tokens[3] == R"(embedded)");
  return rv;
}

int testNullQuotes2()
{
  // Case with properly quoted file
  std::istringstream is(R"("token1","token""two",tok""en3,"token4")");
  simCore::CsvReader reader(is);
  reader.setQuoteChar('\0');
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 4);
  rv += SDK_ASSERT(tokens[0] == R"("token1")");
  rv += SDK_ASSERT(tokens[1] == R"("token""two")");
  rv += SDK_ASSERT(tokens[2] == R"(tok""en3)");
  rv += SDK_ASSERT(tokens[3] == R"("token4")");
  return rv;
}

int testQuotesInMiddle1()
{
  // Degenerate use case, from operator entry. This should match Excel behavior
  std::istringstream is(R"(file,with " the,quote",embedded)");
  simCore::CsvReader reader(is);
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 4);
  rv += SDK_ASSERT(tokens[0] == R"(file)");
  rv += SDK_ASSERT(tokens[1] == R"(with " the)");
  rv += SDK_ASSERT(tokens[2] == R"(quote")");
  rv += SDK_ASSERT(tokens[3] == R"(embedded)");
  return rv;
}

int testQuotesInMiddle2()
{
  // Degenerate use case, from operator entry. This should match Excel behavior
  std::istringstream is(R"("token1","token""two",tok""en3,"token4")");
  simCore::CsvReader reader(is);
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 4);
  rv += SDK_ASSERT(tokens[0] == R"(token1)");
  rv += SDK_ASSERT(tokens[1] == R"(token"two)");
  rv += SDK_ASSERT(tokens[2] == R"(tok""en3)"); // note double quotes
  rv += SDK_ASSERT(tokens[3] == R"(token4)");
  return rv;
}

int testQuotesInMiddle3()
{
  // Degenerate use case, from operator entry. This should match Excel behavior
  std::istringstream is(R"(a, "b", "dog, cat", food,"dog, cat", food)");
  simCore::CsvReader reader(is);
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 7);
  rv += SDK_ASSERT(tokens[0] == R"(a)");
  rv += SDK_ASSERT(tokens[1] == R"( "b")");
  rv += SDK_ASSERT(tokens[2] == R"( "dog)");
  rv += SDK_ASSERT(tokens[3] == R"( cat")");
  rv += SDK_ASSERT(tokens[4] == R"( food)");
  rv += SDK_ASSERT(tokens[5] == R"(dog, cat)");
  rv += SDK_ASSERT(tokens[6] == R"( food)");
  return rv;
}

int testQuotesInMiddle4()
{
  // Degenerate use case, from operator entry. This should match Excel behavior
  std::istringstream is(R"(a,"quote " ends early,c)");
  simCore::CsvReader reader(is);
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == R"(a)");
  rv += SDK_ASSERT(tokens[1] == R"(quote  ends early)");
  rv += SDK_ASSERT(tokens[2] == R"(c)");
  return rv;
}

int testQuotesInMiddle5()
{
  // Degenerate use case, from operator entry. This should match Excel behavior
  std::istringstream is(R"(a,"quoted "" token" "that ends",c)");
  simCore::CsvReader reader(is);
  std::vector<std::string> tokens;

  int rv = 0;
  rv += SDK_ASSERT(reader.readLine(tokens) == 0);
  rv += SDK_ASSERT(tokens.size() == 3);
  rv += SDK_ASSERT(tokens[0] == R"(a)");
  rv += SDK_ASSERT(tokens[1] == R"(quoted " token "that ends")");
  rv += SDK_ASSERT(tokens[2] == R"(c)");
  return rv;
}

int testReadTrimmedSkipEmpty()
{
  std::vector<std::string> tokens;
  int rv = 0;

  { // First test with skip-empty-lines off
    std::istringstream is("\n\nsimple,line\n \nthree\n");
    simCore::CsvReader reader(is);
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.empty());
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.empty());
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 2);
    rv += SDK_ASSERT(tokens[0] == "simple");
    rv += SDK_ASSERT(tokens[1] == "line");
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.empty());
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 1);
    rv += SDK_ASSERT(tokens[0] == "three");
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) != 0);
    rv += SDK_ASSERT(tokens.empty());
  }

  { // Next, test with skip-empty-lines on
    std::istringstream is("\n\nsimple,line\n \nthree\n");
    simCore::CsvReader reader(is);
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, true) == 0);
    rv += SDK_ASSERT(tokens.size() == 2);
    rv += SDK_ASSERT(tokens[0] == "simple");
    rv += SDK_ASSERT(tokens[1] == "line");
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, true) == 0);
    rv += SDK_ASSERT(tokens.size() == 1);
    rv += SDK_ASSERT(tokens[0] == "three");
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) != 0);
  }
  return rv;
}

int testCommentsInMiddle()
{
  std::vector<std::string> tokens;
  int rv = 0;

  { // First test with mid-line comments enabled
    std::istringstream is("#CommentLine,PostComment\nNo Comment Line,Second Token\nComment#Mid-Line,PostComment\n");
    simCore::CsvReader reader(is);
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.empty());
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 2);
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 1);
    rv += SDK_ASSERT(tokens[0] == "Comment");
  }

  { // Test that, even when enabled, comments are ignored mid-quote
    std::istringstream is("\"Quoted#CommentLine\",PostComment\n");
    simCore::CsvReader reader(is);
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 2);
    rv += SDK_ASSERT(tokens[0] == "Quoted#CommentLine"); // The quote characters themselves are removed by tokenization
    rv += SDK_ASSERT(tokens[1] == "PostComment");
  }

  { // Test with mid-line comments disabled
    std::istringstream is("#CommentLine,PostComment\nNo Comment Line,Second Token\nComment#Mid-Line,PostComment\n");
    simCore::CsvReader reader(is);
    reader.setAllowMidlineComments(false);
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.empty());
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 2);
    rv += SDK_ASSERT(reader.readLineTrimmed(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 2);
    rv += SDK_ASSERT(tokens[0] == "Comment#Mid-Line");
    rv += SDK_ASSERT(tokens[1] == "PostComment");
  }

  return rv;
}

int testMultiLineNumber()
{
  std::vector<std::string> tokens;
  int rv = 0;

  // test that line number is captured correctly when parsing a quote that spans multiple lines
  {
    std::istringstream is("\"open quote\n\",end quote\nnextline");
    simCore::CsvReader reader(is);
    rv += SDK_ASSERT(reader.readLine(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 2);
    rv += SDK_ASSERT(tokens[0] == "open quote\n");
    rv += SDK_ASSERT(tokens[1] == "end quote");
    // this is first line in the file, which was the starting line for the read
    rv += SDK_ASSERT(reader.lineNumber() == 1);
    rv += SDK_ASSERT(reader.readLine(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 1);
    rv += SDK_ASSERT(tokens[0] == "nextline");
    // this is 3rd line in the file, since previous read handled 2 lines within the quotes
    rv += SDK_ASSERT(reader.lineNumber() == 3);
  }

  {
    std::istringstream is("\"\n\nfirst line\n\"\nfourth line");
    simCore::CsvReader reader(is);
    rv += SDK_ASSERT(reader.readLine(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 1);
    rv += SDK_ASSERT(tokens[0] == "\n\nfirst line\n");
    // this is first line in the file, which was the starting line for the read
    rv += SDK_ASSERT(reader.lineNumber() == 1);
    rv += SDK_ASSERT(reader.readLine(tokens, false) == 0);
    rv += SDK_ASSERT(tokens.size() == 1);
    rv += SDK_ASSERT(tokens[0] == "fourth line");
    // this is the 5th lnie in the file, since the previous read handled 4 lines in the quotes
    rv += SDK_ASSERT(reader.lineNumber() == 5);
  }
  return rv;
}

}

int CsvReaderTest(int argc, char *argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testCsvReadLine() == 0);
  rv += SDK_ASSERT(testCsvReadLineTrimmed() == 0);
  rv += SDK_ASSERT(testCsvWithComments() == 0);
  rv += SDK_ASSERT(testCsvLineNumber() == 0);
  rv += SDK_ASSERT(testReadEmptyLines() == 0);
  rv += SDK_ASSERT(testDelimiterInToken() == 0);
  rv += SDK_ASSERT(testEmptyLineInQuotedToken() == 0);
  rv += SDK_ASSERT(testQuotedComment() == 0);
  rv += SDK_ASSERT(testSymmetricWhitespace() == 0);
  rv += SDK_ASSERT(testLotsOfQuotes() == 0);
  rv += SDK_ASSERT(testNullQuotes1() == 0);
  rv += SDK_ASSERT(testNullQuotes2() == 0);
  rv += SDK_ASSERT(testQuotesInMiddle1() == 0);
  rv += SDK_ASSERT(testQuotesInMiddle2() == 0);
  rv += SDK_ASSERT(testQuotesInMiddle3() == 0);
  rv += SDK_ASSERT(testQuotesInMiddle4() == 0);
  rv += SDK_ASSERT(testQuotesInMiddle5() == 0);
  rv += SDK_ASSERT(testReadTrimmedSkipEmpty() == 0);
  rv += SDK_ASSERT(testCommentsInMiddle() == 0);
  rv += SDK_ASSERT(testMultiLineNumber() == 0);

  return rv;
}
