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
#include <vector>
#include <cstdlib>
#include "simCore/Common/SDKAssert.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"

namespace
{
  int checkTokens(const std::vector<std::string> &v1, const std::vector<std::string> &v2)
  {
    if (v1.size() != v2.size())
      return 1;

    int rv = 0;
    std::vector<std::string>::const_iterator j = v2.begin();
    for (std::vector<std::string>::const_iterator i = v1.begin(); i != v1.end(); ++i, ++j)
    {
      if (*i != *j)
        ++rv;
    }
    return rv;
  }

  int testGetTerminateForStringPos()
  {
    std::string test1 = "'This is' \"\"\"a\"\"\" test";
    std::string rv = simCore::getTerminateForStringPos(test1, 0);
    int err = 0;
    err += SDK_ASSERT(rv == "'");
    rv = simCore::getTerminateForStringPos(test1, 1);
    if (rv != "") return 2;
    rv = simCore::getTerminateForStringPos(test1, 6);
    if (test1[6] != 'i') return 3;
    if (rv != "") return 4;
    rv = simCore::getTerminateForStringPos(test1, 10);
    if (test1[10] != '"') return 5;
    if (test1[9] != ' ') return 6;
    if (rv != "\"\"\"") return 7;
    return err;
  }

  int testGetFirstCharPosAfterString()
  {
    int rv = 0;
    //size_t getPosOf(const std::string& str, size_t start, const std::string& termString)
    //                   0123456789 0 1 23 4 5 678901
    std::string test1 = "'This is' \"\"\"a\"\"\" test";
    rv += SDK_ASSERT(test1[5] == ' ');
    rv += SDK_ASSERT(simCore::getFirstCharPosAfterString(test1, 0, "") == 5);
    rv += SDK_ASSERT(test1[8] == '\'');
    rv += SDK_ASSERT(simCore::getFirstCharPosAfterString(test1, 1, "'") == 9);
    rv += SDK_ASSERT(test1[9] == ' ');
    rv += SDK_ASSERT(test1[17] == ' ');
    rv += SDK_ASSERT(simCore::getFirstCharPosAfterString(test1, 11, "\"\"\"") == 17);
    return rv;
  }

  int testTokenizeWithQuotes()
  {
    int rv = 0;
    std::vector<std::string> tokens;
    std::vector<std::string> expected;

    // expected cases
    simCore::tokenizeWithQuotes(tokens, "token1 token2");
    expected.push_back("token1");
    expected.push_back("token2");
    rv += SDK_ASSERT(checkTokens(tokens, expected) == 0);

    simCore::tokenizeWithQuotes(tokens, "name=value token2");
    expected.clear();
    expected.push_back("name=value");
    expected.push_back("token2");
    rv += SDK_ASSERT(checkTokens(tokens, expected) == 0);

    simCore::tokenizeWithQuotes(tokens, "name=value \"quoted token\"");
    expected.clear();
    expected.push_back("name=value");
    expected.push_back("\"quoted token\"");
    rv += SDK_ASSERT(checkTokens(tokens, expected) == 0);

    // stress tests (spaces - before, after, and middle)
    simCore::tokenizeWithQuotes(tokens, "  token1   token2  ");
    expected.clear();
    expected.push_back("token1");
    expected.push_back("token2");
    rv += SDK_ASSERT(checkTokens(tokens, expected) == 0);

    simCore::tokenizeWithQuotes(tokens, " \" token1 \"  \"token2 \" ");
    expected.clear();
    expected.push_back("\" token1 \"");
    expected.push_back("\"token2 \"");
    rv += SDK_ASSERT(checkTokens(tokens, expected) == 0);

    simCore::tokenizeWithQuotes(tokens, "  \"token1 token2\" ");
    expected.clear();
    expected.push_back("\"token1 token2\"");
    rv += SDK_ASSERT(checkTokens(tokens, expected) == 0);

    simCore::tokenizeWithQuotes(tokens, "  token1=\"token2\"  token4 ");
    expected.clear();
    expected.push_back("token1=\"token2\"");
    expected.push_back("token4");
    rv += SDK_ASSERT(checkTokens(tokens, expected) == 0);

    std::string prefRule = "ruleName=Draw ruleValue=\"yes\"  nameExpression=\".*\"  entityType=PBGLD  categoryFilters=\"piTest2(1)~Unlisted Value(1)~No Value(1)~value1(1)\"";
    simCore::tokenizeWithQuotes(tokens, prefRule);
    expected.clear();
    expected.push_back("ruleName=Draw");
    expected.push_back("ruleValue=\"yes\"");
    expected.push_back("nameExpression=\".*\"");
    expected.push_back("entityType=PBGLD");
    expected.push_back("categoryFilters=\"piTest2(1)~Unlisted Value(1)~No Value(1)~value1(1)\"");
    rv += SDK_ASSERT(checkTokens(tokens, expected) == 0);

    // mismatched quote
    simCore::tokenizeWithQuotes(tokens, "  \"token1 token2 ");
    expected.clear();
    expected.push_back("\"token1 token2 ");
    rv += SDK_ASSERT(checkTokens(tokens, expected) == 0);
    return rv;
  }

  int testQuoteTokenizer()
  {
    int rv = 0;

    std::vector<std::string> tokens;
    std::string str = "This is a test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 4);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[3] == "test");

    str = "This \"is a\" test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 3);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "\"is a\"");
    rv += SDK_ASSERT(tokens[2] == "test");

    str = "This 'is a' test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 3);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "'is a'");
    rv += SDK_ASSERT(tokens[2] == "test");

    str = "This \"\"\"is a\"\"\" test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 3);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "\"\"\"is a\"\"\"");
    rv += SDK_ASSERT(tokens[2] == "test");

    str = "This \"\"\"is\" a\"\"\" test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 3);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "\"\"\"is\" a\"\"\"");
    rv += SDK_ASSERT(tokens[2] == "test");

    simCore::quoteTokenizer(tokens, " ");
    rv += SDK_ASSERT(tokens.empty());

    // From emptyQuotes.asi
    std::string testString = "GenericData 300 \"Simdis_ScreenText\" \"\" 30.000000 10";
    simCore::quoteTokenizer(tokens, testString);
    rv += SDK_ASSERT(tokens.size() == 6);
    rv += SDK_ASSERT(tokens[2] == "\"Simdis_ScreenText\"");
    rv += SDK_ASSERT(tokens[3] == "\"\"");

    // Test escaped quotes used in ASI files
    str = "This \"is \\\" a\" test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 3);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "\"is \\\" a\"");
    rv += SDK_ASSERT(tokens[2] == "test");

    str = "This \"is \\\" \\\" a\" test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 3);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "\"is \\\" \\\" a\"");
    rv += SDK_ASSERT(tokens[2] == "test");

    str = "\\\"This is a test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 4);
    rv += SDK_ASSERT(tokens[0] == "\\\"This");
    rv += SDK_ASSERT(tokens[1] == "is");
    rv += SDK_ASSERT(tokens[2] == "a");
    rv += SDK_ASSERT(tokens[3] == "test");

    str = "This is \\\" a test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 5);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "is");
    rv += SDK_ASSERT(tokens[2] == "\\\"");
    rv += SDK_ASSERT(tokens[3] == "a");
    rv += SDK_ASSERT(tokens[4] == "test");

    str = "This is a test\\\"";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 4);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "is");
    rv += SDK_ASSERT(tokens[2] == "a");
    rv += SDK_ASSERT(tokens[3] == "test\\\"");

    // Test escaping the escape
    str = "This \"is \\\\\" a\" test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 4);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "\"is \\\\\"");
    rv += SDK_ASSERT(tokens[2] == "a\"");
    rv += SDK_ASSERT(tokens[3] == "test");

    str = "This \"is \\\\\\\" a\" test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 3);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "\"is \\\\\\\" a\"");
    rv += SDK_ASSERT(tokens[2] == "test");

    str = "This \"is \\\\\\\\\" a\" test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 4);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "\"is \\\\\\\\\"");
    rv += SDK_ASSERT(tokens[2] == "a\"");
    rv += SDK_ASSERT(tokens[3] == "test");

    str = "\\\\\"This is a test";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 4);
    rv += SDK_ASSERT(tokens[0] == "\\\\\"This");
    rv += SDK_ASSERT(tokens[1] == "is");
    rv += SDK_ASSERT(tokens[2] == "a");
    rv += SDK_ASSERT(tokens[3] == "test");

    str = "This is a test\\\\\"";
    simCore::quoteTokenizer(tokens, str);
    rv += SDK_ASSERT(tokens.size() == 4);
    rv += SDK_ASSERT(tokens[0] == "This");
    rv += SDK_ASSERT(tokens[1] == "is");
    rv += SDK_ASSERT(tokens[2] == "a");
    rv += SDK_ASSERT(tokens[3] == "test\\\\\"");

    return rv;
  }

  int testCommentTokens()
  {
    int rv = 0;
    std::string test1;
    std::vector<std::string> vec;

    test1 = "Insert a \"#comment\" after # here";
    simCore::quoteTokenizer(vec, test1);
    simCore::removeCommentTokens(vec);
    rv += SDK_ASSERT(vec.size() == 4);

    test1 = "Insert 'a #comment' after // here # ok";
    simCore::quoteTokenizer(vec, test1);
    simCore::removeCommentTokens(vec);
    rv += SDK_ASSERT(vec.size() == 3);
    rv += SDK_ASSERT(vec[2] == "after");

    test1 = "Insert 'a #comment' after// here # ok";
    simCore::quoteTokenizer(vec, test1);
    simCore::removeCommentTokens(vec);
    rv += SDK_ASSERT(vec.size() == 3);
    rv += SDK_ASSERT(vec[2] == "after");

    test1 = "Insert 'a #comment' after# here # ok";
    simCore::quoteTokenizer(vec, test1);
    simCore::removeCommentTokens(vec);
    rv += SDK_ASSERT(vec.size() == 3);
    rv += SDK_ASSERT(vec[2] == "after");

    test1 = "Insert 'a #comment' aft#er here # ok";
    simCore::quoteTokenizer(vec, test1);
    simCore::removeCommentTokens(vec);
    rv += SDK_ASSERT(vec.size() == 3);
    rv += SDK_ASSERT(vec[2] == "aft");

    test1 = "Insert 'a #comment' 'aft#er' here # ok";
    simCore::quoteTokenizer(vec, test1);
    simCore::removeCommentTokens(vec);
    rv += SDK_ASSERT(vec.size() == 4);
    rv += SDK_ASSERT(vec[3] == "here");

    // emptyQuotes.asi
    test1 = "GenericData 300 \"Simdis_ScreenText\" \"\" 30.000000 10";
    simCore::quoteTokenizer(vec, test1);
    simCore::removeCommentTokens(vec);
    rv += SDK_ASSERT(vec.size() == 6);
    rv += SDK_ASSERT(vec[2] == "\"Simdis_ScreenText\"");
    rv += SDK_ASSERT(vec[3] == "\"\"");
    return rv;
  }

  int testRemoveQuotes()
  {
    int rv = 0;
    rv += SDK_ASSERT(simCore::removeQuotes("'Test'") == "Test");
    rv += SDK_ASSERT(simCore::removeQuotes("'Test") == "'Test");
    rv += SDK_ASSERT(simCore::removeQuotes("\"Test") == "\"Test");
    rv += SDK_ASSERT(simCore::removeQuotes("\"Test\"") == "Test");
    rv += SDK_ASSERT(simCore::removeQuotes("\"\"\"Test\"\"\"") == "Test");
    rv += SDK_ASSERT(simCore::removeQuotes("''") == "");
    rv += SDK_ASSERT(simCore::removeQuotes("' '") == " ");
    rv += SDK_ASSERT(simCore::removeQuotes("\"\"") == "");
    rv += SDK_ASSERT(simCore::removeQuotes("\" \"") == " ");
    return rv;
  }

  int escapeTest()
  {
    int rv = 0;
    const std::string noQuotes = "Token1 Token2 Token3";
    std::vector<std::string> tokens;
    simCore::escapeTokenize(tokens, noQuotes);
    rv += SDK_ASSERT(tokens.size() == 3);
    rv += SDK_ASSERT(tokens[0] == "Token1");
    rv += SDK_ASSERT(tokens[1] == "Token2");
    rv += SDK_ASSERT(tokens[2] == "Token3");

    const std::string allQuotes = "\"Token1\" \"Token with spaces\" \"Token3\"";
    simCore::escapeTokenize(tokens, allQuotes);
    rv += SDK_ASSERT(tokens.size() == 3);
    rv += SDK_ASSERT(tokens[0] == "\"Token1\"");
    rv += SDK_ASSERT(tokens[1] == "\"Token with spaces\"");
    rv += SDK_ASSERT(tokens[2] == "\"Token3\"");

    const std::string someQuotes = "Token1 \"Middle \\\"Quoted\\\" token \"  Token3";
    simCore::escapeTokenize(tokens, someQuotes);
    rv += SDK_ASSERT(tokens.size() == 3);
    rv += SDK_ASSERT(tokens[0] == "Token1");
    rv += SDK_ASSERT(tokens[1] == "\"Middle \"Quoted\" token \"");
    rv += SDK_ASSERT(tokens[2] == "Token3");

    return rv;
  }

  ///------------------------------------------------------------

  int testHasEnv(const std::string& str, bool expectedAnswer=true)
  {
    bool check = simCore::hasEnv(str);
    int rv = SDK_ASSERT(check == expectedAnswer);
    std::cout << "Test string <" << str << ">: result = " << ((check == expectedAnswer) ? "pass" : "fail") << std::endl;
    return rv;
  }

  int testHasEnv()
  {
    int rv = 0;
    std::cout << "===========" << std::endl;
    std::cout << "Test hasEnv" << std::endl;
    std::cout << "===========" << std::endl;
    rv += testHasEnv("", false);
    rv += testHasEnv("   ", false);
    rv += testHasEnv("$(PATH", false);
    rv += testHasEnv("$()", true);
    rv += testHasEnv("$(SIMDIS_DIR)", true);
    rv += testHasEnv(" $(TMP) ", true);
    rv += testHasEnv(" $(ENV_NO_EXIST) ", true);
    rv += testHasEnv("foo)$(SIMDIS_DIR)bar", true);
    rv += testHasEnv("foo)$(SIMDIS_DIR", false);
    rv += testHasEnv("$(SIMDIS_DIR)/bin/x86-nt", true);
    rv += testHasEnv("$(SIMDIS_TERRAIN)/imagery/sub/directory/", true);
    rv += testHasEnv("$(SIMDIS_HOME)/subdir/path", true);
    rv += testHasEnv("$(SIMDIS_USER_DIR)/test", true);
    rv += testHasEnv("$(PEOPLE_DIR)/CMakeLists.txt", true);
    rv += testHasEnv("${SIMDIS_HOME}/subdir/path", false);
    rv += testHasEnv("$SIMDIS_HOME/subdir/path", false);
    rv += testHasEnv("$( SIMDIS_HOME )/subdir/path", true);
    rv += testHasEnv("$(SIMDIS_HOME )/subdir/path", true);
    rv += testHasEnv("$( SIMDIS_HOME)/subdir/path", true);
    std::cout << "================================" << std::endl << std::endl;
    return rv;
  }

  ///------------------------------------------------------------

  int testExpandEnv(const std::string& withEnv, const std::string& expect)
  {
    std::string fromFunc = simCore::expandEnv(withEnv);
    int rv = SDK_ASSERT(fromFunc == expect);
    std::cout << "Test string <" << withEnv << ">: result = [" << fromFunc << "], expected [" << expect << "]" << std::endl;
    return rv;
  }

  void setEnvVar(const std::string& key, const std::string& value)
  {
  #ifdef WIN32
    std::string env = key + "=" + value;
    _putenv(env.c_str());
  #else
    setenv(key.c_str(), value.c_str(), 1);
  #endif
  }

  // SIMDIS_DIR to c:/people/..., or similar; string-safe
  std::string env(const std::string& envVar)
  {
    if (getenv(envVar.c_str()) != NULL)
      return getenv(envVar.c_str());
    return "$(" + envVar + ")";
  }

  int testExpandEnv()
  {
    int rv = 0;
    std::cout << "==============" << std::endl;
    std::cout << "Test expandEnv" << std::endl;
    std::cout << "==============" << std::endl;

    setEnvVar("FILE_TEST_SLASH_DIR", "c:\\test/slash\\dir");
    setEnvVar("TEST_HOST_ARCH", "win32");
    setEnvVar("TEST_HOST_OS", "nt");
    setEnvVar("SIMDIS_DIR", "/usr/local/simdis");
    setEnvVar("SIMDIS_HOME", "/home/simdis");
    setEnvVar("SYSTEMROOT", "/");
    setEnvVar("TMP", "/tmp");

    rv += testExpandEnv("", "");
    rv += testExpandEnv("   ", "   ");
    rv += testExpandEnv("$(PATH", "$(PATH");
    rv += testExpandEnv("$()", "$()");
    rv += testExpandEnv("$(SIMDIS_DIR)", env("SIMDIS_DIR"));
    rv += testExpandEnv(" $(TMP) ", std::string(" ") + env("TMP") + " ");
    rv += testExpandEnv(" $(ENV_NO_EXIST) ", "  ");
    rv += testExpandEnv("foo)$(SIMDIS_DIR)bar", "foo)" + env("SIMDIS_DIR") + "bar");
    rv += testExpandEnv("foo)$(SIMDIS_DIR", "foo)$(SIMDIS_DIR");
    rv += testExpandEnv("$(SIMDIS_HOME)", env("SIMDIS_HOME"));
    // Ensure no other formats work besides $()
    rv += testExpandEnv("$SIMDIS_HOME/subdir/path", "$SIMDIS_HOME/subdir/path");
    rv += testExpandEnv("${SIMDIS_HOME}/subdir/path", "${SIMDIS_HOME}/subdir/path");
    rv += testExpandEnv("$( SIMDIS_HOME )/subdir/path", "$( SIMDIS_HOME )/subdir/path");
    rv += testExpandEnv("$(SIMDIS_HOME )/subdir/path", "$(SIMDIS_HOME )/subdir/path");
    rv += testExpandEnv("$( SIMDIS_HOME)/subdir/path", "$( SIMDIS_HOME)/subdir/path");
    // Test system envs (back slashes)
    if (env("SYSTEMROOT") != "")
      rv += testExpandEnv("$(SYSTEMROOT)/system32", env("SYSTEMROOT")+ "/system32");
    // Test multiple envs in a name
    rv += testExpandEnv("$(SIMDIS_HOME)/$(TEST_HOST_ARCH)/$(TEST_HOST_OS)/test", env("SIMDIS_HOME") + "/" + env("TEST_HOST_ARCH") + "/" + env("TEST_HOST_OS") + "/test");
    // Test multiple envs in a name, including one that does not exist (review 546 case)
    rv += testExpandEnv("$(SIMDIS_HOME)/$(TEST_HOST_PLAT)/$(TEST_HOST_OS)/test", env("SIMDIS_HOME") + "//" + env("TEST_HOST_OS") + "/test");
    // Test env with both front and back slashes
    rv += testExpandEnv("$(FILE_TEST_SLASH_DIR)", env("FILE_TEST_SLASH_DIR"));
    return rv;
  }

  }

int TokenizerTest(int argc, char *argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testGetTerminateForStringPos() == 0);
  rv += SDK_ASSERT(testGetFirstCharPosAfterString() == 0);
  rv += SDK_ASSERT(testTokenizeWithQuotes() == 0);
  rv += SDK_ASSERT(testQuoteTokenizer() == 0);
  rv += SDK_ASSERT(testCommentTokens() == 0);
  rv += SDK_ASSERT(testRemoveQuotes() == 0);
  rv += SDK_ASSERT(escapeTest() == 0);

  rv += SDK_ASSERT(testHasEnv() == 0);
  rv += SDK_ASSERT(testExpandEnv() == 0);

  std::cout << "simCore TokenizerTest " << ((rv == 0) ? "passed" : "failed") << std::endl;

  return rv;
}
