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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <algorithm>
#include "simCore/String/Utils.h"
#include "simCore/String/TextReplacer.h"

namespace simCore
{
  TextReplacer::TextReplacer() {}

  TextReplacer::~TextReplacer()
  {
    for (std::vector<TextReplacer::Replaceable*>::iterator i = replaceables_.begin(); i != replaceables_.end(); ++i)
    {
      delete *i;
    }
  }

  std::string TextReplacer::format(const std::string& formatString)
  {
    if (formatString.empty())
      return "";
    std::string str = formatString;
    for (std::vector<TextReplacer::Replaceable*>::const_iterator i = replaceables_.begin(); i != replaceables_.end(); ++i)
    {
      str = simCore::StringUtils::substitute(str, (*i)->getVariableName(), (*i)->getText());
    }
    return str;
  }

  // on success, the TextReplacer will assume ownership of the Replaceable
  int TextReplacer::addReplaceable(simCore::TextReplacer::Replaceable* r)
  {
    std::vector<TextReplacer::Replaceable*>::const_iterator i = std::find(replaceables_.begin(), replaceables_.end(), r);
    if (i == replaceables_.end())
    {
      replaceables_.push_back(r);
      return 0;
    }
    return 1;
  }

  void TextReplacer::deleteReplaceable(simCore::TextReplacer::Replaceable* r)
  {
    std::vector<TextReplacer::Replaceable*>::iterator i = std::find(replaceables_.begin(), replaceables_.end(), r);
    if (i != replaceables_.end())
      replaceables_.erase(i);
  }
}

