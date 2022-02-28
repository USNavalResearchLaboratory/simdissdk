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
 *               EW Modeling and Simulation, Code 5770
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * For more information please send email to simdis@enews.nrl.navy.mil
 *
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * U.S. Naval Research Laboratory.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 */
#include "simData/MessageVisitor/protobuf.h"
#include "simData/MessageVisitor/MessageVisitor.h"

// Protobuf uses GetMessage(), which windows.h renames with #define to GetMessageA
#ifdef WIN32
#undef GetMessage
#endif

namespace simData { namespace protobuf {

void MessageVisitor::visit(const google::protobuf::Message &message, MessageVisitor::Visitor &v, const std::string& varNameStack)
{
  const google::protobuf::Descriptor* desc = message.GetDescriptor();
  const google::protobuf::Reflection* refl = message.GetReflection();
  std::string prefix = varNameStack;
  if (!prefix.empty())
    prefix.append(".");

  for (int k = 0; k < desc->field_count(); ++k)
  {
    const google::protobuf::FieldDescriptor* fieldDesc = desc->field(k);
    // recurse through repeated message if it is a message, otherwise, it should be treated as a single field
    if (fieldDesc->is_repeated() && fieldDesc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
    {
      int size = refl->FieldSize(message, fieldDesc);
      for (int repIdx = 0; repIdx < size; ++repIdx)
      {
        const google::protobuf::Message& newMsg = refl->GetRepeatedMessage(message, fieldDesc, repIdx);
        visit(newMsg, v, prefix + fieldDesc->name());
      }
    }
    else if (fieldDesc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
    {
      // Nested type, recurse
      const google::protobuf::Message& newMsg = refl->GetMessage(message, fieldDesc);
      visit(newMsg, v, prefix + fieldDesc->name());
    }
    else
    {
      // Not nested; do not recurse
      v.visit(message, *fieldDesc, prefix + fieldDesc->name());
    }
  }
}

} }
