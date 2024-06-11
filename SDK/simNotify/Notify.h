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
#ifndef SIMNOTIFY_NOTIFY_H
#define SIMNOTIFY_NOTIFY_H

#include <sstream>
#include "simCore/Common/Common.h"
#include "simNotify/NotifyHandler.h"
#include "simNotify/NotifySeverity.h"

/// Container for classes relating to core functionality
namespace simNotify
{
  /**
  * @defgroup Notify SIMDIS SDK notification system
  * The SIMDIS SDK notification system provides functionality for logging messages
  * to a variety of output resources, such as the console or a file.  Messages are
  * associated with a severity, which describes the importance of the message.
  *
  * The Message severity hierarchy contains seven severity levels, defined by the
  * NotifySeverity enumeration.  The notification system keeps track of a minimum notify
  * severity value that will cause the logging of messages that are associated with
  * lower severity levels to be suppressed.  Setting the minimum notify severity
  * value to simNotify::NOTIFY_INFO will suppress all messages that fall below simNotify::NOTIFY_INFO
  * in the notification severity hierarchy.  The default minimum severity value is
  * simNotify::NOTIFY_NOTICE.
  *
  * Logging of messages is handle by objects implementing the NotifyHandler interface.
  * NotifyHandler supports C++ iostream style operations.  The SIMDIS SDK notification
  * system provides standard notification handlers for logging to stdout, stderr, and
  * files.  Applications needing to log messages to other output sources, such as
  * databases, can do so by implementing a custom NotifyHandler.
  *
  * Different NotifyHandler objects can be individually assigned to the different notify
  * severity levels.  Messages providing general information or warnings can be associated
  * with a notify handler that logs to the console, while messages describing errors or
  * providing debugging information can be associated with a notify handler that logs to a
  * file.  Logging for individual notify severity levels can also be suppressed, independent
  * of the notify severity hierarchy, by associating them with the null notify handler.
  *
  * The notification system's interface has been designed to be compatible with the notify
  * interfaces provided by OpenSceneGraph and osgEarth.  The notify handler object for a
  * specific stream is accessed by calling simNotify::notify(NotifySeverity), and specifying
  * the desired notify severity level for the message, or by using one of the notify
  * macros, such as SIM_NOTICE.  Use of the notify macros is the recommended method
  * for interacting with the notification system.
  *
  * Examples:
  *   // Call NotifyHandler::notify directly, must pre-format the message string
  *   simNotify::notify(simNotify::NOTIFY_NOTICE).notify("Power requirement is 1.21 gigawatts!\n");
  *
  *   // Use operator<<() to format and log a message
  *   simNotify::notify(simNotify::NOTIFY_NOTICE) << "Power requirement is " << std::fixed
  *                                    << std::setprecision(2) << 1.21
  *                                    << " gigawatts!" << std::endl;
  *
  *   // Use the notification macros for brevity and for more efficient message
  *   // suppression (see simNotify::notify(NotifySeverity) documentation for details).
  *   SIM_NOTICE << "Power requirement is " << std::fixed
  *                  << std::setprecision(2) << 1.21
  *                  << " gigawatts!" << std::endl;
  *
  * @{
  */

  /**
  * @brief Retrieve the default notification level.
  *
  * Retrieve the default notification level.  The notification level specifies which
  * messages to suppress and which to display, based on the notify severity value
  * associated with a message.  This function can be used to reset the notification
  * level to the default value.
  *
  * @return a NotifySeverity enumeration value specifying the default notification level.
  */
  SDKNOTIFY_EXPORT NotifySeverity defaultNotifyLevel();

  /**
  * @brief Retrieve the current notification level.
  *
  * Retrieve the current notification level.  The notification level specifies which
  * messages to suppress and which to display, based on the notify severity value
  * associated with a message.  Messages with associated notify severity values equal
  * to or higher than the current notification level in the notify severity hierarchy
  * are displayed.  Messages with associated notify severity values that are lower
  * than the current notification level in the notify severity hierarchy are suppressed.
  *
  * @return a NotifySeverity enumeration value specifying the current notification level.
  */
  SDKNOTIFY_EXPORT NotifySeverity notifyLevel();

  /**
  * @brief Set the current notification level.
  *
  * Set the current notification level.  The notification level specifies which
  * messages to suppress and which to display, based on the notify severity value
  * associated with a message.  Messages with associated notify severity values equal
  * to or higher than the the current notification level in the notify severity hierarchy
  * are displayed.  Messages with associated notify severity values that are lower
  * than the current notification level in the notify severity hierarchy are suppressed.
  *
  * @param[in ] severity the NotifySeverity enumeration value to be assigned to the current notification level.
  */
  SDKNOTIFY_EXPORT void setNotifyLevel(NotifySeverity severity);

  /**
  * @brief Retrieve the display/suppress status for the specified notify severity value.
  *
  * Retrieve the display/suppress status for the specified notify severity value.  Used
  * to determine if a message associated with the specified notify severity value will be
  * displayed or suppressed.
  *
  * @return true if messages associated with the specified notify severity value will
  *         be displayed, false if they will be suppressed.
  *
  */
  SDKNOTIFY_EXPORT bool isNotifyEnabled(NotifySeverity severity);

  /**
  * @brief Retrieve the default notification handler object.
  *
  * Retrieve the default notification handler object.  This function can be used to reset
  * the current notification handler(s) to the default notification handler object.  It is
  * never safe to delete this object.
  *
  * @see setNotifyHandler, setNotifyHandlers
  * @return a shared pointer to the default notification handler object.
  */
  SDKNOTIFY_EXPORT NotifyHandlerPtr defaultNotifyHandler();

  /**
  * @brief Retrieve the null notification handler object.
  *
  * Retrieve the null notification handler object.  The null notification handler is
  * responsible for suppressing messages associated with a notify severity value that
  * is lower than the current notification level.  This object returned by simNotify::notify()
  * when the specified notify severity level is lower than the current notification level.
  * It is never safe to delete this object.
  *
  * @return a shared pointer to the null notification handler object.
  */
  SDKNOTIFY_EXPORT NotifyHandlerPtr nullNotifyHandler();

  /**
  * @brief Retrieve the notification handler object associated with the specified notify severity value.
  *
  * Retrieve the notification handler object associated with the specified notify severity value.
  * It is never safe to delete this object.  A different notification handler can be associated
  * with each notify severity value.
  *
  * @param[in ] severity the notify severity value specifying which notification handler to retrieve.
  * @return a shared pointer to the notification handler object associated with the specified notify
  *         severity value.
  */
  SDKNOTIFY_EXPORT NotifyHandlerPtr notifyHandler(NotifySeverity severity);

  /**
  * @brief Associated a notification handler object with a notify severity value.
  *
  * Associate the specified notification handler object with the specified notify severity value.
  * A different notification handler can be associated with each notify severity value.
  *
  * @param[in ] severity the notify severity value specifying which notification handler to retrieve.
  * @param[in ] handler a shared pointer to the notification handler object to associate with the specified
  *                notify severity value.
  */
  SDKNOTIFY_EXPORT void setNotifyHandler(NotifySeverity severity, NotifyHandlerPtr handler);

  /**
  * @brief Associate all notify severity values with a notification handler object.
  *
  * Associate all notify severity values with the specified notification handler object.
  *
  * @param[in ] handler a shared pointer to the notification handler object to be associated with
  *                all notification levels.
  */
  SDKNOTIFY_EXPORT void setNotifyHandlers(NotifyHandlerPtr handler);

  /**
  * @brief Retrieve a stream for logging messages.
  *
  * Retrieve the stream for logging messages with the specified notify severity
  * value.  A reference to a notification handle object, supporting the
  * standard C++ stream interface accessed with operator<<(), is returned.
  *
  * Example:
  *    simNotify::notify(simNotify::NOTIFY_NOTICE) << "The value of pi is " << std::fixed
  *                                     << std::setprecision(2) << 3.14 << std::endl;
  *
  * When the specified notify severity value is lower than the current notification level,
  * a reference to the null notification handler object is returned to suppress the message.
  * Some macros for invoking this function have been provided, which will turn a call to
  * this function with a suppressed notify severity value int a no-op.  It is more
  * efficient to use the provided macros than to call this function directly.
  *
  * @param[in ] severity the notify severity value specifying the severity level of the message
  *                 to be logged.
  * @return a reference to the notification handler associated with the specified notify
  *         severity value.
  */
  SDKNOTIFY_EXPORT NotifyHandler &notify(NotifySeverity severity);

  /**
  * @brief Retrieve the stream associated with simNotify::NOTIFY_INFO for logging messages.
  *
  * Retrieve the stream associated with the simNotify::NOTIFY_INFO notify severity value for
  * logging messages.  This is an overloaded version of the simNotify::notify() function
  * that always accesses the stream associated with simNotify::NOTIFY_INFO.
  *
  * @return a reference to the notification handler associated with the simNotify::NOTIFY_INFO
  *         notify severity value.
  */
  inline NotifyHandler &notify() { return notify(simNotify::NOTIFY_INFO); }

  /**
   * @brief Representative for notification options (opaque).
   *
   * Opaque class that represents the context of notification.  When compiling the SIMDIS SDK
   * as a static library, each DLL that links to the static library might have its own copy
   * of the NotifyContext.  This could result in each DLL using different handlers for SIM_NOTIFY
   * text output.  To resolve the problem, you can pass the NotifyContext (from
   * simNotify::notifyContext()) from the main application to the DLL, then call
   * simNotify::installNotifyContext() from the DLL.
   */
  class NotifyContext;

  /**
   * @brief Retrieves the opaque notification context for the current module
   *
   * Retrieve the opaque context that manages the current severity and handlers, for the current
   * module.  It is possible when using the SDK as a static library that the main application and
   * DLLs will be initialized with different notification contexts.  You can synchronize the
   * contexts by passing the main application's context to the DLL, and calling the
   * simNotify::installNotifyContext() function from the DLL.
   *
   * Memory is managed entirely inside SDK code.
   *
   * If you are not using the SDK as a static library, you should not require this functionality.
   *
   * @return Opaque pointer to the notification context, for use in installNotifyContext()
   */
  SDKNOTIFY_EXPORT NotifyContext* const notifyContext();

  /**
   * @brief Installs a new notification context to the currently executing module.
   *
   * Configures the currently executing module with a new opaque notification context.  This is
   * intended to synchronize the module's notification settings with those from another module.
   * It is possible when using the SDK as a static library that the main application and
   * DLLs will be initialized with different notification contexts.  You can synchronize the
   * contexts by passing the main application's context to the DLL, and calling the
   * simNotify::installNotifyContext() function from the DLL.
   *
   * Memory is managed entirely inside SDK code.
   *
   * If you are not using the SDK as a static library, you should not require this functionality.
   *
   * @param context Opaque pointer to the notification context, from a previous call to simNotify::notifyContext()
   */
  SDKNOTIFY_EXPORT void installNotifyContext(NotifyContext* const context);

/**
 * Class that instantiates to wrap SIM_WARN, SIM_ALWAYS, etc. so that user strings
 * can be sent to the notifier. This class is expected to be used with the SIM_WARN
 * family of macros to instantiate an object that on destruction sends the output
 * into the appropriate notification handler.
 *
 * This class was written to handle a problem where returning a simNotify::NotifyHandler
 * directly is both inefficient (due to per-"chunk" locking), and prone to multi-threaded
 * crossover of separate chunks. In other words, something like:
 *
 * <code>
 * simNotify::notifyHandler(simNotify::NOTIFY_WARN) << "Chunk1" << "Chunk2\n";
 * </code>
 *
 * ... could interleave output because the two different chunks are output at
 * different times. This class solves that problem by combining chunks into one
 * output sent to the notify handler.
 */
class SingleUseNotifySink
{
public:
  explicit SingleUseNotifySink(simNotify::NotifySeverity severity)
  : severity_(severity),
    enabled_(simNotify::isNotifyEnabled(severity_))
  {
  }

  // Move constructor is required but g++ 4.8 does not correctly support move constructor on stringstream
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__ == 4) && (__GNUC_MINOR__ <= 8)
  SingleUseNotifySink(SingleUseNotifySink&& n)
    : severity_(n.severity_)  // default construct stringstream
  {
  }
#else
  SingleUseNotifySink(SingleUseNotifySink&& n) = default;
#endif
  SingleUseNotifySink& operator=(const SingleUseNotifySink& rhs) = delete;
  SingleUseNotifySink& operator=(SingleUseNotifySink&& rhs) = delete;

  /** Print the text with severity */
  virtual ~SingleUseNotifySink()
  {
    if (enabled_)
      simNotify::notify(severity_).notify(buffer_.str());
  }

  /** Cache the input values in the stringstream */
  template<typename T>
  SingleUseNotifySink& operator <<(const T& val)
  {
    if (enabled_)
      buffer_ << val;
    return *this;
  }

  /** EOL stream */
  typedef std::ostream& (NotifyHandlerEndlFunction)(std::ostream&);

  /**
  * @brief Operator for writing EOL to a stream.
  *
  * Operator to write an EOL value to a stream.  Works with std::endl.
  */
  SingleUseNotifySink& operator<<(NotifyHandlerEndlFunction& endl)
  {
    if (enabled_)
      endl(buffer_);
    return *this;
  }

  /** Stream manipulator */
  typedef std::ios_base& (NotifyHandlerManipFunction)(std::ios_base&);

  /**
  * @brief Operator for manipulating a stream.
  *
  * Operator to manipulate stream output.  Works with std::ios_base
  * derived manipulators such as hex, dec, oct, fixed, scientific, and
  * setprecision
  */
  SingleUseNotifySink& operator<<(NotifyHandlerManipFunction& manip)
  {
    if (enabled_)
      manip(buffer_);
    return *this;
  }

private:
  simNotify::NotifySeverity severity_ = simNotify::NOTIFY_INFO;
  std::stringstream buffer_;

  /** Keep track of enabled state to avoid potentially costly stream operations. */
  bool enabled_ = true;
};

#define SIM_NOTIFY(level) simNotify::SingleUseNotifySink(level)   ///< Notification macro with level specification
#define SIM_ALWAYS SIM_NOTIFY(simNotify::NOTIFY_ALWAYS)     ///< Notification macro using NOTIFY_ALWAYS
#define SIM_FATAL SIM_NOTIFY(simNotify::NOTIFY_FATAL)       ///< Notification macro using NOTIFY_FATAL
#define SIM_ERROR SIM_NOTIFY(simNotify::NOTIFY_ERROR)       ///< Notification macro using NOTIFY_ERROR
#define SIM_WARN SIM_NOTIFY(simNotify::NOTIFY_WARN)         ///< Notification macro using NOTIFY_WARN
#define SIM_NOTICE SIM_NOTIFY(simNotify::NOTIFY_NOTICE)     ///< Notification macro using NOTIFY_NOTICE
#define SIM_INFO SIM_NOTIFY(simNotify::NOTIFY_INFO)         ///< Notification macro using NOTIFY_INFO
#define SIM_DEBUG SIM_NOTIFY(simNotify::NOTIFY_DEBUG_INFO)  ///< Notification macro using NOTIFY_DEBUG_INFO
#define SIM_DEBUG_FP SIM_NOTIFY(simNotify::NOTIFY_DEBUG_FP) ///< Notification macro using NOTIFY_DEBUG_FP

}

/** @} */ // End of Notify Group

#endif /* SIMNOTIFY_NOTIFY_H */
