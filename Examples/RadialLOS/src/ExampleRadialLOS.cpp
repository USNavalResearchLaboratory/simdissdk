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

#include "osgEarthUtil/Controls"
#include "osgEarthSymbology/Geometry"
#include "osgEarthFeatures/Feature"
#include "osgEarthAnnotation/AnnotationEditing"
#include "osgEarthAnnotation/FeatureNode"
#include "osgEarthAnnotation/LocalGeometryNode"

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
    osg::ref_ptr<osgEarth::Annotation::FeatureNode> p2pFeature;

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
          p2pFeature->init();

          if (visible)
            p2p_result->setText("visible");
          else
            p2p_result->setText("obstructed");
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
    vbox->addControl(new LabelControl(s_title, 20, osg::Vec4f(1, 1, 0, 1)));

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

    vbox->addControl(new LabelControl("Drag the crosshairs to test point-to-point LOS."));
    osg::ref_ptr<HBox> resultBox = vbox->addControl(new HBox());
    resultBox->addControl(new LabelControl("P2P result:"));
    app->p2p_result = resultBox->addControl(new LabelControl(""));

    return vbox;
  }

  /**
   * Adapter to fire off a point-to-point LOS test
   */
  struct RunPointToPointLOSCallback : public osgEarth::Annotation::Dragger::PositionChangedCallback
  {
    AppData* app_;
    explicit RunPointToPointLOSCallback(AppData* app) : app_(app) { }

    void onPositionChanged(const osgEarth::Annotation::Dragger* sender, const osgEarth::GeoPoint& position)
    {
      if (sender->getDragging() == false)
      {
        app_->runPointToPointLOS(position);
      }
    }
  };


  /**
   * Creates the crosshairs that you can position to calculate a line of sight
   */
  osg::Node* createP2PGraphics(AppData* app, MapNode* mapNode)
  {
    // create a "crosshairs" cursor for positioning the LOS test:
    osg::ref_ptr<osgEarth::Symbology::MultiGeometry> m = new osgEarth::Symbology::MultiGeometry();

    osg::ref_ptr<osgEarth::Symbology::Geometry> line1 = m->add(new osgEarth::Symbology::LineString());
    line1->push_back(osg::Vec3(-2000.0, 0.0, 0.0));
    line1->push_back(osg::Vec3(2000.0, 0.0, 0.0));

    osg::ref_ptr<osgEarth::Symbology::Geometry> line2 = m->add(new osgEarth::Symbology::LineString());
    line2->push_back(osg::Vec3(0.0, -2000.0, 0.0));
    line2->push_back(osg::Vec3(0.0,  2000.0, 0.0));

    osgEarth::Symbology::Style style;

    osg::ref_ptr<osgEarth::Symbology::LineSymbol> line = style.getOrCreate<osgEarth::Symbology::LineSymbol>();
    line->stroke()->color() = osg::Vec4(1, 1, 0, 1);
    line->stroke()->width() = 5.0f;

    osg::ref_ptr<osgEarth::Symbology::AltitudeSymbol> alt = style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
    alt->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
    alt->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_DRAPE;

    osg::ref_ptr<osgEarth::Annotation::LocalGeometryNode> node =
        new osgEarth::Annotation::LocalGeometryNode(mapNode, m.get(), style);

    node->setPosition(GeoPoint(mapNode->getMapSRS(), RLOS_LON, RLOS_LAT));

    // create a dragger to move the crosshairs around:
    // Note that editor is returned to caller, and owned by caller
    osgEarth::Annotation::GeoPositionNodeEditor* editor = new osgEarth::Annotation::GeoPositionNodeEditor(node.get());
    editor->getPositionDragger()->setColor(osg::Vec4f(1, 1, 1, 1));
    editor->getPositionDragger()->setPickColor(osg::Vec4f(0, 1, 1, 1));
    editor->addChild(node);

    editor->getPositionDragger()->addPositionChangedCallback(new RunPointToPointLOSCallback(app));

    // create a line feature to highlight the point-to-point LOS calculation
    osg::ref_ptr<osgEarth::Symbology::LineString> p2pLine = new osgEarth::Symbology::LineString();
    p2pLine->push_back(osg::Vec3d(RLOS_LON, RLOS_LAT, RLOS_ALT));
    p2pLine->push_back(osg::Vec3d(RLOS_LON, RLOS_LAT, RLOS_ALT));
    style.getOrCreate<osgEarth::AltitudeSymbol>()->technique() == osgEarth::AltitudeSymbol::TECHNIQUE_DRAPE;
    osg::ref_ptr<osgEarth::Features::Feature> feature = new osgEarth::Features::Feature(p2pLine.get(), mapNode->getMapSRS(), style);
    app->p2pFeature = new osgEarth::Annotation::FeatureNode(mapNode, feature.get());
    app->p2pFeature->setNodeMask(0);

    editor->addChild(app->p2pFeature);

    return editor;
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
  scene->getScenario()->addChild(createP2PGraphics(&app, scene->getMapNode()));

  // set the initial eye point
  viewer->getMainView()->setViewpoint(
    osgEarth::Viewpoint("Start", RLOS_LON, RLOS_LAT, RLOS_ALT, 0.0, -45.0, INIT_RANGE_MAX*5000.0),
    5.0);

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}

