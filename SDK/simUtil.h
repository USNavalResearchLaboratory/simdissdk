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
#ifndef SIMDISSDK_SIMUTIL_H
#define SIMDISSDK_SIMUTIL_H

#ifdef _MSC_VER
#pragma message( __FILE__ ": warning <DEPR>: File is deprecated and will be removed in a future release." )
#else
#warning File is deprecated and will be removed in a future release.
#endif

// simUtil/SilverLiningSettings.h is intentionally omitted to avoid commonly missing 3rd party library
// simUtil/TritonSettings.h is intentionally omitted to avoid commonly missing 3rd party library
#include "simUtil/Capabilities.h"
#include "simUtil/DataStoreTestHelper.h"
#include "simUtil/DatumConvert.h"
#include "simUtil/DefaultDataStoreValues.h"
#include "simUtil/DynamicSelectionPicker.h"
#include "simUtil/ExampleControls.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/GogManager.h"
#include "simUtil/GridTransform.h"
#include "simUtil/HudManager.h"
#include "simUtil/HudPositionEditor.h"
#include "simUtil/HudPositionManager.h"
#include "simUtil/IdMapper.h"
#include "simUtil/LayerFactory.h"
#include "simUtil/LineGraphic.h"
#include "simUtil/MapScale.h"
#include "simUtil/MouseDispatcher.h"
#include "simUtil/MouseManipulator.h"
#include "simUtil/MousePositionManipulator.h"
#include "simUtil/NullSkyModel.h"
#include "simUtil/PlatformPopupManipulator.h"
#include "simUtil/PlatformSimulator.h"
#include "simUtil/RecenterEyeOnArea.h"
#include "simUtil/Replaceables.h"
#include "simUtil/ResizeViewManipulator.h"
#include "simUtil/ScreenCoordinateCalculator.h"
#include "simUtil/Shaders.h"
#include "simUtil/StatsHandler.h"
#include "simUtil/StatsSizeFixer.h"
#include "simUtil/StatusText.h"
#include "simUtil/TerrainToggleEffect.h"
#include "simUtil/UnitTypeConverter.h"
#include "simUtil/ViewpointMonitor.h"
#include "simUtil/ViewpointPositions.h"

#endif /* SIMDISSDK_SIMUTIL_H */
