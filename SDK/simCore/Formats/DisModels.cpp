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
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/UtfUtils.h"
#include "simCore/String/Utils.h"
#include "simCore/Formats/DisModels.h"

namespace simCore {

DisModels::DisModels()
{
}

int DisModels::loadFile(const std::string& filename)
{
  if (filename.empty())
    return 0;

  std::ifstream inFile(simCore::streamFixUtf8(simCore::expandEnv(filename)), std::ios::in);
  if (inFile.fail())
    return 1;
  return loadStream(inFile);
}

int DisModels::loadStream(std::istream& is)
{
  if (!is)
    return 1;

  disModels_.clear();
  std::string st;
  while (simCore::getStrippedLine(is, st))
    loadModel(st);
  return 0;
}

int DisModels::loadModel(const std::string& modelsTokens)
{
  std::vector<std::string> tmpvec;
  simCore::quoteCommentTokenizer(modelsTokens, tmpvec);
  const size_t vecLen = tmpvec.size();
  // ignore empty, commented lines, or lines without enough tokens
  if (vecLen < 2)
    return 1;
  // process both legacy 'DIS k.d.c.c.s.s.e modelName' and 'k.d.c.c.s.s.e modelName'; ignore anything after.
  const bool legacyFormat = ((tmpvec[0] == "DIS") && vecLen >= 3);
  const std::string& entityTypeToken = legacyFormat ? tmpvec[1] : tmpvec[0];
  const std::string& modelToken = legacyFormat ? tmpvec[2] : tmpvec[1];
  return loadModel(entityTypeToken, modelToken);
}

int DisModels::loadModel(const std::string& disId, const std::string& modelName)
{
  if (disId.find('.') == std::string::npos)
    return 1;

  std::vector<std::string> sv;
  simCore::stringTokenizer(sv, disId, ".");
  // all 7 components must be specified
  if (sv.size() != 7)
    return 1;

  // if all 7 components are specified, the reconstructed string should match the original
  assert(disId == sv[0] + '.' + sv[1] + '.' + sv[2] + '.' + sv[3] + '.' + sv[4] + '.' + sv[5] + '.' + sv[6]);
  disModels_[disId] = modelName;
  return 0;
}


size_t DisModels::modelCount() const
{
  return disModels_.size();
}

bool DisModels::empty() const
{
  return disModels_.empty();
}

void DisModels::clear()
{
  disModels_.clear();
}

std::string DisModels::getModel(const std::string& disId, unsigned int wildcardLevel) const
{
  if (disModels_.empty())
    return "";

  std::vector<std::string> parts;
  simCore::stringTokenizer(parts, disId, ".");
  if (parts.empty())
    return "";

  while (parts.size() < 7)
    parts.push_back("0");

  // iterative search for best match to the entityType
  for (unsigned int ii = 0; ii <= wildcardLevel; ii++)
  {
    const std::string& etString = entityTypeString_(parts, ii);
    const std::map<std::string, std::string>::const_iterator iter = disModels_.find(etString);
    if (iter != disModels_.end())
      return iter->second;
  }

  return "";
}

std::string DisModels::entityTypeString_(const std::vector<std::string>& parts, unsigned int wildcardLevel) const
{
  // must specify all 7 parts
  assert(parts.size() == 7);
  if (parts.size() != 7)
    return "";

  std::ostringstream buff;
  // The order of entity type is kind.domain.country.category.subcat.specific.extra;
  // but order of the default-model processing here is kind/domain/category/country/...
  // which allows assigning one default model for kind/domain/category that can apply to all countries,
  // while still allowing specific country overrides.
  buff << parts[0];
  buff << "." << ((wildcardLevel >= 6) ? "0" : parts[1]);
  buff << "." << ((wildcardLevel >= 4) ? "0" : parts[2]);
  buff << "." << ((wildcardLevel >= 5) ? "0" : parts[3]);
  buff << "." << ((wildcardLevel >= 3) ? "0" : parts[4]);
  buff << "." << ((wildcardLevel >= 2) ? "0" : parts[5]);
  buff << "." << ((wildcardLevel >= 1) ? "0" : parts[6]);
  return buff.str();
}

}
