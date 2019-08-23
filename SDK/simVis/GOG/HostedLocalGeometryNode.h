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
#ifndef SIMVIS_GOG_HOSTEDLOCALGEOMETRYNODE_H
#define SIMVIS_GOG_HOSTEDLOCALGEOMETRYNODE_H

#include "osgEarthAnnotation/LocalGeometryNode"

namespace simVis
{

/**
 * Instance of a LocalGeometryNode that ignores setMapNode() calls.  This is useful
 * for relative GOGs that attach to a host platform, because they need to be specified
 * with no map.  If a map were specified, then some features like extrusion won't work.
 */
class /* HEADER-ONLY */ HostedLocalGeometryNode : public osgEarth::Annotation::LocalGeometryNode
{
public:
  HostedLocalGeometryNode(osgEarth::Symbology::Geometry* geometry, const osgEarth::Symbology::Style& style)
    : LocalGeometryNode(geometry, style)
  {
  }

  HostedLocalGeometryNode(osg::Node* node, const osgEarth::Symbology::Style& style)
    : LocalGeometryNode()
  {
      getPositionAttitudeTransform()->addChild(node);
      setStyle(style);
  }

  /// Override setMapNode to ignore values
  virtual void setMapNode(osgEarth::MapNode* mapNode)
  {
    // No-op
  }
};

}

#endif /* SIMVIS_GOG_HOSTEDLOCALGEOMETRYNODE_H */
