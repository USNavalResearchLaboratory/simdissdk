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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMNOTIFY_STANDARDNOTIFYHANDLERS_H
#define SIMNOTIFY_STANDARDNOTIFYHANDLERS_H

#include <fstream>
#include <string>
#include <vector>

#include "simCore/Common/Common.h"
#include "simNotify/NotifyHandler.h"

namespace simNotify
{

  /**
  * @ingroup Notify
  * @brief NotifyHandler implementation for writing messages to a console.
  *
  * NotifyHandler implementation for writing messages to a console through
  * stdout and stderr.  The notification severity level specified through
  * setSeverity(NotifySeverity) dictates the selection of stdout and stderr
  * for writing.  The following severity levels will cause messages to be
  * written to stderr:
  *   ALWAYS, FATAL, and WARN
  * The following severity levels will cause messages to be written to stdout:
  *   NOTICE, INFO, DEBUG_INFO, DEBUG_FP
  */
  class SDKNOTIFY_EXPORT StandardNotifyHandler : public NotifyHandler
  {
  public:
    /**
    * @brief Write a message to the console.
    *
    * Write a message to the console through stderr or stdout.  Selection
    * of stderr or stdout is determined by the NotifyHandler objects
    * severity level.  The following severity levels will cause messages to be
    * written to stderr:
    *   ALWAYS, FATAL, and WARN
    * The following severity levels will cause messages to be written to stdout:
    *   NOTICE, INFO, DEBUG_INFO, DEBUG_FP
    *
    * @param[in ] message the message to be written.
    */
    virtual void notify(const std::string &message) override;
  };

  /**
  * @ingroup Notify
  * @brief NotifyHandler implementation for writing messages to a console,
  * with a prefix for the severity.
  *
  * NotifyHandler implementation for writing messages to a console through
  * stdout and stderr.  The notification severity level specified through
  * setSeverity(NotifySeverity) dictates the selection of stdout and stderr
  * for writing.  The following severity levels will cause messages to be
  * written to stderr:
  *   ALWAYS, FATAL, and WARN
  * The following severity levels will cause messages to be written to stdout:
  *   NOTICE, INFO, DEBUG_INFO, DEBUG_FP
  */
  class SDKNOTIFY_EXPORT PrefixedStandardNotifyHandler : public StandardNotifyHandler
  {
  public:
    /**
    * @brief Write a message to the console, with a severity prefix.
    *
    * Write a message to the console through stderr or stdout.  Selection
    * of stderr or stdout is determined by the NotifyHandler objects
    * severity level.  The following severity levels will cause messages to be
    * written to stderr:
    *   ALWAYS, FATAL, and WARN
    * The following severity levels will cause messages to be written to stdout:
    *   NOTICE, INFO, DEBUG_INFO, DEBUG_FP
    */
    virtual void notifyPrefix() override;
  };

  /**
  * @ingroup Notify
  * @brief NotifyHandler implementation for writing messages to stdout.
  *
  * NotifyHandler implementation for writing messages to the console
  * through stdout.  Messages will always be written to stdout.
  */
  class SDKNOTIFY_EXPORT StdoutNotifyHandler : public NotifyHandler
  {
  public:
    /**
    * @brief Write a message to stdout.
    *
    * Write a message to the console through stdout.
    *
    * @param[in ] message the message to be written.
    */
    virtual void notify(const std::string &message) override;
  };

  /**
  * @ingroup Notify
  * @brief NotifyHandler implementation for writing messages to stderr.
  *
  * NotifyHandler implementation for writing messages to the console
  * through stderr.  Messages will always be written to stderr.
  */
  class SDKNOTIFY_EXPORT StderrNotifyHandler : public NotifyHandler
  {
  public:
    /**
    * @brief Write a message to stderr.
    *
    * Write a message to the console through stderr.
    *
    * @param[in ] message the message to be written.
    */
    virtual void notify(const std::string &message) override;
  };

  /**
  * @ingroup Notify
  * @brief NotifyHandler implementation for writing messages to a file.
  *
  * NotifyHandler implementation for writing messages to a file.
  */
  class SDKNOTIFY_EXPORT FileNotifyHandler : public NotifyHandler
  {
  public:
    /**
    * Initializes the FileNotifyHandler, opening the file with the
    * specified name.  Success or failure of the file open operation
    * can be determined by calling the isValid() function immediately
    * after object construction.
    *
    * @param[in ] filename name of the file to open for writing messages.
    */
    FileNotifyHandler(const std::string &filename);

    /**
    * Closes FileNotifyHandler object's output file on destruction.
    */
    virtual ~FileNotifyHandler();

    /**
    * Reports status of file as valid or invalid.  The file is considered
    * valid if it is open and is ready for writing.  The file is considered
    * invalid if the file is not open (because the open operation performed)
    * or an error was encountered when writing to the file.
    */
    bool isValid() const { return !file_ ? false : true; }

    /**
    * @brief Write a message to a file.
    *
    * Write a message to the file specified at object construction.
    *
    * @param[in ] message the message to be written.
    */
    virtual void notify(const std::string &message) override;

  private:
    std::fstream file_;
  };

  /**
  * @ingroup Notify
  * @brief NotifyHandler implementation for writing messages to a stream.
  *
  * NotifyHandler implementation for writing messages to a stream.  This can
  * be useful for sending data to a file or stringstream.
  */
  class SDKNOTIFY_EXPORT StreamNotifyHandler : public NotifyHandler
  {
  public:
    /**
    * Initializes the StreamNotifyHandler, saving a reference to the
    * output stream specified.  The reference is maintained and
    * written to throughout the lifetime of the StreamNotifyHandler,
    * so it is the responsibility of the caller to ensure the stream
    * is valid during the lifetime of StreamNotifyHandler.
    *
    * @param[in ] os Stream to use for writing messages.
    */
    StreamNotifyHandler(std::ostream& os);

    /**
    * @brief Write a message to a stream.
    *
    * Write a message to the stream specified at object construction.
    *
    * @param[in ] message the message to be written.
    */
    virtual void notify(const std::string &message) override;

  private:
    std::ostream& os_;
  };

  /**
   * Saves messages received in order to send them back out to another notify handler.
   * This is particularly useful if you want to, for example, use simNotify routines but
   * require configuration setup in your application (such as notify level), and therefore
   * cannot instantiate your actual notify handlers until after notify messages might
   * have already been pushed out.
   */
  class SDKNOTIFY_EXPORT CaptureHandler : public simNotify::NotifyHandler
  {
  public:
    // From NotifyHandler:
    virtual void notifyPrefix() override;
    virtual void notify(const std::string& message) override;

    /** Empties the cache of messages */
    void clear();
    /** Returns true if there are no messages */
    bool empty() const;

    /** Write contents to the given notify handler, optionally respecting simNotify::isNotifyEnabled() */
    void writeTo(simNotify::NotifyHandler& handler, bool respectNotifyLevel);

    /**
     * Writes to the global notify handler. Call simNotify::setNotifyHandlers() or equivalent
     * before calling this. There are no options to ignore notify levels when using this function.
     */
    void writeToGlobal();

  private:
    /** Represents a single line sent to the handler, from a single notify() call */
    struct NewLine
    {
      /** Severity at the time of notify() creation */
      simNotify::NotifySeverity severity = simNotify::NOTIFY_ALWAYS;
      /** Multiple messages can be streamed together with operator<<() */
      std::vector<std::string> messages;
    };

    std::vector<NewLine> lines_;
  };

  /** Provides a way to send the same notification messages to multiple handlers. */
  class SDKNOTIFY_EXPORT CompositeHandler : public simNotify::NotifyHandler
  {
  public:
    // From NotifyHandler:
    virtual void notifyPrefix() override;
    virtual void notify(const std::string& message) override;

    /** Adds a handler to process messages. Returns 0 on success. */
    int addHandler(simNotify::NotifyHandlerPtr handler);
    /** Removes a handler from processing. Returns 0 on success. */
    int removeHandler(simNotify::NotifyHandlerPtr handler);

  private:
    std::vector<simNotify::NotifyHandlerPtr> handlers_;
  };


}

#endif /* SIMNOTIFY_STANDARDNOTIFYHANDLERS_H */
