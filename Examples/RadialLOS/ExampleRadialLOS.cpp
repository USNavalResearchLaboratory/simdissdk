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
 * Demonstrates the creation of a Radial Line Of Sight (LOS).  A terrain overlay is used to
 * represent the line of sight area for a platform in the midst of terrain altitude data.
 *
 * NOTE:  An Internet connection is required for this example
 */

#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simVis/Viewer.h"
#include "simVis/RadialLOSNode.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simUtil/ExampleResources.h"

#include "osgEarth/Draggers"
#include "osgEarth/Geometry"
#include "osgEarth/Feature"
#include "osgEarth/FeatureNode"
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/Version"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif
//----------------------------------------------------------------------------

// first line, describe the program
static const std::string s_title = "Radial LOS Example";

#define RLOS_LAT          37.33
#define RLOS_LON        -121.85
#define RLOS_ALT         300.00
#define INIT_RANGE_MAX    25.0
#define INIT_RANGE_RES     1.0
#define INIT_AZIM          0.0
#define INIT_AZIM_RES     20.0
#define INIT_FOV         360.0
#define INIT_ALT         RLOS_ALT

//----------------------------------------------------------------------------

namespace
{
  using namespace osgEarth::Util::Controls;

  // Application data for the demo.
  struct AppData
  {
    AppData()
     : los(nullptr),
       mapNode(nullptr),
       p2pFeature(nullptr)
    {
    }

#ifdef HAVE_IMGUI
    float alt = INIT_ALT;
    float azimCenter = INIT_AZIM;
    float fov = INIT_FOV;
    float azimRes = INIT_AZIM_RES;
    float rangeMax = INIT_RANGE_MAX;
    float rangeRes = INIT_RANGE_RES;
    std::string p2pResult;
#endif

    osg::observer_ptr<simVis::RadialLOSNode> los;
    osg::observer_ptr<osgEarth::MapNode>     mapNode;
    osg::observer_ptr<osgEarth::FeatureNode> p2pFeature;

    // Applies the UI control values to the Radial LOS data model.
    void apply()
    {
      simVis::RadialLOS data = los->getDataModel();

#ifdef HAVE_IMGUI
      data.setCentralAzimuth(osgEarth::Angle(azimCenter, osgEarth::Units::DEGREES));
      data.setFieldOfView(osgEarth::Angle(fov, osgEarth::Units::DEGREES));
      data.setAzimuthalResolution(osgEarth::Angle(azimRes, osgEarth::Units::DEGREES));
      data.setMaxRange(osgEarth::Distance(rangeMax, osgEarth::Units::KILOMETERS));
      data.setRangeResolution(osgEarth::Distance(rangeRes, osgEarth::Units::KILOMETERS));
#endif

      los->setDataModel(data);

#ifdef HAVE_IMGUI
      if (alt != los->getCoordinate().alt())
      {
        los->setCoordinate(simCore::Coordinate(
          simCore::COORD_SYS_LLA,
          simCore::Vec3(RLOS_LAT * simCore::DEG2RAD, RLOS_LON * simCore::DEG2RAD, alt)));
      }
      p2pResult.clear();
#endif
    }

    // Runs a p2p LOS test from the origin to the given geopoint.
    void runPointToPointLOS(const osgEarth::GeoPoint& p)
    {
      simCore::Coordinate coord;
      if (simVis::convertGeoPointToCoord(p, coord, mapNode.get()))
      {
        const simVis::RadialLOS& data = los->getDataModel();
        bool visible;
        if (data.getLineOfSight(coord, visible))
        {
          p2pFeature->setNodeMask(~0);
          p2pFeature->getFeature()->getGeometry()->back() = p.vec3d();
#if OSGEARTH_SOVERSION >= 175
          auto newStyle = *p2pFeature->getFeature()->style();
          auto line = newStyle.getOrCreate<osgEarth::LineSymbol>();
#else
          osg::ref_ptr<osgEarth::LineSymbol> line = p2pFeature->getFeature()->style()->getOrCreate<osgEarth::LineSymbol>();
#endif

          if (visible)
          {
#ifdef HAVE_IMGUI
            p2pResult = "visible";
#endif
            line->stroke()->color() = simVis::Color::Lime;
          }
          else
          {
#ifdef HAVE_IMGUI
            p2pResult = "obstructed";
#endif
            line->stroke()->color() = simVis::Color::Red;
          }

#if OSGEARTH_SOVERSION >= 175
          p2pFeature->getFeature()->setStyle(newStyle);
#endif
          p2pFeature->dirty();
        }
        else
        {
#ifdef HAVE_IMGUI
          p2pResult.clear();
#endif
        }
      }
    }
  };
}

//----------------------------------------------------------------------------

namespace
{

#ifdef HAVE_IMGUI
  // ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

class ControlPanel : public simExamples::SimExamplesGui
{
public:
  explicit ControlPanel(AppData& app)
    : simExamples::SimExamplesGui(s_title),
    app_(app)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    if (!isVisible())
      return;

    ImGui::SetNextWindowPos(ImVec2(5, 25), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    bool needUpdate = false;

    if (ImGui::BeginTable("Table", 2))
    {
      float alt = app_.alt;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Altitude MSL", &app_.alt, 0.f, 1000.f, "%.3f m", ImGuiSliderFlags_AlwaysClamp);
      if (alt != app_.alt)
        needUpdate = true;

      float azimCenter = app_.azimCenter;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Central Azimuth", &app_.azimCenter, -180.f, 180.f, "%.3f deg", ImGuiSliderFlags_AlwaysClamp);
      if (azimCenter != app_.azimCenter)
        needUpdate = true;

      float fov = app_.fov;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Field of View", &app_.fov, 10.f, 360.f, "%.3f km", ImGuiSliderFlags_AlwaysClamp);
      if (fov != app_.fov)
        needUpdate = true;

      float azimRes = app_.azimRes;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Azimuth Resolution", &app_.azimRes, 1.f, 40.f, "%.3f deg", ImGuiSliderFlags_AlwaysClamp);
      if (azimRes != app_.azimRes)
        needUpdate = true;

      float rangeMax = app_.rangeMax;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Max Range", &app_.rangeMax, 1.f, 50.f, "%.3f km", ImGuiSliderFlags_AlwaysClamp);
      if (rangeMax != app_.rangeMax)
        needUpdate = true;

      float rangeRes = app_.rangeRes;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Range Resolution", &app_.rangeRes, .5f, 5.f, "%.3f km", ImGuiSliderFlags_AlwaysClamp);
      if (rangeRes != app_.rangeRes)
        needUpdate = true;

      ImGui::EndTable();

      ImGui::Text("Drag the sphere to test point-to-point LOS.");
      if (!app_.p2pResult.empty())
      {
        std::stringstream ss;
        ss << "P2P Result: " << app_.p2pResult;
        ImGui::Text("%s", ss.str().c_str());
      }

      if (needUpdate)
        app_.apply();
    }

    ImGui::End();
  }

private:
  AppData& app_;
};
#endif

#if OSGEARTH_SOVERSION < 173
  /**
   * Adapter to fire off a point-to-point LOS test
   */
  struct RunPointToPointLOSCallback : public osgEarth::Dragger::PositionChangedCallback
  {
    AppData& app_;
    explicit RunPointToPointLOSCallback(AppData& app) : app_(app) { }

    void onPositionChanged(const osgEarth::Dragger* sender, const osgEarth::GeoPoint& position)
    {
      if (sender->getDragging() == false)
      {
        app_.runPointToPointLOS(position);
      }
    }
  };
#endif

  /**
   * Creates the crosshairs that you can position to calculate a line of sight
   */
  osg::Node* createP2PGraphics(AppData& app)
  {
    MapNode* mapNode = app.mapNode.get();
    osgEarth::SphereDragger* dragger = new osgEarth::SphereDragger(mapNode);
    dragger->setPosition(GeoPoint(mapNode->getMapSRS(), RLOS_LON, RLOS_LAT));
    dragger->setColor(simVis::Color::White);
    dragger->setPickColor(simVis::Color::Aqua);

    // create a "crosshairs" cursor for positioning the LOS test:
    osg::ref_ptr<osgEarth::MultiGeometry> m = new osgEarth::MultiGeometry();
    osg::ref_ptr<osgEarth::Geometry> line1 = m->add(new osgEarth::LineString());
    line1->push_back(osg::Vec3(-2000.0, 0.0, 0.0));
    line1->push_back(osg::Vec3(2000.0, 0.0, 0.0));
    osg::ref_ptr<osgEarth::Geometry> line2 = m->add(new osgEarth::LineString());
    line2->push_back(osg::Vec3(0.0, -2000.0, 0.0));
    line2->push_back(osg::Vec3(0.0,  2000.0, 0.0));

    // Configure line style
    osgEarth::Style style;
    osg::ref_ptr<osgEarth::LineSymbol> line = style.getOrCreate<osgEarth::LineSymbol>();
    line->stroke()->color() = simVis::Color::Yellow;
#if OSGEARTH_SOVERSION < 169
    line->stroke()->width() = 5.0f;
#else
    line->stroke()->width() = osgEarth::Distance(5.0f, osgEarth::Units::PIXELS);
#endif
    osg::ref_ptr<osgEarth::AltitudeSymbol> alt = style.getOrCreate<osgEarth::AltitudeSymbol>();
    alt->clamping() = osgEarth::AltitudeSymbol::CLAMP_TO_TERRAIN;
    alt->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_SCENE;
    alt->binding() = osgEarth::AltitudeSymbol::BINDING_VERTEX;

    // Set up LGN to hold the multi-geometry
    osg::ref_ptr<osgEarth::LocalGeometryNode> node = new osgEarth::LocalGeometryNode(m.get(), style);
    node->setMapNode(mapNode);
    node->setPosition(GeoPoint(mapNode->getMapSRS(), RLOS_LON, RLOS_LAT, RLOS_ALT));

    // create a line feature to highlight the point-to-point LOS calculation
    osg::ref_ptr<osgEarth::LineString> p2pLine = new osgEarth::LineString();
    p2pLine->push_back(osg::Vec3d(RLOS_LON, RLOS_LAT, RLOS_ALT));
    p2pLine->push_back(osg::Vec3d(RLOS_LON, RLOS_LAT, RLOS_ALT));
    osg::ref_ptr<osgEarth::Feature> feature = new osgEarth::Feature(p2pLine.get(), mapNode->getMapSRS(), style);
    app.p2pFeature = new osgEarth::FeatureNode(feature.get());
    app.p2pFeature->setMapNode(mapNode);
    app.p2pFeature->setNodeMask(0);

    osg::Group* editorGroup = new osg::Group;
    editorGroup->addChild(dragger);
    editorGroup->addChild(node.get());
    editorGroup->addChild(app.p2pFeature.get());

#if OSGEARTH_SOVERSION < 173
    dragger->addPositionChangedCallback(new RunPointToPointLOSCallback(app));
#else
    dragger->onPositionChanged([&app](const auto* sender, const auto& geoPoint) {
      if (sender && sender->getDragging() == false)
        app.runPointToPointLOS(geoPoint);
      });
#endif
    return editorGroup;
  }
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(simExamples::createRemoteWorldMap());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // Application data:
  AppData app;

  // Initialize the LOS:
  osg::observer_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  app.mapNode = scene->getMapNode();

  app.los = new simVis::RadialLOSNode(app.mapNode.get());
  app.los->setCoordinate(simCore::Coordinate(
    simCore::COORD_SYS_LLA,
    simCore::Vec3(simCore::DEG2RAD * RLOS_LAT, simCore::DEG2RAD * RLOS_LON, RLOS_ALT)));
  app.los->setVisibleColor(osg::Vec4(1, 1, 1, 0.6));
  app.los->setObstructedColor(osg::Vec4(1, 0, 0, 0.6));
  app.los->setActive(true);
  app.apply();

  // Add it to the scene:
  scene->getScenario()->addChild(app.los.get());

  // Create a cursor for positioning a P2P LOS test:
  scene->getScenario()->addChild(createP2PGraphics(app));

  // set the initial eye point
  viewer->getMainView()->setViewpoint(
    osgEarth::Viewpoint("Start", RLOS_LON, RLOS_LAT, RLOS_ALT, 0.0, -45.0, INIT_RANGE_MAX*2000.0));

#ifdef HAVE_IMGUI
  ::GUI::OsgImGuiHandler* gui = new ::GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(app));
#endif

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}

