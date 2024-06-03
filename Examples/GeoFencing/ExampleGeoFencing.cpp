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

/**
 * Demonstrates the use of the simCore::GeoFence to monitor a geospatial region.
 */

#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Common/Version.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/GeoFence.h"
#include "simCore/Calc/Geometry.h"
#include "simVis/Constants.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simUtil/ExampleResources.h"

#include "osgEarth/Feature"
#include "osgEarth/FeatureNode"
#include "osgEarth/Geometry"
#include "osgEarth/LineDrawable"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
namespace ui = osgEarth::Util::Controls;
#endif

//----------------------------------------------------------------------------

/** Eye position lat/lon */
inline constexpr double DEFAULT_LON_DEG = -90.;
inline constexpr double DEFAULT_LAT_DEG = 0.;

/** Length of a ray in meters */
inline constexpr double RAY_LENGTH = 6e6;
/** Scale factor for hull triangle/trapezoid, for the "inside the earth" line, relative to 1 earth width WGS_A */
inline constexpr double HULL_INSIDE_MULTIPLIER = 0.85;
/** Scale factor for hull triangle/trapezoid, for the "out in space" line, relative to 1 earth width WGS_A */
inline constexpr double HULL_OUTSIDE_FACTOR = 1.25;

/** Line colors for the rays, in order */
static const std::vector<simVis::Color> LINE_COLORS = {
  simVis::Color::Lime,
  simVis::Color::Fuchsia,
  simVis::Color::Aqua,
};

/** Container for a single fence and its associated graphics */
struct FenceAndGraphics
{
  simCore::GeoFence fence;
  osg::ref_ptr<osg::Node> outline;
  osg::ref_ptr<osg::Node> hull;
  osg::ref_ptr<osgEarth::LineDrawable> rayLines;
};

// Application data for the demo.
struct AppData
{
  /** Configured fences */
  std::vector<FenceAndGraphics> fences;
  /** Map node, needed for adding nodes */
  osg::ref_ptr<osgEarth::MapNode> mapnode;

#ifdef HAVE_IMGUI
  std::string feedbackText;
#else
  osg::ref_ptr<ui::LabelControl> feedbackLabel;
#endif

  /** Sets the feedback text, in IMGUI/non-IMGUI independent way */
  void setFeedbackText(const std::string& text)
  {
#ifdef HAVE_IMGUI
    feedbackText = text;
#else
    feedbackLabel->setText(text);
#endif
  }
};

//----------------------------------------------------------------------------

#ifdef HAVE_IMGUI

struct ControlPanel : public simExamples::SimExamplesGui
{
  explicit ControlPanel(AppData& app)
    : simExamples::SimExamplesGui("GeoFencing Test Example"),
    app_(app)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    if (!isVisible())
      return;

    if (firstDraw_)
    {
      ImGui::SetNextWindowPos(ImVec2(5, 25));
      firstDraw_ = false;
    }
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "The yellow areas are geo-fences.");
    ImGui::Text("Move mouse to test whether inside/outside");
    if (!app_.feedbackText.empty())
      ImGui::Text(app_.feedbackText.c_str());

    ImGui::End();
  }

private:
  AppData& app_;
};

#else
namespace
{
  ui::Control* createUI(AppData* app)
  {
    // vbox returned to caller as caller's memory
    ui::VBox* vbox = new ui::VBox();
    vbox->setAbsorbEvents(true);
    vbox->setVertAlign(ui::Control::ALIGN_TOP);
    vbox->setPadding(10);
    vbox->setBackColor(0.f, 0.f, 0.f, 0.4f);
    vbox->addControl(new ui::LabelControl("GeoFencing Test", 20.f));
    vbox->addControl(new ui::LabelControl("The yellow areas are geo-fences.", simVis::Color::Yellow));
    vbox->addControl(new ui::LabelControl("Move mouse to test whether inside/outside"));
    app->feedbackLabel = vbox->addControl(new ui::LabelControl());

    return vbox;
  }
}
#endif
//----------------------------------------------------------------------------

namespace
{

/// styles a feature, expecting a polygon
void styleAnnotation(osgEarth::Style& style, const simVis::Color& fillColor, bool depthTest)
{
  namespace sym = osgEarth;
  style.getOrCreate<sym::PolygonSymbol>()->fill()->color() = simVis::Color(fillColor, 0.5f);
  style.getOrCreate<sym::LineSymbol>()->stroke()->color() = simVis::Color::White;
  style.getOrCreate<sym::LineSymbol>()->stroke()->width() = 2.f;
  style.getOrCreate<sym::LineSymbol>()->tessellationSize()->set(100, osgEarth::Units::KILOMETERS);
  style.getOrCreate<sym::AltitudeSymbol>()->verticalOffset() = 10000;
  style.getOrCreate<sym::RenderSymbol>()->backfaceCulling() = false;
  style.getOrCreate<sym::RenderSymbol>()->depthTest() = depthTest;
  style.getOrCreate<sym::RenderSymbol>()->clipPlane() = simVis::CLIPPLANE_VISIBLE_HORIZON;
}

/// draws a fence on the map with a filled outline
osg::Node* buildFilledPolygon(const std::vector<simCore::Vec3>& v, osgEarth::MapNode* mapnode, const simVis::Color& fillColor, bool depthTest)
{
  // convert it to an osgEarth geometry:
  osg::ref_ptr<osgEarth::Polygon> geom = new osgEarth::Polygon();
  simCore::CoordinateConverter cc;
  for (unsigned i = 0; i < v.size(); ++i)
  {
    simCore::Coordinate llaCoord;
    cc.convertEcefToGeodetic(simCore::Coordinate(simCore::COORD_SYS_ECEF, v[i]), llaCoord);
    const simCore::Vec3& deg = llaCoord.position() * simCore::RAD2DEG;
    geom->push_back(osg::Vec3d(deg.y(), deg.x(), llaCoord.position().z()));
  }
  geom->open();

  // make and style a feature:
  osg::ref_ptr<osgEarth::Feature> feature = new osgEarth::Feature(geom.get(), mapnode->getMap()->getSRS());
  styleAnnotation(feature->style().mutable_value(), fillColor, depthTest);
  feature->geoInterp() = osgEarth::GEOINTERP_GREAT_CIRCLE;

  osgEarth::FeatureNode* featureNode = new osgEarth::FeatureNode(feature.get());
  featureNode->setMapNode(mapnode);
  return featureNode;
}

/// creates all the fences
void buildFences(AppData& app, simVis::SceneManager* scene)
{
  // fence 1 : a simple poly that doesn't overlap anything.
  {
    std::vector<simCore::Vec3> p;
    p.push_back(simCore::Vec3(34, -121, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(32, -93, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(47, -94, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(45, -122, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(34, -121, 0) * simCore::DEG2RAD);
    FenceAndGraphics newFence;
    newFence.fence = simCore::GeoFence(p, simCore::COORD_SYS_LLA);
    app.fences.push_back(newFence);
  }

  // fence 2 : a fence spanning the north pole!
  {
    std::vector<simCore::Vec3> p;
    p.push_back(simCore::Vec3(60, 0, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(60, 60, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(60, 140, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(75, -140, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(60, 0, 0) * simCore::DEG2RAD);
    FenceAndGraphics newFence;
    newFence.fence = simCore::GeoFence(p, simCore::COORD_SYS_LLA);
    app.fences.push_back(newFence);
  }

  // fence 3 : a fence spanning the south pole!
  {
    std::vector<simCore::Vec3> p;
    p.push_back(simCore::Vec3(-50, -120, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(-50, -140, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(-50, 40, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(-50, 0, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(-50, -120, 0) * simCore::DEG2RAD);
    FenceAndGraphics newFence;
    newFence.fence = simCore::GeoFence(p, simCore::COORD_SYS_LLA);
    app.fences.push_back(newFence);
  }

  // fence 4 : a fence spanning the anti-meridian!
  {
    std::vector<simCore::Vec3> p;
    p.push_back(simCore::Vec3(20, 140, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(-20, 140, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(-20, -140, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(20, -140, 0) * simCore::DEG2RAD);
    FenceAndGraphics newFence;
    newFence.fence = simCore::GeoFence(p, simCore::COORD_SYS_LLA);
    app.fences.push_back(newFence);
  }

  // fence 5 : an invalid geofence (because it's not convex)
  {
    std::vector<simCore::Vec3> p;
    p.push_back(simCore::Vec3(0, 0, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(0, 30, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(30, 30, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(15, 15, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(30, 0, 0) * simCore::DEG2RAD);
    p.push_back(simCore::Vec3(0, 0, 0) * simCore::DEG2RAD);
    FenceAndGraphics newFence;
    newFence.fence = simCore::GeoFence(p, simCore::COORD_SYS_LLA);
    app.fences.push_back(newFence);
  }

  // Create all the hulls
  for (auto& fence : app.fences)
  {
    // Create all the feature nodes for the outline and all the hulls
    const auto& hullTriangles = fence.fence.triangles();
    osg::ref_ptr<osg::Group> hullGroup = new osg::Group;
    for (const auto& tri : hullTriangles)
    {
      std::vector<simCore::Vec3> triangleToTrapEcef;
      for (const auto& v : { tri.a, tri.b, tri.c })
      {
        // Skip the 0,0,0 point
        if (v == simCore::Vec3{0, 0, 0})
          continue;
        // Make a trapezoid instead, avoiding (0,0,0)
        const simCore::Vec3 inside = v.normalize() * simCore::WGS_A * HULL_INSIDE_MULTIPLIER;
        const simCore::Vec3 outside = v.normalize() * simCore::WGS_A * HULL_OUTSIDE_FACTOR;
        if (triangleToTrapEcef.empty())
        {
          triangleToTrapEcef.push_back(inside);
          triangleToTrapEcef.push_back(outside);
        }
        else
        {
          triangleToTrapEcef.push_back(outside);
          triangleToTrapEcef.push_back(inside);
        }
      }
      auto* triangleShape = buildFilledPolygon(triangleToTrapEcef, scene->getMapNode(), simVis::Color::Gray, true);
      hullGroup->addChild(triangleShape);
    }
    // Hulls are disabled unless you mouse over them
    hullGroup->setNodeMask(0);

    // Create the line drawable that represents the most recent testing rays
    osg::ref_ptr<osgEarth::LineDrawable> rayLines = new osgEarth::LineDrawable(GL_LINES);
    rayLines->setColor(simVis::Color::Lime);
    rayLines->setLineWidth(2.f);
    rayLines->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    rayLines->setNodeMask(0);

    // Save to the fence struct
    fence.rayLines = rayLines;
    fence.hull = hullGroup.get();
    fence.outline = buildFilledPolygon(fence.fence.points(), scene->getMapNode(), simVis::Color::Yellow, false);

    // Add visualizations to the map
    scene->getScenario()->addChild(fence.outline.get());
    scene->getScenario()->addChild(fence.hull.get());
    scene->getScenario()->addChild(fence.rayLines.get());
  }
}

/// event handler to test whether mouse clicks are inside a fence.
struct Tester : public osgGA::GUIEventHandler
{
  AppData* app_ = nullptr;
  explicit Tester(AppData* app)
    : app_(app)
  {
  }

  /** On mouse move, test the new ECEF point and print to screen/modify graphics */
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*) override
  {
    if (ea.getEventType() == ea.MOVE)
    {
      osg::Vec3d world;
      if (app_->mapnode->getTerrain()->getWorldCoordsUnderMouse(aa.asView(), ea.getX(), ea.getY(), world))
        testEcef(simCore::Vec3(world.x(), world.y(), world.z()));
      else
        app_->setFeedbackText("Mouse off the terrain.");
    }
    return false;
  }

  /** Performs the testing of a given ECEF point against the fences */
  void testEcef(const simCore::Vec3& ecef)
  {
    bool insideFence = false;
    for (unsigned i = 0; i < app_->fences.size(); ++i)
    {
      const auto& fence = app_->fences[i].fence;
      std::vector<simCore::Ray> rays;
      if (fence.contains(ecef, rays))
      {
        app_->setFeedbackText("Inside a fence!");
        insideFence = true;
        app_->fences[i].hull->setNodeMask(1);
        app_->fences[i].rayLines->setNodeMask(1);
        applyRays(rays, app_->fences[i].rayLines.get());
      }
      else
      {
        app_->fences[i].hull->setNodeMask(0);
        app_->fences[i].rayLines->setNodeMask(0);
      }
    }
    if (!insideFence)
      app_->setFeedbackText("No.");
  }

  /** Given a line drawable, update its ray graphics */
  void applyRays(const std::vector<simCore::Ray>& rays, osgEarth::LineDrawable* graphic)
  {
    if (!graphic)
      return;
    graphic->clear();
    for (size_t k = 0; k < rays.size(); ++k)
    {
      const auto& ray = rays[k];
      graphic->pushVertex(osg::Vec3d(ray.origin.x(), ray.origin.y(), ray.origin.z()));
      graphic->setColor(k * 2 + 0, LINE_COLORS[k % 3]);
      const auto& endPoint = ray.origin + ray.direction.normalize() * RAY_LENGTH;
      graphic->pushVertex(osg::Vec3d(endPoint.x(), endPoint.y(), endPoint.z()));
      graphic->setColor(k * 2 + 1, LINE_COLORS[k % 3]);
    }
    graphic->finish();
  }
};

}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // Application data:
  AppData app;
  app.mapnode = viewer->getSceneManager()->getMapNode();

  // Generate some fences.
  buildFences(app, viewer->getSceneManager());

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(app));
#else
  // Install the UI:
  viewer->getMainView()->addOverlayControl(createUI(&app));
#endif

  viewer->getMainView()->setViewpoint(osgEarth::Viewpoint("start", DEFAULT_LON_DEG, DEFAULT_LAT_DEG, 0., 0, -90, 1e7));

  // Install the click handler:
  viewer->addEventHandler(new Tester(&app));

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}
