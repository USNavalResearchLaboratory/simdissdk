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
#ifndef SIMEXAMPLES_SIMEXAMPLESGUI_H
#define SIMEXAMPLES_SIMEXAMPLESGUI_H

#include "imgui.h"
#include "imgui_internal.h"
#include "osgEarth/ImGui/ImGui"
#include <string>

struct ImFont;
namespace osg { class RenderInfo; }

namespace simExamples {

/** Base class for an ImGui GUI window */
class SimExamplesGui : public osgEarth::GUI::BaseGUI
{
public:
  virtual ~SimExamplesGui();

  /** Set the default font used by all text in the GUI */
  void setDefaultFont(ImFont* font);
  /** Set the large font used optionally used by text in the GUI. See pushLargeFont_() and popLargeFont_() */
  void setLargeFont(ImFont* font);

protected:
  explicit SimExamplesGui(const std::string& name);

  /** Push the large font onto the font stack. Any text created before calling popLargeFont_() will use the large font. */
  void pushLargeFont_();
  /** Pop the large font off of the font stack. Reverts to using the default font. */
  void popLargeFont_();

  bool firstDraw_;

private:
  ImFont* defaultFont_;
  ImFont* largeFont_;
  bool largeFontPushed_;
};

}

#endif
