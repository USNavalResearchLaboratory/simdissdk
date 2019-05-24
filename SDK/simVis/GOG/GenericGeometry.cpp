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
#include "simVis/GOG/GenericGeometry.h"
#include "simVis/GOG/Utils.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "osgEarthFeatures/Feature"
#include "osgEarthAnnotation/FeatureNode"
#include "osgEarthAnnotation/LocalGeometryNode"

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;

#define LC "[GOG::GenericGeometry] "
