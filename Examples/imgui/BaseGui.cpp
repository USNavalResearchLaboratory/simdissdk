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
#include "imgui.h"
#include "BaseGui.h"

namespace GUI {

BaseGui::BaseGui(const std::string& name)
  : name_(name),
  defaultFont_(nullptr),
  largeFont_(nullptr),
  largeFontPushed_(false)
{
}

BaseGui::~BaseGui()
{
}

const char* BaseGui::name() const
{
  return name_.c_str();
}

void BaseGui::setDefaultFont(ImFont* font)
{
  if (defaultFont_ != nullptr || font == nullptr)
  {
    assert(0); // Dev error, should only be set once and should be set to a valid font
    return;
  }
  defaultFont_ = font;
}

void BaseGui::setLargeFont(ImFont* font)
{
  if (largeFont_ != nullptr || font == nullptr)
  {
    assert(0); // Dev error, should only be set once and should be set to a valid font
    return;
  }
  largeFont_ = font;
}

void BaseGui::pushLargeFont_()
{
  if (!largeFontPushed_)
  {
    ImGui::PushFont(largeFont_);
    largeFontPushed_ = true;
  }
}

void BaseGui::popLargeFont_()
{
  if (largeFontPushed_)
  {
    ImGui::PopFont();
    largeFontPushed_ = false;
  }
}

}
