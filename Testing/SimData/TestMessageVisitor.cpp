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
#include <vector>
#include "simCore/Common/SDKAssert.h"
#include "simData/DataTypes.h"
#include "simData/MessageVisitor/Message.h"
#include "simData/MessageVisitor/MessageVisitor.h"
#include "simData/MessageVisitor/protobuf.h"

namespace {

int testGetField()
{
  int rv = 0;
  simData::PlatformPrefs platPrefs;
  platPrefs.mutable_commonprefs()->set_draw(false);
  platPrefs.set_brightness(28);

  // valid field
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 == simData::protobuf::getField(platPrefs, out, "brightness"));
    rv += SDK_ASSERT(out.first != nullptr && out.second != nullptr);
  }
  // valid field
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 == simData::protobuf::getField(platPrefs, out, "icon"));
    rv += SDK_ASSERT(out.first != nullptr && out.second != nullptr);
  }
  // valid submessage
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 == simData::protobuf::getField(platPrefs, out, "commonPrefs"));
    rv += SDK_ASSERT(out.first != nullptr);
    rv += SDK_ASSERT(out.second == nullptr);
  }
  // valid field in submessage
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 == simData::protobuf::getField(platPrefs, out, "commonPrefs.dataDraw"));
    rv += SDK_ASSERT(out.first != nullptr && out.second != nullptr);
  }


  // optional submessages are not found unless they are set
  // valid submessage, but it is optional and empty
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 != simData::protobuf::getField(platPrefs, out, "trackPrefs"));
  }
  // valid field in submessage, but optional and empty
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 != simData::protobuf::getField(platPrefs, out, "trackPrefs.trackDrawMode"));
  }

  // add a setting to the optional submessage
  platPrefs.mutable_trackprefs()->set_linewidth(1.76);

  // valid submessage
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 == simData::protobuf::getField(platPrefs, out, "trackPrefs"));
    rv += SDK_ASSERT(out.first != nullptr);
    rv += SDK_ASSERT(out.second == nullptr);
  }
  // valid field in submessage
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 == simData::protobuf::getField(platPrefs, out, "trackPrefs.trackDrawMode"));
    rv += SDK_ASSERT(out.first != nullptr && out.second != nullptr);
  }


  // test that we handle not finding invalid fields
  // invalid field
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 != simData::protobuf::getField(platPrefs, out, "brghtness"));
  }
  // invalid field in a valid submessage
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 != simData::protobuf::getField(platPrefs, out, "commonPrefs.brghtness"));
  }
  // getField cannot process a valid path prefixed with top level message
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 != simData::protobuf::getField(platPrefs, out, "PlatformPrefs.commonPrefs"));
  }
  // invalid path that begins with valid field (instead of a submessage)
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 != simData::protobuf::getField(platPrefs, out, "brightness.commonPrefs"));
  }
  // field names are case sensitive
  {
    std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
    rv += SDK_ASSERT(0 != simData::protobuf::getField(platPrefs, out, "commonPrefs.datadraw"));
  }

  return rv;
}

int testClearField()
{
  int rv = 0;
  simData::PlatformPrefs platPrefs;
  platPrefs.mutable_commonprefs()->set_draw(false);
  platPrefs.set_brightness(28);

  // failure - invalid field
  rv += SDK_ASSERT(0 != simData::protobuf::clearField(platPrefs, "brghtness"));

  // success - valid field is cleared
  rv += SDK_ASSERT(0 == simData::protobuf::clearField(platPrefs, "brightness"));

  // failure - already cleared
  rv += SDK_ASSERT(0 != simData::protobuf::clearField(platPrefs, "brightness"));

  // failure - ClearField does not clear messages
  rv += SDK_ASSERT(0 != simData::protobuf::clearField(platPrefs, "commonPrefs"));

  // success - valid field in submessage
  rv += SDK_ASSERT(0 == simData::protobuf::clearField(platPrefs, "commonPrefs.draw"));

  return rv;
}

// visitor that finds only fields that are set
class FindSetFieldsVisitor : public simData::protobuf::MessageVisitor::Visitor
{
public:
  explicit FindSetFieldsVisitor(std::vector<std::string>& fieldList)
    : fieldList_(fieldList)
  {
  }

  virtual void visit(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor& descriptor, const std::string& variableName)
  {
    const google::protobuf::Reflection& reflection = *message.GetReflection();
    if (descriptor.is_repeated())
    {
      if (reflection.FieldSize(message, &descriptor) > 0)
        fieldList_.push_back(variableName);
    }
    else if (reflection.HasField(message, &descriptor))
      fieldList_.push_back(variableName);
  }

private:
  std::vector<std::string>& fieldList_;
};

/// Tests a protobuf MessageVisitor: visits PlatformPrefs to find fields that are set, then uses TagStack functionality to find and clear those fields
int testMessageVisitor()
{
  int rv = 0;
  simData::PlatformPrefs platPrefs;
  const bool orig_draw = platPrefs.commonprefs().draw();
  const int orig_brightness = platPrefs.brightness();
  const double orig_linewidth = platPrefs.trackprefs().linewidth();

  platPrefs.mutable_commonprefs()->set_draw(false);
  platPrefs.set_brightness(28);
  platPrefs.mutable_trackprefs()->set_linewidth(1.76);
  // a repeated field, two entries still count as one field
  platPrefs.add_gogfile("abcd");
  platPrefs.add_gogfile("efgh");

  // verify that our fields are set to something different than default
  rv += SDK_ASSERT(platPrefs.commonprefs().draw() != orig_draw);
  rv += SDK_ASSERT(platPrefs.brightness() != orig_brightness);
  rv += SDK_ASSERT(platPrefs.trackprefs().linewidth() != orig_linewidth);
  rv += SDK_ASSERT(platPrefs.gogfile_size() == 2);

  std::vector<std::string> fieldList;
  FindSetFieldsVisitor findSetFieldsVisitor(fieldList);
  simData::protobuf::MessageVisitor::visit(platPrefs, findSetFieldsVisitor);

  // test that we found only the fields that we set
  rv += SDK_ASSERT(fieldList.size() == 4);

  // locate the fields that are set, and then clear them
  for (std::vector<std::string>::const_iterator iter = fieldList.begin(); iter != fieldList.end(); ++iter)
  {
    rv += SDK_ASSERT(0 == simData::protobuf::clearField(platPrefs, *iter));
  }

  rv += SDK_ASSERT(platPrefs.commonprefs().draw() == orig_draw);
  rv += SDK_ASSERT(platPrefs.brightness() == orig_brightness);
  rv += SDK_ASSERT(platPrefs.trackprefs().linewidth() == orig_linewidth);
  rv += SDK_ASSERT(platPrefs.gogfile_size() == 0);

  return rv;
}
}

int TestMessageVisitor(int argc, char* argv[])
{
  int rv = 0;
  rv += testGetField();
  rv += testClearField();
  rv += testMessageVisitor();
  return rv;
}
