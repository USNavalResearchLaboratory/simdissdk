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
#ifndef SIMVIS_GOG_ANNOTATION_H
#define SIMVIS_GOG_ANNOTATION_H

#include "simCore/Common/Common.h"
#include "simVis/GOG/GOGNode.h"
#include "simVis/GOG/Utils.h"
#include "osgEarth/MapNode"


namespace simVis { namespace GOG
{
  class GogNodeInterface;

  /** Display GOG Annotation */
  class SDKVIS_EXPORT TextAnnotation
  {
  public:
    /** Create the annotation from the parser data and GOG meta data */
    GogNodeInterface* deserialize(
      const osgEarth::Config&  conf,
      simVis::GOG::ParserData& p,
      const GOGNodeType&       nodeType,
      const GOGContext&        context,
      const GogMetaData&       metaData,
      osgEarth::MapNode*       mapNode);
  };

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_ANNOTATION_H
