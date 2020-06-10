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
#ifndef SIMNOTIFY_NOTIFYHANDLER_H
#define SIMNOTIFY_NOTIFYHANDLER_H

#include <ios>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>

#include "simCore/Common/Common.h"
#include "simNotify/NotifySeverity.h"

namespace simNotify
{

  /**
  * @ingroup Notify
  * @brief Defines the interface to be used for logging messages with the notification system.
  *
  * Defines the interface to be used for logging messages with the notification system.
  * Custom notify handlers are created by extending the NotifyHandler class, and implementing
  * the notify(const std::string &) function to direct output to a specific I/O resource.
  * Support for C++ iostream style logging is provided through the operator<<()
  * implementations.
  *
  * NotifyHandler objects should never be used directly for writing messages.  They should
  * always be accessed through the simNotify::notify() family of functions or macros.
  */
  class SDKNOTIFY_EXPORT NotifyHandler
  {
  public:
    /**
    * Initializes the NotifyHandler object with a default severity level of
    * simNotify::NOTIFY_INFO.
    */
    NotifyHandler();

    virtual ~NotifyHandler();

    /**
    * @brief Set the severity level with which future messages are to be associated.
    *
    * Set the severity level with which future messages are to be associated.
    * Severity level is used to determine the prefix to be written before a
    * message.  The severity level can also be used by NotifyHandler
    * implementations to direct messages associated with different levels to
    * different I/O resources.
    *
    * @param[in ] severity notification severity level to be used when logging messages.
    */
    void setSeverity(NotifySeverity severity);

    /**
    * @brief Retrieve the current severity level.
    *
    * Retrieve the current severity level, as set with setSeverity().  If
    * a severity level has not been explicitly specified with setSeverity(),
    * the default value will be simNotify::NOTIFY_INFO.
    *
    * @return the notification severity level currently used when logging messages.
    */
    NotifySeverity severity() const;

    /**
    * @brief Print a prefix before a message.
    *
    * Print a prefix before a message.  The default implementation prints
    * the name of the current severity level.  Classes extending NotifyHandler
    * can override this function to print a custom prefix.  This function
    * is not (and should not) be called by the NotifyHandler's notify(const std::string &)
    * function.  Instead, it is called by the simNotify::notify() family of functions and
    * macros after a NotifyHandler object has been selected for the specified notification
    * severity level.
    *
    * @see simNotify::notify(NotifySeverity)
    */
    virtual void notifyPrefix();

    /**
    * @brief Write a message to an I/O resource.
    *
    * Write a message to an I/O resource.  This is a pure virtual function
    * which must be implemented by a class that extends NotifyHandler.
    *
    * @param[in ] message the message to be written.
    */
    virtual void notify(const std::string &message) = 0;

    /**
    * @brief Operator for writing data to a stream.
    *
    * Operator to write basic types, or complex types with operator<<() support,
    * to an I/O resource.  Provides C++ iostream like functionality.
    *
    * @param[in ] value value to be converted to a string and written to an I/O resource.
    */
    template <typename T>
    NotifyHandler &operator<<(const T &value)
    {
      lockMutex_();
      stream_ << value;
      std::string output = stream_.str();
      stream_.str("");
      unlockMutex_();
      notify(output);
      return *this;
    }

    /**
    * @brief Operator for writing data to a stream.
    *
    * Operator to write std::string data to an I/O resource.  This
    * is an overload of the template version of operator<<() for more
    * efficient string handling.
    *
    * @param[in ] message string to be written to an I/O resource.
    */
    NotifyHandler &operator<<(const std::string &message);

    /**
    * @brief Operator for writing data to a stream.
    *
    * Operator to write char* data to an I/O resource.  This
    * is an overload of the template version of operator<<() for more
    * efficient string handling.
    *
    * @param[in ] message string to be written to an I/O resource.
    */
    NotifyHandler &operator<<(const char *message);

    /** EOL stream */
    typedef std::ostream &(NotifyHandlerEndlFunction)(std::ostream &);
    /**
    * @brief Operator for writing EOL to a stream.
    *
    * Operator to write an EOL value to a stream.  Works with std::eol.
    */
    NotifyHandler &operator<<(NotifyHandlerEndlFunction &);

    /** Stream manipulator */
    typedef std::ios_base &(NotifyHandlerManipFunction)(std::ios_base &);
    /**
    * @brief Operator for manipulating a stream.
    *
    * Operator to manipulate stream output.  Works with std::ios_base
    * derived manipulators such as hex, dec, oct, fixed, scientific, and
    * setprecision
    */
    NotifyHandler &operator<<(NotifyHandlerManipFunction &manip);

  protected:
    /** Override this to provide a way to lock a mutex for thread safety on notify */
    virtual void lockMutex_() {}
    /** Override this to provide a way to unlock a mutex for thread safety on notify */
    virtual void unlockMutex_() {}

  private:
    NotifySeverity severity_;       ///< The current severity level to be used when writing to an I/O resource.
    std::ostringstream stream_;     ///< Object for converting non-string types to strings for writing to an I/O resource.
  };

  /**
  * @brief Shared pointer for managing NotifyHandler memory.
  *
  * Shared pointer for managing NotifyHandler memory. For situations where
  * multiple objects are using a NotifyHandler, with no clear ownership of the
  * NotifyHandler object. The referenced NotifyHandler object will be deleted
  * automatically when the last object using the NotifyHandler releases the
  * shared pointer. The simCore notify system holds all references to
  * NotifyHandler objects with NotifyHandlePtr.
  */
  typedef std::shared_ptr<NotifyHandler> NotifyHandlerPtr;

}

#endif /* SIMNOTIFY_NOTIFYHANDLER_H */
