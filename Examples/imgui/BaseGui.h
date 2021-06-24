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
#ifndef GUI_BASEGUI_H
#define GUI_BASEGUI_H

#include "imgui.h"
#include "imgui_internal.h"
#include <string>

namespace osg { class RenderInfo; }
namespace GUI {

// base class for and ImGui GUI window
class BaseGui
{
public:
  //! name of the GUI panel
  virtual const char* name() const final;
  //! render this GUI
  virtual void draw(osg::RenderInfo& ri) = 0;

protected:
  explicit BaseGui(const std::string& name);

private:
  std::string name_;
};

}

#endif
