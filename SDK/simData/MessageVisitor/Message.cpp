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
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 */
#include "simCore/String/Tokenizer.h"
#include "simData/MessageVisitor/protobuf.h"
#include "simData/MessageVisitor/Message.h"

namespace simData { namespace protobuf {

int getField(google::protobuf::Message& message, std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*>& out, const std::string& path)
{
  std::vector<std::string> tokens;
  simCore::stringTokenizer(tokens, path, ".");
  if (tokens.empty())
    return 1;

  google::protobuf::Message* currentMessage = &message;
  const google::protobuf::FieldDescriptor* lastDesc = NULL;
  for (size_t i = 0; i < tokens.size(); ++i)
  {
    const google::protobuf::Descriptor* msgDesc = currentMessage->GetDescriptor();
    lastDesc = msgDesc->FindFieldByName(tokens[i]);

    if (lastDesc == NULL)
    {
      // The given path does not match the definition of the protobuf
      return 2;
    }

    // Keep going down?
    if (lastDesc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
    {
      // Valid sub-message name, but it does not exist in this instance of the message
      if (!currentMessage->GetReflection()->HasField(*currentMessage, lastDesc))
        return 3;
      // mutable version
      currentMessage = currentMessage->GetReflection()->MutableMessage(const_cast<google::protobuf::Message*>(currentMessage), lastDesc);
      if (currentMessage == NULL)  // Safety catch
        return 4;
    }
    else if (i != tokens.size() - 1)
    {
      // we're not to the last token, but current token was found and is not a message.
      // not a valid path for the input message
      return 5;
    }
  }
  // if assert fails, check that assignment to currentMessage above correctly tests NULL
  assert(currentMessage != NULL);
  // if assert fails, check that assignment to lastDesc above correctly tests NULL
  assert(lastDesc != NULL);

  out.first = currentMessage;
  // if input path specified a message, just return the message with no field
  if (lastDesc->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
    out.second = lastDesc;
  return 0;
}

int clearField(google::protobuf::Message& message, const std::string& path)
{
  std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*> out;
  const int status = simData::protobuf::getField(message, out, path);
  if (0 != status)
    return status;
  if (out.second == NULL)
  {
    // path specified a message, not a field
    return -1;
  }

  // a non-zero return from GetField means that a message was returned in out.first
  assert(out.first != NULL);

  google::protobuf::Message* newMsg = out.first;
  const google::protobuf::Reflection *reflection = newMsg->GetReflection();
  if (out.second->is_repeated())
  {
    if (reflection->FieldSize(*newMsg, out.second) <= 0)
      return -2;
  }
  else if (!reflection->HasField(*newMsg, out.second))
    return -3;

  reflection->ClearField(newMsg, out.second);
  return 0;
}

}
}
