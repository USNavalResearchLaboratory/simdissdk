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
#ifndef SIMUTIL_STATUSTEXT_H
#define SIMUTIL_STATUSTEXT_H

#include <osg/ref_ptr>
#include "simCore/Common/Common.h"
#include "simCore/String/TextReplacer.h"

namespace simUtil {

class HudColumnText;
class View;

/**
 * Representation of Status Text that gets used in SIMDIS.  This is a text area that is
 * configurable using a simCore::TextReplacer.  Set up a template status specification,
 * and on each frame the text will update.  For convenience, the class is placed in a
 * MatrixTransform.  Text is aligned lower-left wherever you position the matrix.
 */
class SDKUTIL_EXPORT StatusTextNode : public osg::MatrixTransform
{
public:
  explicit StatusTextNode(simCore::TextReplacerPtr textReplacer);

  /**
   * Display the status as specified by statusSpec
   * @return 0 on success, !0 on error
   */
  int setStatusSpec(const std::string& statusSpec, const osg::Vec4f& color = simVis::Color::White, double fontSize = 12.0, const std::string& font = "arial.ttf");

  /** Override from osg::MatrixTransform to call update_() */
  virtual void traverse(osg::NodeVisitor& nv);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }
  /** Return the class name */
  virtual const char* className() const { return "StatusTextNode"; }

protected:
  /** Destructor */
  virtual ~StatusTextNode();

  /** Build the status text object */
  virtual void create_(const std::string& status, const osg::Vec4f& color, const std::string& font, double fontSize);
  /** Update the existing text object  */
  void update_();

  osg::ref_ptr<simUtil::HudColumnText> statusHudText_;  ///< Pointer to the HudText that contains the status display
  simCore::TextReplacerPtr textReplacer_;               ///< Pointer to the replacer that processes status specification into status text
  std::string statusSpec_;                              ///< Current status specifier from which status display is generated
};

/**
 * Class that manages status display info overlay on the specified view.  This is
 * a StatusTextNode that is able to automatically reposition itself inside a View.
 */
class SDKUTIL_EXPORT StatusText : public StatusTextNode
{
public:
  /// Enumeration of positions for status display
  enum Position
  {
    LEFT_BOTTOM,
    LEFT_CENTER,
    LEFT_TOP
  };

  /** Constructs a new StatusText */
  StatusText(simVis::View* view, simCore::TextReplacerPtr textReplacer, StatusText::Position pos=LEFT_BOTTOM);

  /** Remove the status display */
  void removeFromView();

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }
  /** Return the class name */
  virtual const char* className() const { return "StatusText"; }

protected:
  /** Destructor */
  virtual ~StatusText();

private:
  class FrameEventHandler;
  /**
   * Called by FrameEventHandler when the window re-sizes, passes the
   * width and height (in pixels) to the hud status text to resize
   */
  void resize_(int widthPx, int heightPx);

  /** Build the status text object, positioned based on window coords */
  virtual void create_(const std::string& status, const osg::Vec4f& color, const std::string& font, double fontSize);

private:
  osg::observer_ptr<simVis::View> view_;                ///< Pointer to the view in which the status will be displayed
  osg::ref_ptr<FrameEventHandler> frameEventHandler_;   ///< Pointer to the frame event handler
  Position position_;                                   ///< Position of status display
  int windowWidthPx_;                                   ///< Save a copy of the window width (pixels)
  int windowHeightPx_;                                  ///< Save a copy of the window height (pixels)
};

} // namespace simUtil

#endif // SIMUTIL_STATUSTEXT_H
