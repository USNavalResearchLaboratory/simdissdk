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
#include "simVis/GOG/GOGRegistry.h"
#include "simNotify/Notify.h"

#include "simVis/GOG/Annotation.h"
#include "simVis/GOG/Arc.h"
#include "simVis/GOG/Circle.h"
#include "simVis/GOG/Cylinder.h"
#include "simVis/GOG/Ellipse.h"
#include "simVis/GOG/Ellipsoid.h"
#include "simVis/GOG/Hemisphere.h"
#include "simVis/GOG/LatLonAltBox.h"
#include "simVis/GOG/Line.h"
#include "simVis/GOG/LineSegs.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Points.h"
#include "simVis/GOG/Polygon.h"
#include "simVis/GOG/Sphere.h"

#include "simVis/GOG/Utils.h"

using namespace simVis;
using namespace simVis::GOG;
using namespace osgEarth;

#define LC "[GOG::GOGRegistry] "

namespace
{
  /** Template version of GOGRegistry::Deserializer that calls typename T's deserialize() method */
  template<typename T>
  struct SF : public simVis::GOG::GOGRegistry::Deserializer
  {
    /** Forward the call to the typename T's deserialize() method */
    GogNodeInterface* operator()(
      const osgEarth::Config&  conf,
      simVis::GOG::ParserData& pd,
      const GOGNodeType&       nodeType,
      const GOGContext&        context,
      const GogMetaData&       metaData,
      MapNode*                 mapNode) const
    {
      T ser;
      return ser.deserialize(conf, pd, nodeType, context, metaData, mapNode);
    }
  };
}

namespace simVis { namespace GOG {

GOGRegistry::GOGRegistry(osgEarth::MapNode* mapNode)
 : mapNode_(mapNode)
{
  add("annotation",   new SF<TextAnnotation>());
  add("arc",          new SF<Arc>());
  add("circle",       new SF<Circle>());
  add("cylinder",     new SF<Cylinder>());
  add("ellipse",      new SF<Ellipse>());
  add("ellipsoid",    new SF<Ellipsoid>());
  add("hemisphere",   new SF<Hemisphere>());
  add("latlonaltbox", new SF<LatLonAltBox>());
  add("line",         new SF<Line>());
  add("linesegs",     new SF<LineSegs>());
  add("poly",         new SF<Polygon>());
  add("polygon",      new SF<Polygon>());
  add("points",       new SF<Points>());
  add("sphere",       new SF<Sphere>());
}

GOGRegistry::GOGRegistry(const GOGRegistry& rhs)
 : mapNode_(rhs.mapNode_.get()),
   deserializers_(rhs.deserializers_)
{
  //nop
}

void GOGRegistry::add(const std::string& tag, Deserializer* functor)
{
  deserializers_[tag] = functor;
}

GogNodeInterface* GOGRegistry::createGOG(const Config& conf, const GOGNodeType& nodeType, const Style& overrideStyle, const GOGContext& context, const GogMetaData& metaData, GogFollowData& followData) const
{
  GogNodeInterface* result = NULL;
  std::string key = toLower(conf.key());

  DeserializerTable::const_iterator i = deserializers_.find(key);
  if (i != deserializers_.end())
  {
    const Deserializer* f = i->second.get();

    ParserData parserData(conf, context, metaData.shape);

    // apply any override style params:
    if (!overrideStyle.empty())
      parserData.style_ = parserData.style_.combineWith(overrideStyle);

    result = (*f)(conf, parserData, nodeType, context, metaData, mapNode_.get());

    // get the follow orientation data
    followData.locatorFlags = parserData.locatorComps_;
    followData.orientationOffsets = simCore::Vec3(parserData.localHeadingOffset_->as(Units::RADIANS),
      parserData.localPitchOffset_->as(Units::RADIANS),
      parserData.localRollOffset_->as(Units::RADIANS));

    // post-processing:
    if (result && result->osgNode())
    {
      result->osgNode()->setName(parserData.getName());
      parserData.applyToAnnotationNode(result->osgNode());
      result->osgNode()->getOrCreateStateSet()->setMode(GL_BLEND, 1);
    }
  }
  return result;
}

}}
