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

#include "osgEarth/Controls"
#include "osgEarth/Draggers"
#include "osgEarth/Geometry"
#include "osgEarth/Feature"
#include "osgEarth/FeatureNode"
#include "osgEarth/LocalGeometryNode"

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
     : altitude(NULL),
       azim_center(NULL),
       fov(NULL),
       azim_res(NULL),
       range_max(NULL),
       range_res(NULL),
       p2p_result(NULL),
       los(NULL),
       mapNode(NULL),
       p2pFeature(NULL)
    {
    }

    osg::ref_ptr<HSliderControl> altitude;
    osg::ref_ptr<HSliderControl> azim_center;
    osg::ref_ptr<HSliderControl> fov;
    osg::ref_ptr<HSliderControl> azim_res;
    osg::ref_ptr<HSliderControl> range_max;
    osg::ref_ptr<HSliderControl> range_res;
    osg::ref_ptr<LabelControl>   p2p_result;

    osg::ref_ptr<simVis::RadialLOSNode> los;
    osg::ref_ptr<osgEarth::MapNode>     mapNode;
    osg::ref_ptr<osgEarth::FeatureNode> p2pFeature;

    // Applies the UI control values to the Radial LOS data model.
    void apply()
    {
      simVis::RadialLOS data = los->getDataModel();

      data.setCentralAzimuth(osgEarth::Angle(azim_center->getValue(), osgEarth::Units::DEGREES));
      data.setFieldOfView(osgEarth::Angle(fov->getValue(), osgEarth::Units::DEGREES));
      data.setAzimuthalResolution(osgEarth::Angle(azim_res->getValue(), osgEarth::Units::DEGREES));
      data.setMaxRange(osgEarth::Distance(range_max->getValue(), osgEarth::Units::KILOMETERS));
      data.setRangeResolution(osgEarth::Distance(range_res->getValue(), osgEarth::Units::KILOMETERS));

      los->setDataModel(data);

      if (altitude->getValue() != los->getCoordinate().alt())
      {
        los->setCoordinate(simCore::Coordinate(
          simCore::COORD_SYS_LLA,
          simCore::Vec3(RLOS_LAT*simCore::DEG2RAD, RLOS_LON*simCore::DEG2RAD, altitude->getValue())));
      }

      p2p_result->setText("");
      if (p2pFeature)
        p2pFeature->setNodeMask(0);
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
          osg::ref_ptr<osgEarth::LineSymbol> line = p2pFeature->getFeature()->style()->getOrCreate<osgEarth::LineSymbol>();

          if (visible)
          {
            p2p_result->setText("visible");
            line->stroke()->color() = simVis::Color::Lime;
          }
          else
          {
            p2p_result->setText("obstructed");
            line->stroke()->color() = simVis::Color::Red;
          }

          p2pFeature->dirty();
        }
        else
        {
          p2pFeature->setNodeMask(0);
          p2p_result->setText("error");
        }
      }
    }
  };
}

//----------------------------------------------------------------------------

namespace
{
  using namespace osgEarth::Util::Controls;

  struct ApplyUI : public ControlEventHandler
  {
    explicit ApplyUI(AppData* app) : app_(app) { }
    AppData* app_;
    void onValueChanged(Control* c, bool value) { app_->apply(); }
    void onValueChanged(Control* c, float value) { app_->apply(); }
    void onValueChanged(Control* c, double value) { onValueChanged(c, (float)value); }
  };

  osgEarth::Util::Controls::Control* createUI(AppData* app)
  {
    VBox* vbox = new VBox();
    vbox->setAbsorbEvents(true);
    vbox->setVertAlign(Control::ALIGN_TOP);
    vbox->setPadding(10);
    vbox->setBackColor(0, 0, 0, 0.4);
    vbox->addControl(new LabelControl(s_title, 20, simVis::Color::Yellow));

    osg::ref_ptr<ApplyUI> applyUI = new ApplyUI(app);

    osg::ref_ptr<Grid> g = vbox->addControl(new Grid());
    unsigned row=0, col=0;

    row++;
    g->setControl(col, row, new LabelControl("Altitude MSL"));
    app->altitude = g->setControl(col+1, row, new HSliderControl(0.0, 1000.0, INIT_ALT, applyUI.get()));
    g->setControl(col+2, row, new LabelControl(app->altitude.get()));
    g->setControl(col+3, row, new LabelControl("m"));
    app->altitude->setHorizFill(true, 250.0);

    row++;
    g->setControl(col, row, new LabelControl("Central azimuth"));
    app->azim_center = g->setControl(col+1, row, new HSliderControl(-180.0, 180.0, INIT_AZIM, applyUI.get()));
    g->setControl(col+2, row, new LabelControl(app->azim_center.get()));
    g->setControl(col+3, row, new LabelControl("deg"));

    row++;
    g->setControl(col, row, new LabelControl("Field of view"));
    app->fov = g->setControl(col+1, row, new HSliderControl(10.0, 360.0, INIT_FOV, applyUI.get()));
    g->setControl(col+2, row, new LabelControl(app->fov.get()));
    g->setControl(col+3, row, new LabelControl("deg"));

    row++;
    g->setControl(col, row, new LabelControl("Azimuth resolution"));
    app->azim_res = g->setControl(col+1, row, new HSliderControl(1.0, 40.0, INIT_AZIM_RES, applyUI.get()));
    g->setControl(col+2, row, new LabelControl(app->azim_res.get()));
    g->setControl(col+3, row, new LabelControl("deg"));

    row++;
    g->setControl(col, row, new LabelControl("Max range"));
    app->range_max = g->setControl(col+1, row, new HSliderControl(1.0, 50.0, INIT_RANGE_MAX, applyUI.get()));
    g->setControl(col+2, row, new LabelControl(app->range_max.get()));
    g->setControl(col+3, row, new LabelControl("km"));

    row++;
    g->setControl(col, row, new LabelControl("Range resolution"));
    app->range_res = g->setControl(col+1, row, new HSliderControl(0.5, 5.0, INIT_RANGE_RES, applyUI.get()));
    g->setControl(col+2, row, new LabelControl(app->range_res.get()));
    g->setControl(col+3, row, new LabelControl("km"));

    vbox->addControl(new LabelControl("Drag the sphere to test point-to-point LOS."));
    osg::ref_ptr<HBox> resultBox = vbox->addControl(new HBox());
    resultBox->addControl(new LabelControl("P2P result:"));
    app->p2p_result = resultBox->addControl(new LabelControl(""));

    return vbox;
  }

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
    line->stroke()->width() = 5.0f;
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
    editorGroup->addChild(node);
    editorGroup->addChild(app.p2pFeature);

    dragger->addPositionChangedCallback(new RunPointToPointLOSCallback(app));

    return editorGroup;
  }
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();
  osg::ref_ptr<osgEarth::Map> map = simExamples::createRemoteWorldMap();

  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // Application data:
  AppData app;

  // Install the UI:
  viewer->getMainView()->addOverlayControl(createUI(&app));

  // Initialize the LOS:
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
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
  scene->getScenario()->addChild(app.los);

  // Create a cursor for positioning a P2P LOS test:
  scene->getScenario()->addChild(createP2PGraphics(app));

  // set the initial eye point
  viewer->getMainView()->setViewpoint(
    osgEarth::Viewpoint("Start", RLOS_LON, RLOS_LAT, RLOS_ALT, 0.0, -45.0, INIT_RANGE_MAX*2000.0));

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}

