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
#ifndef SIMCORE_STRING_XMLWRITER_H
#define SIMCORE_STRING_XMLWRITER_H

#include <deque>
#include <iostream>
#include <string>
#include <vector>

#include "simCore/Common/Common.h"
#include "simCore/String/Utils.h"

namespace simCore
{

/**
 * @brief Utility class for writing data to an ostream as properly-formatted XML strings
 *
 * This class supports either ASCII input or UTF-8 input, but always encodes the output to UTF-8.
 * This matters when outputting characters above 127 like the degree symbol.
 * An application may write a degree symbol using the ASCII code of 0xb0, but will read
 * the UTF-8 symbol of 0xc2b0.
 *
 * A good article on the topic of character sets and encoding is:
 *
 * https://www.joelonsoftware.com/2003/10/08/the-absolute-minimum-every-software-developer-absolutely-positively-must-know-about-unicode-and-character-sets-no-excuses/
 */
class SDKCORE_EXPORT XmlWriter
{
public:
  /**
   * Constructs a new XmlWriter with the given output stream
   * @param[in ] os Ostream that all XML content will be written to
   */
  explicit XmlWriter(std::ostream& os);

  /** Destructor ensures that all blocks are terminated properly */
  virtual ~XmlWriter();

  /**
   * Sets the indention level.  This is handled automatically through the
   * blocks concept, but this value can be set to force a deeper indent
   * @param[in ] i Number of space characters to prepend to all output
   */
  void setIndent(int i);

  /**
   * Sets the block indention level.  This value determines how many spaces
   * a block is indented inside its parent block. Defaults to 1.
   * @param[in ] i Number of space characters to prepend per block level
   */
  void setBlockIndent(int i);

  /**
   * Sets whether tag comments are written to the output. Defaults to false.
   * @param[in ] fl Value of the setting
   */
  void setUseComments(bool fl);

  /**
   * Sets whether to use minimal or greedy CDATA sections. When on and CDATA is required, as much as possible is
   * embedded in CDATA.  When off, only small subsets of the XML data is embedded in CDATA.  Greedy CDATA mode
   * with encoded carriage returns will result in a binary-equivalent input/output for XML.  Both non-greedy
   * CDATA, and greedy-CDATA without encoded carriage returns, will have minor differences in text line endings.
   * Output text with greedy-CDATA, but encoded carriage returns may be slightly less human-readable.
   * Defaults to false.
   * @param[in ] fl Value of the setting
   */
  void setGreedyCData(bool fl);

  /**
   * Changes whether carriage returns (\r) are encoded as "#xD;" or not.  On by default.
   * @param[in ] fl Value of the setting
   */
  void setEncodeCarriageReturns(bool fl);

  /**
   * Changes the expected input encoding true = ASCII, false =  UTF-8.  Defaults to true/ASCII.
   * @param[in ] fl Value of the setting
   */
  void setAsciiInput(bool fl);

  /**
   * Changes the namespace used for tags.  Automatically prepended to tags with a colon for namespacing.
   * @param[in ] ns String to use for namespace
   */
  void setNamespace(const std::string& ns);

  /**
   * Writes out the comment string with the proper indentation.
   * Note: Ignores the useComments setting.
   * @param[in ] comment Text to write to output as a comment
   */
  void saveComment(const std::string& comment);

  /**
   * Stores an attribute pair to be written in the next block or tag.  May be
   * called multiple times.  Calling with the same attribute name more than once
   * will overwrite the previous value.  Attribute pairs are cleared out once
   * written.  Be sure that attr is a valid attribute name as per XML spec, and
   * ensure that value is a valid encoded string; this function will not error
   * check your input strings.
   * @param[in ] attr Name portion of the attribute name-value pair
   * @param[in ] value Value portion of the attribute name-value pair
   */
  void prepareAttribute(const std::string& attr, const std::string& value);

  /**
   * Starts a new block with given tag and comment, with optional attributes.
   * @param[in ] tag Name of the block element to begin
   * @param[in ] comment Comment string to write before the block, if useComments is true
   * @param[in ] attr String of attribute(s) to include in the block's tag, written exactly as given
   */
  void startBlock(const std::string& tag, const std::string& comment="", const std::string& attr="");

  /** Closes out the current block (call after startBlock()) */
  void endBlock();

  /**
   * Writes the given documentation header <?xml ... ?>.
   * @param[in ] version Value for "version" header attribute, omitted if empty
   * @param[in ] encoding Value for "encoding" header attribute, omitted if empty
   * @param[in ] standalone Boolean value for "standalone" header attribute, translated to string
   */
  void writeXmlDeclaration(const std::string& version = "1.0", const std::string& encoding = "UTF-8", bool standalone = true);

  /**
   * Writes a tag to the stream.  Properly handles comments, data, and attributes.
   * @param[in ] tag String defining the XML element
   * @param[in ] data String written between start and end tags.
   *     If empty, the element will use a self-closing tag
   * @param[in ] comment String written prior to the start tag, if useComments is true
   * @param[in ] attr Attribute string written as-is into the start tag
   */
  void writeTag(const std::string& tag, const std::string& data, const std::string& comment = "", const std::string& attr = "");

  /** Integer variant of writing a tag.
   * @param[in ] tag String defining the XML element
   * @param[in ] data Integer written as string between start and end tags.
   *     If empty, the element will use a self-closing tag
   * @param[in ] comment String written prior to the start tag, if useComments is true
   * @param[in ] attr Attribute string written as-is into the start tag
   */
  void writeTag(const std::string& tag, int data, const std::string& comment = "", const std::string& attr = "");

  /** Unsigned integer variant of writing a tag.
   * @param[in ] tag String defining the XML element
   * @param[in ] data Unsigned int written as string between start and end tags.
   *     If empty, the element will use a self-closing tag
   * @param[in ] comment String written prior to the start tag, if useComments is true
   * @param[in ] attr Attribute string written as-is into the start tag
   */
  void writeTag(const std::string& tag, unsigned int data, const std::string& comment = "", const std::string& attr = "");

  /** Uint64 variant of writing a tag.
   * @param[in ] tag String defining the XML element
   * @param[in ] data Unsigned int written as string between start and end tags.
   *     If empty, the element will use a self-closing tag
   * @param[in ] comment String written prior to the start tag, if useComments is true
   * @param[in ] attr Attribute string written as-is into the start tag
   */
  void writeTag(const std::string& tag, uint64_t data, const std::string& comment = "", const std::string& attr = "");

  /** Double variant of writing a tag.
   * @param[in ] tag String defining the XML element
   * @param[in ] data Double written as string between start and end tags.
   *     If empty, the element will use a self-closing tag
   * @param[in ] comment String written prior to the start tag, if useComments is true
   * @param[in ] attr Attribute string written as-is into the start tag
   */
  void writeTag(const std::string& tag, double data, const std::string& comment = "", const std::string& attr = "");

  /**
   * Raw string variant of writing a tag.  Use this only if your text is
   * preformatted for XML output, e.g. containing CDATA.
   * @param[in ] tag String defining the XML element
   * @param[in ] data String written between start and end tags.
   *     If empty, the element will use a self-closing tag
   * @param[in ] comment String written prior to the start tag, if useComments is true
   * @param[in ] attr Attribute string written as-is into the start tag
   */
  void writeRawTag(const std::string& tag, const std::string& data, const std::string& comment = "", const std::string& attr = "");

private:
  /** Returns appropriate number of spaces for current indent */
  std::string getSpaces_() const;

  /**
   * Encapsulates the entire data string in CDATA tags, properly handling
   * "]]>" strings and (if encodeCarriageReturn is true) "\r" characters.
   * @param[in ] data Data string to wrap
   */
  void writeCDataGreedy_(const std::string& data);

  /**
   * Iterates through the string, wrapping problematic characters in CDATA tags.
   * @param[in ] data Data string to wrap
   */
  void writeCData_(const std::string& data);

  /**
   * Writes out comment if needed, then indent spaces, then <TAG, then attributes.
   * Does not close the XML element.
   * @param[in ] comment Comment string, only written if writeComments_ is true
   * @param[in ] tag String defining the XML element
   * @param[in ] attr Attribute string written as-is into the start tag
   */
  void writeTagStart_(const std::string& comment, const std::string& tag, const std::string& attr);

  /**
   * Writes stored attributes out and clears the vector
   * @param[in ] os Stream object that attributes are written to
   * @param[in ] extraAttribs Attribute string written as-is to os, before stored attributes
   */
  void writeAttribs_(std::ostream& os, const std::string& extraAttribs);

  /**
   * Creates a UTF-8 string from a single ASCII byte
   * @param[in ] ascii ASCII byte to convert
   * @return UTF-8 string corresponding to the given byte
   */
  std::string asciiToUtf8_(const unsigned char ascii) const;

  /**
   * Converts a possibly ASCII string to UTF-8
   * @param[in ] inputString ASCII string to convert
   * @return UTF-8 string corresponding to the given input string
   */
  std::string convertAsciiToUtf8_(const std::string& inputString) const;

  /// New entries added to the front; contains depth-sorted block tags
  std::deque<std::string> blocks_;
  std::ostream& os_;
  int indent_;
  int blockIndent_;
  bool writeComments_;
  bool greedyCData_;
  bool encodeCarriageReturns_;
  bool inputIsAscii_;
  std::string namespace_;

  /** Struct representing an attribute name-value pair */
  struct NameValue
  {
    std::string name;
    std::string value;
    NameValue(const std::string& inName, const std::string& inValue)
      : name(inName),
      value(inValue)
    {}
  };
  std::vector<NameValue> attribs_;
};

}

#endif /* SIMCORE_STRING_XMLWRITER_H */
