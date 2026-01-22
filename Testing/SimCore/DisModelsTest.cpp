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
#include <sstream>
#include <string>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Formats/DisModels.h"

namespace {

// Test that DisModels can load a *.dis file and parse the contents
int testDisModels()
{
  // Create minimal test DIS Model file with comments
  const std::string modelDisFile = R"(
// Kind: Platform, Domain: Land
1.1.0.0.0.0.0 generic_land
1.1.0.1.0.0.0 category_land

// Country-specific Specialization
1.1.225.1.0.0.0 country_land

// Kind: Munition
2.1.0.1.0.0.0 category_munition
)";

  // Load the file content
  simCore::DisModels disModel;
  std::stringstream ss(modelDisFile);
  if (disModel.loadStream(ss) != 0)
    return 1;

  int rv = 0;
  // At level 0, only exact matches pass
  rv += SDK_ASSERT(disModel.getModel("1.1.0.0.0.0.0", 0) == "generic_land");
  rv += SDK_ASSERT(disModel.getModel("2.1.0.1.0.0.0", 0) == "category_munition");
  // Test that non-matches correctly fail
  rv += SDK_ASSERT(disModel.getModel("1.1.0.0.0.0.1", 0) == "");
  rv += SDK_ASSERT(disModel.getModel("2.1.0.1.0.0.1", 0) == "");

  // Incomplete string should fill with wildcards
  rv += SDK_ASSERT(disModel.getModel("1.1.0.1", 0) == "category_land");

  // Wildcard level 4 should actually skip the third token, country
  rv += SDK_ASSERT(disModel.getModel("1.1.123.1.0.0.0", 4) == "category_land");
  // More specific matches (providing a valid country code in the third token) should be preferred
  // over generic 1.1.0.1.xxx, even when not exact matches
  rv += SDK_ASSERT(disModel.getModel("1.1.225.1.0.0.0", 4) == "country_land");
  rv += SDK_ASSERT(disModel.getModel("1.1.225.1.0.0.1", 4) == "country_land");

  // Loaded 4 models; test clearing
  rv += SDK_ASSERT(disModel.modelCount() == 4);
  rv += SDK_ASSERT(!disModel.empty());
  disModel.clear();
  rv += SDK_ASSERT(disModel.modelCount() == 0);
  rv += SDK_ASSERT(disModel.empty());

  // Previous success should fail
  rv += SDK_ASSERT(disModel.getModel("1.1.0.1", 0) == "");
  // Add the string and succeed
  rv += SDK_ASSERT(disModel.loadModel("1.1.0.1.0.0.0 category_land") == 0);
  rv += SDK_ASSERT(disModel.modelCount() == 1);
  rv += SDK_ASSERT(!disModel.empty());
  rv += SDK_ASSERT(disModel.getModel("1.1.0.1", 0) == "category_land");

  return rv;
}

}

int DisModelsTest(int argc, char* argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testDisModels() == 0);

  std::cout << "simCore DisModelsTest: " << (rv == 0 ? "PASSED" : "FAILED") << "\n";

  return rv;
}
