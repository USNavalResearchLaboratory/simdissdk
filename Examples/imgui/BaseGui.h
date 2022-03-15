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
#ifndef GUI_BASEGUI_H
#define GUI_BASEGUI_H

#include "imgui.h"
#include "imgui_internal.h"
#include <string>

struct ImFont;
namespace osg { class RenderInfo; }
namespace GUI {

/** Base class for an ImGui GUI window */
class BaseGui
{
public:
  virtual ~BaseGui();
  /** name of the GUI panel */
  virtual const char* name() const final;
  /** render this GUI */
  virtual void draw(osg::RenderInfo& ri) = 0;

  /** Set the default font used by all text in the GUI */
  void setDefaultFont(ImFont* font);
  /** Set the large font used optionally used by text in the GUI. See pushLargeFont_() and popLargeFont_() */
  void setLargeFont(ImFont* font);

protected:
  explicit BaseGui(const std::string& name);

  /** Push the large font onto the font stack. Any text created before calling popLargeFont_() will use the large font. */
  void pushLargeFont_();
  /** Pop the large font off of the font stack. Reverts to using the default font. */
  void popLargeFont_();

private:
  std::string name_;
  ImFont* defaultFont_;
  ImFont* largeFont_;
  bool largeFontPushed_;
};

}

#endif
