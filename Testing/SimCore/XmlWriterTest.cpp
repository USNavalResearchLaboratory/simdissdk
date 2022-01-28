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
#include <limits>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simCore/String/XmlWriter.h"

namespace
{

int testOneCData(const std::string& tag, const std::string& input, const std::string& expected, bool encodeCarriageReturn, bool greedy)
{
  std::stringstream ss;
  simCore::XmlWriter writer(ss);
  writer.setEncodeCarriageReturns(encodeCarriageReturn);
  writer.setGreedyCData(greedy);
  writer.writeTag(tag, input);
  std::string actual = ss.str();
  return SDK_ASSERT(actual == expected);
}

int testCData()
{
  int rv = 0;

  // Non-greedy; encoding CR on
  rv += SDK_ASSERT(0 == testOneCData("Text", "One line of text", "<Text>One line of text</Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\nLine 2\nLine 3", "<Text>Line 1&#xA;Line 2&#xA;Line 3</Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\rLine 2\rLine 3", "<Text>Line 1&#xA;Line 2&#xA;Line 3</Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\r\nLine 2\r\nLine 3", "<Text>Line 1&#xA;Line 2&#xA;Line 3</Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "]]>", "<Text>]]<![CDATA[>]]></Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\n]]>\nLine 3", "<Text>Line 1&#xA;]]<![CDATA[>]]>&#xA;Line 3</Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "One line with \">\" in it", "<Text>One line with \">\" in it</Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "& at the start", "<Text><![CDATA[&]]> at the start</Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "In the middle & of the text", "<Text>In the middle <![CDATA[&]]> of the text</Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "At the end &", "<Text>At the end <![CDATA[&]]></Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "&& at the start", "<Text><![CDATA[&&]]> at the start</Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "In the middle && of the text", "<Text>In the middle <![CDATA[&&]]> of the text</Text>\n", true, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "At the end &&", "<Text>At the end <![CDATA[&&]]></Text>\n", true, false));

  // Non-greedy; encoding CR off
  rv += SDK_ASSERT(0 == testOneCData("Text", "One line of text", "<Text>One line of text</Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\nLine 2\nLine 3", "<Text>Line 1\rLine 2\rLine 3</Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\rLine 2\rLine 3", "<Text>Line 1\rLine 2\rLine 3</Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\r\nLine 2\r\nLine 3", "<Text>Line 1\rLine 2\rLine 3</Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "]]>", "<Text>]]<![CDATA[>]]></Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\n]]>\nLine 3", "<Text>Line 1\r]]<![CDATA[>]]>\rLine 3</Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "One line with \">\" in it", "<Text>One line with \">\" in it</Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "& at the start", "<Text><![CDATA[&]]> at the start</Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "In the middle & of the text", "<Text>In the middle <![CDATA[&]]> of the text</Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "At the end &", "<Text>At the end <![CDATA[&]]></Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "&& at the start", "<Text><![CDATA[&&]]> at the start</Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "In the middle && of the text", "<Text>In the middle <![CDATA[&&]]> of the text</Text>\n", false, false));
  rv += SDK_ASSERT(0 == testOneCData("Text", "At the end &&", "<Text>At the end <![CDATA[&&]]></Text>\n", false, false));

  // Greedy; encoding CR on
  rv += SDK_ASSERT(0 == testOneCData("Text", "One line of text", "<Text>One line of text</Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\nLine 2\nLine 3", "<Text><![CDATA[Line 1\nLine 2\nLine 3]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\rLine 2\rLine 3", "<Text><![CDATA[Line 1]]>&#xD;<![CDATA[Line 2]]>&#xD;<![CDATA[Line 3]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\r\nLine 2\r\nLine 3", "<Text><![CDATA[Line 1]]>&#xD;<![CDATA[\nLine 2]]>&#xD;<![CDATA[\nLine 3]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "]]>", "<Text><![CDATA[]]]]><![CDATA[>]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\n]]>\nLine 3", "<Text><![CDATA[Line 1\n]]]]><![CDATA[>\nLine 3]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "One line with \">\" in it", "<Text><![CDATA[One line with \">\" in it]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "& at the start", "<Text><![CDATA[& at the start]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "In the middle & of the text", "<Text><![CDATA[In the middle & of the text]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "At the end &", "<Text><![CDATA[At the end &]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "&& at the start", "<Text><![CDATA[&& at the start]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "In the middle && of the text", "<Text><![CDATA[In the middle && of the text]]></Text>\n", true, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "At the end &&", "<Text><![CDATA[At the end &&]]></Text>\n", true, true));

  // Greedy; encoding CR off
  rv += SDK_ASSERT(0 == testOneCData("Text", "One line of text", "<Text>One line of text</Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\nLine 2\nLine 3", "<Text><![CDATA[Line 1\nLine 2\nLine 3]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\rLine 2\rLine 3", "<Text><![CDATA[Line 1\rLine 2\rLine 3]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\r\nLine 2\r\nLine 3", "<Text><![CDATA[Line 1\r\nLine 2\r\nLine 3]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "]]>", "<Text><![CDATA[]]]]><![CDATA[>]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "Line 1\n]]>\nLine 3", "<Text><![CDATA[Line 1\n]]]]><![CDATA[>\nLine 3]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "One line with \">\" in it", "<Text><![CDATA[One line with \">\" in it]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "& at the start", "<Text><![CDATA[& at the start]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "In the middle & of the text", "<Text><![CDATA[In the middle & of the text]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "At the end &", "<Text><![CDATA[At the end &]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "&& at the start", "<Text><![CDATA[&& at the start]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "In the middle && of the text", "<Text><![CDATA[In the middle && of the text]]></Text>\n", false, true));
  rv += SDK_ASSERT(0 == testOneCData("Text", "At the end &&", "<Text><![CDATA[At the end &&]]></Text>\n", false, true));

  return rv;
}

int testWriteTag()
{
  int rv = 0;
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.writeTag("a", " b ");
    rv += SDK_ASSERT(ss.str() == "<a> b </a>\n");
  }
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.writeTag("a", 1);
    rv += SDK_ASSERT(ss.str() == "<a>1</a>\n");
  }
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.writeTag("a", -35);
    rv += SDK_ASSERT(ss.str() == "<a>-35</a>\n");
  }
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.writeTag("a", 0.5);
    rv += SDK_ASSERT(ss.str() == "<a>0.5</a>\n");
  }

  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.writeTag("a", "&");
    rv += SDK_ASSERT(ss.str() == "<a><![CDATA[&]]></a>\n");
  }

  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    std::string ampersand("&");
    doc.writeTag("a", ampersand);
    rv += SDK_ASSERT(ss.str() == "<a><![CDATA[&]]></a>\n");
  }

  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.writeRawTag("a", "<![CDATA[&]]>");
    rv += SDK_ASSERT(ss.str() == "<a><![CDATA[&]]></a>\n");
  }

  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.writeTag("a", "<![CDATA[&]]>");
    rv += SDK_ASSERT(ss.str() != "<a><![CDATA[&]]></a>\n");
  }

  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.writeXmlDeclaration();
    doc.writeTag("a", "b");
    rv += SDK_ASSERT(ss.str() == "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<a>b</a>\n");
  }
  return rv;
}

int testXmlWriter()
{
  int rv = 0;
  // Test comments and attributes
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setUseComments(true);
    doc.startBlock("a", "b", "c=\"d\" e=\"f\"");
    doc.endBlock();
    rv += SDK_ASSERT(ss.str() == "<!-- b -->\n<a c=\"d\" e=\"f\">\n</a>\n");
  }
  // Repeat test with empty tag
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setUseComments(true);
    doc.writeTag("a", "", "b", "c=\"d\" e=\"f\"");
    rv += SDK_ASSERT(ss.str() == "<!-- b -->\n<a c=\"d\" e=\"f\" />\n");
  }

  // Test indent
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.startBlock("tag");
    doc.startBlock("subtag");
    doc.writeTag("child", "");
    doc.endBlock();
    doc.endBlock();
    rv += SDK_ASSERT(ss.str() == "<tag>\n <subtag>\n  <child />\n </subtag>\n</tag>\n");
  }

  // Test simple CData with attrib
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setUseComments(true);
    doc.writeTag("a", "b", "comment", "c=\"d\" e=\"f\"");
    rv += SDK_ASSERT(ss.str() == "<!-- comment -->\n<a c=\"d\" e=\"f\">b</a>\n");
  }
  // Test simple CData with attrib with CData-able item
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setUseComments(true);
    doc.writeTag("a", "&", "comment", "c=\"d\" e=\"f\"");
    rv += SDK_ASSERT(ss.str() == "<!-- comment -->\n<a c=\"d\" e=\"f\"><![CDATA[&]]></a>\n");
  }

  // Test namespaces
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setNamespace("xs");
    doc.startBlock("x", "", "y=\"z\"");
    doc.writeTag("emp", "", "", "q=\"r\"");
    doc.writeTag("n", "o", "", "g=\"h\"");
    doc.writeTag("a", "b", "", "c=\"d\"");
    doc.endBlock();
    rv += SDK_ASSERT(ss.str() == "<xs:x y=\"z\">\n <xs:emp q=\"r\" />\n <xs:n g=\"h\">o</xs:n>\n <xs:a c=\"d\">b</xs:a>\n</xs:x>\n");
  }
  // Repeat test, but with a colon at end of namespace
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setNamespace("xs:");
    doc.writeTag("a", "&", "comment", "c=\"d\" e=\"f\"");
    rv += SDK_ASSERT(ss.str() == "<xs:a c=\"d\" e=\"f\"><![CDATA[&]]></xs:a>\n");
  }


  // Repeat test, but with a colon at end of namespace
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setNamespace("xs:");
    doc.writeTag("a", "&", "comment", "c=\"d\" e=\"f\"");
    rv += SDK_ASSERT(ss.str() == "<xs:a c=\"d\" e=\"f\"><![CDATA[&]]></xs:a>\n");
  }

  // Adding attribute to a tag - simple
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.prepareAttribute("e", "f");
    doc.writeTag("a", "b", "", "c=\"d\"");
    rv += SDK_ASSERT(ss.str() == "<a c=\"d\" e=\"f\">b</a>\n");
  }
  // Adding attribute to a tag, then adding a tag
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.prepareAttribute("e", "f");
    doc.writeTag("a", "b", "", "c=\"d\"");
    doc.writeTag("g", "h");
    rv += SDK_ASSERT(ss.str() == "<a c=\"d\" e=\"f\">b</a>\n<g>h</g>\n");
  }
  // Adding attribute to a tag, then adding an empty tag
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.prepareAttribute("e", "f");
    doc.writeTag("a", "b", "", "c=\"d\"");
    doc.writeTag("g", "");
    rv += SDK_ASSERT(ss.str() == "<a c=\"d\" e=\"f\">b</a>\n<g />\n");
  }
  // Adding attribute to a tag, then adding an CData
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.prepareAttribute("e", "f");
    doc.writeTag("a", "b", "", "c=\"d\"");
    doc.writeTag("g", "h");
    rv += SDK_ASSERT(ss.str() == "<a c=\"d\" e=\"f\">b</a>\n<g>h</g>\n");
  }
  // Adding attribute to a tag, then adding a block
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.prepareAttribute("e", "f");
    doc.writeTag("a", "b", "", "c=\"d\"");
    doc.startBlock("g");
    doc.endBlock();
    rv += SDK_ASSERT(ss.str() == "<a c=\"d\" e=\"f\">b</a>\n<g>\n</g>\n");
  }

  // Adding multiple attributes to multiple items
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.prepareAttribute("e", "f");
    doc.prepareAttribute("h", "i");
    doc.writeTag("a", "b");
    doc.prepareAttribute("j", "k");
    doc.startBlock("g");
    doc.writeTag("p", "");
    doc.endBlock();
    rv += SDK_ASSERT(ss.str() == "<a e=\"f\" h=\"i\">b</a>\n<g j=\"k\">\n <p />\n</g>\n");
  }

  return rv;
}

int testUtf8ErrorsInXmlWriter()
{
  static const std::string DEG_UTF8 = "\xc2\xb0";
  static const std::string DEG_STRING_UTF8 = "deg" + DEG_UTF8 + "ree";
  static const std::string DEG_ASCII = "\xb0";
  static const std::string DEG_STRING_ASCII = "deg" + DEG_ASCII + "ree";
  static const std::string HDR = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";

  int rv = 0;

  // Test that correct UTF-8 encoding of degree symbol is read back correctly, when fixing errors
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setAsciiInput(true);
    doc.writeXmlDeclaration();
    doc.writeTag("a", DEG_STRING_UTF8);
    rv += SDK_ASSERT(ss.str() == (HDR + "<a>" + DEG_STRING_UTF8 + "</a>\n"));
  }

  // Test that correct UTF-8 encoding of degree symbol is read back correctly, when NOT fixing errors
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setAsciiInput(false);
    doc.writeXmlDeclaration();
    doc.writeTag("a", DEG_STRING_UTF8);
    rv += SDK_ASSERT(ss.str() == (HDR + "<a>" + DEG_STRING_UTF8 + "</a>\n"));
  }

  // Test that incorrect ASCII encoding of degree symbol is read back correctly, when fixing errors
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setAsciiInput(true);
    doc.writeXmlDeclaration();
    doc.writeTag("a", DEG_STRING_ASCII);
    rv += SDK_ASSERT(ss.str() == (HDR + "<a>" + DEG_STRING_UTF8 + "</a>\n"));
  }

  // Test that incorrect ASCII encoding of degree symbol is read back incorrectly, when NOT fixing errors
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setAsciiInput(false);
    doc.writeXmlDeclaration();
    doc.writeTag("a", DEG_STRING_ASCII);
    rv += SDK_ASSERT(ss.str() == (HDR + "<a>" + DEG_STRING_ASCII + "</a>\n"));
  }

  // Test that a valid 3-byte character string in UTF-8 is left alone (random valid character)
  {
    static const std::string VALID_THREE_BYTE_STRING = "\xe2\x94\xb0";
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setAsciiInput(true);
    doc.writeXmlDeclaration();
    doc.writeTag("a", VALID_THREE_BYTE_STRING);
    rv += SDK_ASSERT(ss.str() == (HDR + "<a>" + VALID_THREE_BYTE_STRING + "</a>\n"));
  }

  // Test that a double-degree is corrected
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setAsciiInput(true);
    doc.writeXmlDeclaration();
    doc.writeTag("a", DEG_ASCII + DEG_ASCII);
    rv += SDK_ASSERT(ss.str() == (HDR + "<a>" + DEG_UTF8 + DEG_UTF8 + "</a>\n"));
  }

  // Test attributes too
  {
    std::stringstream ss;
    simCore::XmlWriter doc(ss);
    doc.setAsciiInput(true);
    doc.writeXmlDeclaration();
    doc.prepareAttribute("b", DEG_STRING_ASCII);
    doc.writeTag("a", "foo");
    rv += SDK_ASSERT(ss.str() == (HDR + "<a b=\"" + DEG_STRING_UTF8 + "\">foo</a>\n"));
  }

  return rv;
}

}

int XmlWriterTest(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  int rv = 0;
  rv += SDK_ASSERT(0 == testCData());
  rv += SDK_ASSERT(0 == testWriteTag());
  rv += SDK_ASSERT(0 == testXmlWriter());
  rv += SDK_ASSERT(0 == testUtf8ErrorsInXmlWriter());
  std::cout << "simCore simCore::XmlWriterTest " << ((rv == 0) ? "passed" : "failed") << std::endl;

  return rv;
}
