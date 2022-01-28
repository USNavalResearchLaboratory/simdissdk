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
#include "simNotify/Notify.h"
#include "simCore/String/XmlWriter.h"

namespace
{
  /** Unary predicate for finding the first character with the MSB set */
  class hasMsbSet
  {
  public:
    bool operator()(unsigned char ascii) const
    {
      return (ascii & 0x80) != 0;
    }
  };
}

namespace simCore
{

XmlWriter::XmlWriter(std::ostream& os)
  : os_(os),
  indent_(0),
  blockIndent_(1),
  writeComments_(false),
  greedyCData_(false),
  encodeCarriageReturns_(true),
  inputIsAscii_(true)
{
}

XmlWriter::~XmlWriter()
{
  if (!attribs_.empty())
  {
    SIM_ERROR << "XmlWriter, " << attribs_.size() << " unused attributes when ending document" << std::endl;
  }
  if (!blocks_.empty())
  {
    SIM_ERROR << "XmlWriter, " << blocks_.size() << " unclosed XML elements when ending document" << std::endl;
  }
  while (!blocks_.empty())
    endBlock();
}

void XmlWriter::setIndent(int i)
{
  indent_ = i;
}

void XmlWriter::setBlockIndent(int i)
{
  blockIndent_ = i;
}

void XmlWriter::setUseComments(bool fl)
{
  writeComments_ = fl;
}

void XmlWriter::setGreedyCData(bool fl)
{
  greedyCData_ = fl;
}

void XmlWriter::setEncodeCarriageReturns(bool fl)
{
  encodeCarriageReturns_ = fl;
}

void XmlWriter::setAsciiInput(bool fl)
{
  inputIsAscii_ = fl;
}

void XmlWriter::setNamespace(const std::string& ns)
{
  namespace_ = ns;
  // Force it to end in a colon internally
  if (!namespace_.empty() && namespace_[namespace_.length() - 1] != ':')
    namespace_.append(":");
}

void XmlWriter::saveComment(const std::string& comment)
{
  if (!comment.empty())
    os_ << getSpaces_() << "<!-- " << convertAsciiToUtf8_(comment) << " -->\n";
}

void XmlWriter::prepareAttribute(const std::string& attr, const std::string& value)
{
  // Error on some definite invalid strings
  if (attr.find_first_of("><&\n\t\r") != std::string::npos)
  {
    SIM_ERROR << "XmlWriter::prepareAttribute, invalid attribute string: " << attr << std::endl;
    return;
  }
  attribs_.push_back(NameValue(attr, convertAsciiToUtf8_(value)));
}

void XmlWriter::startBlock(const std::string& tag, const std::string& comment, const std::string& attr)
{
  // Tag string must not start with <
  if (!tag.empty() && tag[0] == '<')
  {
    SIM_ERROR << "XmlWriter::startBlock, invalid tag string: " << tag << std::endl;
    return;
  }

  if (writeComments_)
    saveComment(comment);

  os_ << getSpaces_();
  blocks_.push_front(tag);
  os_ << "<" << namespace_ << tag;
  writeAttribs_(os_, attr);
  os_ << ">\n";
}

void XmlWriter::endBlock()
{
  if (blocks_.empty())
  {
    SIM_ERROR << "XmlWriter::endBlock, no blocks in progress" << std::endl;
    return;
  }

  std::string tag = *(blocks_.begin());
  blocks_.erase(blocks_.begin());
  os_ << getSpaces_() << "</" << namespace_ << tag << ">\n";
}

void XmlWriter::writeXmlDeclaration(const std::string& version, const std::string& encoding, bool standalone)
{
  if (!version.empty())
    prepareAttribute("version", version);
  if (!encoding.empty())
    prepareAttribute("encoding", encoding);

  prepareAttribute("standalone", standalone ? "yes" : "no");
  os_ << "<?xml";
  writeAttribs_(os_, "");
  os_ << "?>\n";
}

void XmlWriter::writeTag(const std::string& tag, const std::string& data, const std::string& comment, const std::string& attr)
{
  writeTagStart_(comment, tag, attr);
  if (data.empty())
  {
    os_ << " />\n";
  }
  // Technically > is not illegal, but use as a proxy for ]]> which is illegal
  else if (data.find_first_of("><&\n\t\r") == std::string::npos)
    os_ << ">" << convertAsciiToUtf8_(data) << "</" << namespace_ << tag << ">\n";
  else
  {
    os_ << ">";
    if (greedyCData_)
      writeCDataGreedy_(convertAsciiToUtf8_(data));
    else
      writeCData_(convertAsciiToUtf8_(data));
    os_ << "</" << namespace_ << tag << ">\n";
  }
}

void XmlWriter::writeTag(const std::string& tag, int data, const std::string& comment, const std::string& attr)
{
  writeTagStart_(comment, tag, attr);
  os_ << ">" << data << "</" << namespace_ << tag << ">\n";
}

void XmlWriter::writeTag(const std::string& tag, unsigned int data, const std::string& comment, const std::string& attr)
{
  writeTagStart_(comment, tag, attr);
  os_ << ">" << data << "</" << namespace_ << tag << ">\n";
}

void XmlWriter::writeTag(const std::string& tag, uint64_t data, const std::string& comment, const std::string& attr)
{
  writeTagStart_(comment, tag, attr);
  os_ << ">" << data << "</" << namespace_ << tag << ">\n";
}

void XmlWriter::writeTag(const std::string& tag, double data, const std::string& comment, const std::string& attr)
{
  writeTagStart_(comment, tag, attr);
  os_ << ">" << data << "</" << namespace_ << tag << ">\n";
}

void XmlWriter::writeRawTag(const std::string& tag, const std::string& data, const std::string& comment, const std::string& attr)
{
  if (data.empty())
  {
    // Empty tag -- just fall back on writeTag() for special treatment
    writeTag(tag, data, comment, attr);
    return;
  }
  writeTagStart_(comment, tag, attr);
  os_ << ">" << convertAsciiToUtf8_(data) << "</" << namespace_ << tag << ">\n";
}

std::string XmlWriter::getSpaces_() const
{
  if ((blocks_.size() * blockIndent_) + indent_ > 0)
    return std::string((blocks_.size() * blockIndent_) + indent_, ' ');
  return "";
}

void XmlWriter::writeCDataGreedy_(const std::string& data)
{
  if (data.find("]]>") != std::string::npos || (data.find('\r') != std::string::npos && encodeCarriageReturns_))
  {
    std::string encoded = simCore::StringUtils::substitute(data, "]]>", "]]]]><![CDATA[>", true);
    // Replace \r with "&#xD;"
    if (encodeCarriageReturns_)
      encoded = simCore::StringUtils::substitute(encoded, "\r", "]]>&#xD;<![CDATA[", true);
    encoded = "<![CDATA[" + encoded + "]]>";
    // Avoid empty CDATA strings
    os_ << simCore::StringUtils::substitute(encoded, "<![CDATA[]]>", "");
  }
  else
    os_ << "<![CDATA[" << data << "]]>";
}

void XmlWriter::writeCData_(const std::string& data)
{
  // might need to divide into multiple CDATA to handle ]]> and carriage returns
  bool inCData = false;

  for (std::string::const_iterator ch = data.begin(); ch != data.end(); ++ch)
  {
    if ((*ch == '\n') || (*ch == '\r'))  // encode carriage return
    {
      if (inCData)
      {
        // Finish CDATA
        os_ << "]]>";
        inCData = false;
      }

      if (encodeCarriageReturns_)
        os_ << "&#xA;";
      else
        os_ << "\r";
      if ((*ch == '\r') && (std::distance(ch, data.end()) > 1))
      {
        // Per https://www.w3.org/TR/REC-xml/#sec-line-ends  \r\n  and \n are translated into \r
        if (*(ch + 1) == '\n')
          ++ch;
      }
    }
    else if ((*ch == ']') && (std::distance(ch, data.end()) > 2))
    {
      // handle the special case where the string has ]]> which needs to be spread across a CDATA
      if ((*(ch + 1) == ']') && (*(ch + 2) == '>'))
      {
        os_ << *ch;
        ++ch;
        os_ << *ch;
        ++ch;
        if (inCData)
        {
          // Finish CDATA
          os_ << "]]>";
          inCData = false;
          os_ << *ch;
        }
        else
        {
          inCData = true;
          os_ << "<![CDATA[" << *ch;
        }
      }
      else // nothing special
      {
        if (inCData)
        {
          // Finish CDATA
          os_ << "]]>";
          inCData = false;
        }

        os_ << *ch;
      }
    }
    else if ((*ch == '<') || (*ch == '&') || (*ch == '\t'))  // Test for special characters
    {
      if (!inCData)
      {
        os_ << "<![CDATA[";
        inCData = true;
      }
      os_ << *ch;
    }
    else  // nothing special
    {
      if (inCData)
      {
        // Finish CDATA
        os_ << "]]>";
        inCData = false;
      }

      os_ << *ch;
    }
  }

  // must finish any active CDATA
  if (inCData)
  {
    // Finish CDATA
    os_ << "]]>";
    inCData = false;
  }
}

void XmlWriter::writeTagStart_(const std::string& comment, const std::string& tag, const std::string& attr)
{
  // Tag string must not start with <
  if (!tag.empty() && tag[0] == '<')
  {
    SIM_ERROR << "XmlWriter::startBlock, invalid tag string: " << tag << std::endl;
    return;
  }

  if (writeComments_)
    saveComment(comment);
  os_ << getSpaces_() << "<" << namespace_ << tag;
  writeAttribs_(os_, attr);
}

void XmlWriter::writeAttribs_(std::ostream& os, const std::string& extraAttribs)
{
  // Legacy attribute support
  if (!extraAttribs.empty())
    os_ << " " << extraAttribs;

  if (attribs_.empty())
    return;
  for (auto i = attribs_.begin(); i != attribs_.end(); ++i)
    os << " " << i->name << "=\"" << i->value << "\"";
  attribs_.clear();
}

std::string XmlWriter::asciiToUtf8_(const unsigned char ascii) const
{
  std::stringstream ss;
  // Simple case: using lower 127 code points
  if ((ascii & 0x80) == 0)
  {
    ss << ascii;
    return ss.str();
  }

  // Take the two MSB and make them the 2 LSB of this new byte
  unsigned char firstByte = 0xc0;
  firstByte |= ((ascii & 0xc0) >> 6);

  // Take the 6 LSB and make them the 6 LSB of the second byte
  unsigned char secondByte = 0x80;
  secondByte |= (ascii & 0x3f);
  ss << firstByte << secondByte;
  return ss.str();
}

std::string XmlWriter::convertAsciiToUtf8_(const std::string& inputString) const
{
  // If the input is not ASCII or if string does not have any characters with the MSB set, just return the string
  if ((!inputIsAscii_) || (std::find_if(inputString.begin(), inputString.end(), hasMsbSet()) == inputString.end()))
    return inputString;

  std::stringstream ss;
  // First character not written to output stream
  size_t startOfString = 0;
  // Always-incrementing counter through the string
  size_t stringPos = 0;
  // Number of continuation bytes we're still expecting since start of a UTF-8 multi-byte
  int continuationBytesNeeded = 0;

  // States for the state machine
  enum States {
    OUTSIDE_UTF8,
    NEED_MORE_TOKENS
  } state = OUTSIDE_UTF8;

  // Loop through the string
  while (stringPos < inputString.length())
  {
    // Cache the character
    const unsigned char ch = inputString[stringPos];

    switch (state)
    {
    case OUTSIDE_UTF8:
      // If character's MSB is set, we're entering UTF-8 multi-byte territory
      if ((ch & 0x80) == 0)
        break;
      // Characters from [startOfString, stringPos) are valid
      ss << inputString.substr(startOfString, stringPos - startOfString);
      startOfString = stringPos;

      // We're going to either detect an error, or change state
      if ((ch & 0x40) != 0)
      {
        // Start of a string.  Detect whether we want one, two, or three more tokens by bit testing
        if ((ch & 0x20) == 0)
          continuationBytesNeeded = 1;
        else if ((ch & 0x10) == 0)
          continuationBytesNeeded = 2;
        else
          continuationBytesNeeded = 3;
        state = NEED_MORE_TOKENS;
      }
      else
      {
        // Error; re-encode as UTF-8, then restart parsing after this character
        ss << asciiToUtf8_(ch);
        startOfString = stringPos + 1;
      }
      break;

    case NEED_MORE_TOKENS:
      // Must start with "10" in MSB, or error
      if ((ch & 0xC0) == 0x80)
      {
        --continuationBytesNeeded;
        // If we're out of bytes, finish writing, and move on; else just keep reading
        if (continuationBytesNeeded == 0)
        {
          ss << inputString.substr(startOfString, 1 + stringPos - startOfString);
          startOfString = stringPos + 1;
          state = OUTSIDE_UTF8;
        }
      }
      else
      {
        // Error; re-encode as UTF-8, then restart parsing after this character
        for (size_t k = startOfString; k < stringPos; ++k)
          ss << asciiToUtf8_(inputString[k]);
        startOfString = stringPos + 1;
        continuationBytesNeeded = 0;
        state = OUTSIDE_UTF8;
      }
      break;
    }

    // Move to next string
    ++stringPos;
  }

  // Write from the start of the string, to the end.  If we're
  // in the middle of a UTF-8 though, we need to re-encode each byte
  if (state == OUTSIDE_UTF8)
    ss << inputString.substr(startOfString);
  else
  {
    for (size_t k = startOfString; k < inputString.length(); ++k)
      ss << asciiToUtf8_(inputString[k]);
  }

  // All done
  return ss.str();
}

}
