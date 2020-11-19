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
#include <iomanip>
#include <set>

#include "osgEarth/LocalGeometryNode"

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simVis/Utils.h"

#include "simVis/GOG/Annotation.h"
#include "simVis/GOG/Arc.h"
#include "simVis/GOG/Circle.h"
#include "simVis/GOG/Cone.h"
#include "simVis/GOG/Cylinder.h"
#include "simVis/GOG/Ellipse.h"
#include "simVis/GOG/Ellipsoid.h"
#include "simVis/GOG/Hemisphere.h"
#include "simVis/GOG/ImageOverlay.h"
#include "simVis/GOG/LatLonAltBox.h"
#include "simVis/GOG/Line.h"
#include "simVis/GOG/LineSegs.h"
#include "simVis/GOG/Orbit.h"
#include "simVis/GOG/Points.h"
#include "simVis/GOG/Polygon.h"
#include "simVis/GOG/Sphere.h"

#include "simVis/GOG/GOG.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/Loader.h"

//------------------------------------------------------------------------

namespace simVis { namespace GOG {

Loader::Loader(const simCore::GOG::Parser& parser, osgEarth::MapNode* mapNode)
  : parser_(parser),
    mapNode_(mapNode)
{
  referencePosition_ = simCore::Vec3(BSTUR.lat(), BSTUR.lon(), BSTUR.alt());
}

Loader::~Loader()
{}

void Loader::setReferencePosition(const simCore::Vec3& referencePosition)
{
  referencePosition_ = referencePosition;
}

void Loader::loadGogs(std::istream& input, bool attached, GogNodeVector& output) const
{
  std::vector<simCore::GOG::GogShapePtr> gogs;
  parser_.parse(input, gogs);

  for (simCore::GOG::GogShapePtr gog : gogs)
  {
    GogNodeInterfacePtr gogNode = buildGogNode_(gog, attached);
    if (gogNode)
      output.push_back(gogNode);
  }
}

GogNodeInterfacePtr Loader::buildGogNode_(simCore::GOG::GogShapePtr gog, bool attached) const
{
  GogNodeInterfacePtr rv;

  if (attached && !gog->isRelative())
  {
    std::string gogName;
    gog->getName(gogName);
    SIM_WARN << "Attempting to load attached GOG with absolute points, cannot create shape for " << gogName << "\n";
    return nullptr;
  }

  switch (gog->shapeType())
  {
  case simCore::GOG::ShapeType::UNKNOWN:
    break;
  case simCore::GOG::ShapeType::CIRCLE:
  {
    const simCore::GOG::Circle* circle = dynamic_cast<const simCore::GOG::Circle*>(gog.get());
    if (circle)
      rv.reset(Circle::createCircle(*circle, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::LINE:
  {
    const simCore::GOG::Line* line = dynamic_cast<const simCore::GOG::Line*>(gog.get());
    if (line)
      rv.reset(Line::createLine(*line, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::ANNOTATION:
  {
    const simCore::GOG::Annotation* anno = dynamic_cast<const simCore::GOG::Annotation*>(gog.get());
    if (anno)
      rv.reset(TextAnnotation::createAnnotation(*anno, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::ARC:
  {
    const simCore::GOG::Arc* arc = dynamic_cast<const simCore::GOG::Arc*>(gog.get());
    if (arc)
      rv.reset(Arc::createArc(*arc, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::CONE:
  {
    const simCore::GOG::Cone* cone = dynamic_cast<const simCore::GOG::Cone*>(gog.get());
    if (cone)
      rv.reset(Cone::createCone(*cone, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::CYLINDER:
  {
    const simCore::GOG::Cylinder* cyl = dynamic_cast<const simCore::GOG::Cylinder*>(gog.get());
    if (cyl)
      rv.reset(Cylinder::createCylinder(*cyl, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::ELLIPSE:
  {
    const simCore::GOG::Ellipse* ellipse = dynamic_cast<const simCore::GOG::Ellipse*>(gog.get());
    if (ellipse)
      rv.reset(Ellipse::createEllipse(*ellipse, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::ELLIPSOID:
  {
    const simCore::GOG::Ellipsoid* ellipsoid = dynamic_cast<const simCore::GOG::Ellipsoid*>(gog.get());
    if (ellipsoid)
      rv.reset(Ellipsoid::createEllipsoid(*ellipsoid, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::HEMISPHERE:
  {
    const simCore::GOG::Hemisphere* hemi = dynamic_cast<const simCore::GOG::Hemisphere*>(gog.get());
    if (hemi)
      rv.reset(Hemisphere::createHemisphere(*hemi, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::LATLONALTBOX:
  {
    const simCore::GOG::LatLonAltBox* llab = dynamic_cast<const simCore::GOG::LatLonAltBox*>(gog.get());
    if (llab)
      rv.reset(LatLonAltBox::createLatLonAltBox(*llab, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::LINESEGS:
  {
    const simCore::GOG::LineSegs* lineSegs = dynamic_cast<const simCore::GOG::LineSegs*>(gog.get());
    if (lineSegs)
      rv.reset(LineSegs::createLineSegs(*lineSegs, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::ORBIT:
  {
    const simCore::GOG::Orbit* orbit = dynamic_cast<const simCore::GOG::Orbit*>(gog.get());
    if (orbit)
      rv.reset(Orbit::createOrbit(*orbit, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::POINTS:
  {
    const simCore::GOG::Points* points = dynamic_cast<const simCore::GOG::Points*>(gog.get());
    if (points)
      rv.reset(Points::createPoints(*points, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::POLYGON:
  {
    const simCore::GOG::Polygon* poly = dynamic_cast<const simCore::GOG::Polygon*>(gog.get());
    if (poly)
      rv.reset(Polygon::createPolygon(*poly, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::SPHERE:
  {
    const simCore::GOG::Sphere* sphere = dynamic_cast<const simCore::GOG::Sphere*>(gog.get());
    if (sphere)
      rv.reset(Sphere::createSphere(*sphere, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }
  case simCore::GOG::ShapeType::IMAGEOVERLAY:
    const simCore::GOG::ImageOverlay* imageOverlay = dynamic_cast<const simCore::GOG::ImageOverlay*>(gog.get());
    if (imageOverlay)
      rv.reset(ImageOverlay::createImageOverlay(*imageOverlay, attached, referencePosition_, mapNode_.get()));
    else
      assert(0); // parser error, shape type doesn't match class
    break;
  }

  if (rv)
  {
    // set the shape object
    rv->setShapeObject(gog);
    // post processing
    if (rv->osgNode())
    {
      // set the scale
      LoaderUtils::setScale(*(gog.get()), rv->osgNode());
      // turn on blending
      rv->osgNode()->getOrCreateStateSet()->setMode(GL_BLEND, 1);
      // turn off lighting
      simVis::setLighting(rv->osgNode()->getOrCreateStateSet(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
    }
  }
  return rv;
}

} }
