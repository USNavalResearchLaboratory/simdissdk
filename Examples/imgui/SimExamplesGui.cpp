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
#include "SimExamplesGui.h"

namespace simExamples {

SimExamplesGui::SimExamplesGui(const std::string& name)
  : OSGEARTH_GUI_BASE_CLASS(name.c_str()),
  defaultFont_(nullptr),
  largeFont_(nullptr),
  largeFontPushed_(false)
{
  setVisible(true);
}

SimExamplesGui::~SimExamplesGui()
{
}

void SimExamplesGui::setDefaultFont(ImFont* font)
{
  if (defaultFont_ != nullptr || font == nullptr)
  {
    assert(0); // Dev error, should only be set once and should be set to a valid font
    return;
  }
  defaultFont_ = font;
}

void SimExamplesGui::setLargeFont(ImFont* font)
{
  if (largeFont_ != nullptr || font == nullptr)
  {
    assert(0); // Dev error, should only be set once and should be set to a valid font
    return;
  }
  largeFont_ = font;
}

void SimExamplesGui::pushLargeFont_()
{
  if (!largeFontPushed_)
  {
    ImGui::PushFont(largeFont_);
    largeFontPushed_ = true;
  }
}

void SimExamplesGui::popLargeFont_()
{
  if (largeFontPushed_)
  {
    ImGui::PopFont();
    largeFontPushed_ = false;
  }
}

void SimExamplesGui::handlePressedKeys_()
{
  for (const auto& [key, func] : keyFuncs_)
  {
    if (ImGui::IsKeyPressed(key))
      func();
  }
}

void SimExamplesGui::addKeyFunc_(ImGuiKey key, const KeyFunc& func)
{
  keyFuncs_[key] = func;
}

}
