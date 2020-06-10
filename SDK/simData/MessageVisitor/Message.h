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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * U.S. Naval Research Laboratory.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 */
#ifndef SIMDATA_PROTOBUF_MESSAGE_H
#define SIMDATA_PROTOBUF_MESSAGE_H

#include <string>
#include "simCore/Common/Export.h"

namespace google {
  namespace protobuf {
    class Message;
    class FieldDescriptor;
  }
}
namespace simData { namespace protobuf {

  /**
  * Returns the sub-message and field descriptor, if any, of the given message matching a given "." separated path (e.g., "piData.platform.create")
  * @param[in] message Message that contains the field descriptor.  Is a message definition only, and can be
  *  used to pull out siblings, reflection, and message/class names
  * @param[out] outPair a std:pair that will receive ptrs to Message and Descriptor for the found field.
  * @param[in] path a "." separated path (e.g., "piData.platform.create")
  * @return 0 if field was found, non-0 if field was not found
  */
  SDKDATA_EXPORT int getField(google::protobuf::Message& message, std::pair<google::protobuf::Message*, const google::protobuf::FieldDescriptor*>& outPair, const std::string& path);

  /**
  * Clears the field specified by the path if found in the given message
  * @param[in,out] message Message that contains the field, and that will have the field cleared
  * @param[in] path a "." separated path (e.g., "piData.platform.create")
  * @return 0 if field was found and cleared, non-0 if field was not found
  */
  SDKDATA_EXPORT int clearField(google::protobuf::Message& message, const std::string& path);
}}

#endif
