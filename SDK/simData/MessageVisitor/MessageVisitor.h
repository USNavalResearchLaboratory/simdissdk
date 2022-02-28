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
#ifndef SIMDATA_PROTOBUF_MESSAGEVISITOR_H
#define SIMDATA_PROTOBUF_MESSAGEVISITOR_H

#include <string>
#include "simCore/Common/Export.h"

namespace google { namespace protobuf {
  class Message;
  class FieldDescriptor;
} }

namespace simData { namespace protobuf {

/** Class that provides methods allowing a user to visit each entry in a google protobuf hierarchically */
class SDKDATA_EXPORT MessageVisitor
{
public:
  class Visitor;

  /**
  * Starts visitation of a message, calling Visitor::visit() on every message field.  Note
  * that sub-messages are iterated through, but Visitor::visit() is not called on them.
  */
  static void visit(const google::protobuf::Message& message, MessageVisitor::Visitor& v, const std::string& varNameStack = "");

  /** Defines a visitor interface that visits every field in a message, recursively */
  class Visitor
  {
  public:
    virtual ~Visitor() {}

    /**
    * Visits the field 'descriptor' within the message 'message'
    * @param message Message that contains the field descriptor.  Is a message definition only, and can be
    *  used to pull out siblings, reflection, and message/class names
    * @param descriptor Descriptor for the current field.  This is essentially a description of the current
    *  variable, and contains the variable name and class name for the field.
    * @param variableName Will contain the fully qualified name of the variable based on variable names (as
    *  opposed to class names).  This is different from descriptor.full_name(), which uses class names.  For
    *  example, variableName might be "commonPrefs.offset.x", while descriptor.full_name() might return
    *  something like "Position.x".  In other words, variableName is cognizant of scope.
    */
    virtual void visit(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor& descriptor, const std::string& variableName) = 0;
  };
};

} }

#endif /* SIMDATA_PROTOBUF_MESSAGEVISITOR_H */
