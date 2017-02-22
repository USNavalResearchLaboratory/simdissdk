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
 * Locator Test
 *
 * A unit test program that validates the behavior of the Locator subsystem.
 */

#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"

/// storage of time/space data for our platform
#include "simData/MemoryDataStore.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"

#include "simVis/ScenarioDataStoreAdapter.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"

#include "osgEarth/StringUtils"
#include "osgEarthSymbology/Style"
#include "osgEarthUtil/LatLongFormatter"
#include "osgEarthUtil/MGRSFormatter"
#include "osgEarthAnnotation/LabelNode"
#include "osg/LineWidth"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;
using namespace osgEarth::Annotation;


/// keep a handle, for toggling
static osg::ref_ptr<Control> s_helpControl;

struct App
{
  osg::ref_ptr<simVis::Locator>     root;
  osg::ref_ptr<simVis::LocatorNode> rootNode;
  osg::ref_ptr<CheckBoxControl>     rootCheck;

  osg::ref_ptr<simVis::Locator>     posOffset;
  osg::ref_ptr<simVis::LocatorNode> posOffsetNode;
  osg::ref_ptr<CheckBoxControl>     posOffsetCheck;

  osg::ref_ptr<simVis::Locator>     posOriOffset;
  osg::ref_ptr<simVis::LocatorNode> posOriOffsetNode;
  osg::ref_ptr<CheckBoxControl>     posOriOffsetCheck;

  osg::ref_ptr<simVis::Locator>     oriOffset;
  osg::ref_ptr<simVis::LocatorNode> oriOffsetNode;
  osg::ref_ptr<CheckBoxControl>     oriOffsetCheck;

  osg::ref_ptr<simVis::Locator>     resolvedOriOffset;
  osg::ref_ptr<simVis::LocatorNode> resolvedOriOffsetNode;
  osg::ref_ptr<CheckBoxControl>     resolvedOriOffsetCheck;

  osg::ref_ptr<simVis::Locator>     resolvedPosOriOffset;
  osg::ref_ptr<simVis::LocatorNode> resolvedPosOriOffsetNode;
  osg::ref_ptr<CheckBoxControl>     resolvedPosOriOffsetCheck;

  osg::ref_ptr<HSliderControl>      lat;
  osg::ref_ptr<HSliderControl>      lon;
  osg::ref_ptr<HSliderControl>      alt;

  osg::ref_ptr<HSliderControl>      yaw;
  osg::ref_ptr<HSliderControl>      pitch;
  osg::ref_ptr<HSliderControl>      roll;

  osg::ref_ptr<HSliderControl>      xOffset;
  osg::ref_ptr<HSliderControl>      yOffset;
  osg::ref_ptr<HSliderControl>      zOffset;

  osg::ref_ptr<HSliderControl>      yawOffset;
  osg::ref_ptr<HSliderControl>      pitchOffset;
  osg::ref_ptr<HSliderControl>      rollOffset;

  osg::ref_ptr<HSliderControl>      xOffset2;
  osg::ref_ptr<HSliderControl>      yOffset2;
  osg::ref_ptr<HSliderControl>      zOffset2;

  osg::ref_ptr<HSliderControl>      yawOffset2;
  osg::ref_ptr<HSliderControl>      pitchOffset2;
  osg::ref_ptr<HSliderControl>      rollOffset2;

  const SpatialReference* mapSRS;
  osg::ref_ptr<osg::Group>             graph;

  void update()
  {
    root->setCoordinate(simCore::Coordinate(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD * lat->getValue(), simCore::DEG2RAD * lon->getValue(), alt->getValue()),
      simCore::Vec3(simCore::DEG2RAD * yaw->getValue(), simCore::DEG2RAD * pitch->getValue(), simCore::DEG2RAD * roll->getValue())), 0.0);

    posOffset->setLocalOffsets(
        simCore::Vec3(xOffset->getValue(), yOffset->getValue(), zOffset->getValue()),
        simCore::Vec3());

    oriOffset->setLocalOffsets(
      simCore::Vec3(),
      simCore::Vec3(simCore::DEG2RAD * yawOffset->getValue(), simCore::DEG2RAD * pitchOffset->getValue(), simCore::DEG2RAD * rollOffset->getValue()));

    posOriOffset->setLocalOffsets(
      simCore::Vec3(xOffset->getValue(), yOffset->getValue(), zOffset->getValue()),
      simCore::Vec3(simCore::DEG2RAD * yawOffset->getValue(), simCore::DEG2RAD * pitchOffset->getValue(), simCore::DEG2RAD * rollOffset->getValue()));

    resolvedOriOffset->setLocalOffsets(
      simCore::Vec3(xOffset2->getValue(), yOffset2->getValue(), zOffset2->getValue()),
      simCore::Vec3(simCore::DEG2RAD * yawOffset2->getValue(), simCore::DEG2RAD * pitchOffset2->getValue(), simCore::DEG2RAD * rollOffset2->getValue()));

    resolvedPosOriOffset->setLocalOffsets(
      simCore::Vec3(xOffset2->getValue(), yOffset2->getValue(), zOffset2->getValue()),
      simCore::Vec3(simCore::DEG2RAD * yawOffset2->getValue(), simCore::DEG2RAD * pitchOffset2->getValue(), simCore::DEG2RAD * rollOffset2->getValue()));

    // checkboxes
    rootNode->setNodeMask(rootCheck->getValue() ? ~0 : 0);
    posOffsetNode->setNodeMask(posOffsetCheck->getValue() ? ~0 : 0);
    posOriOffsetNode->setNodeMask(posOriOffsetCheck->getValue() ? ~0 : 0);
    oriOffsetNode->setNodeMask(oriOffsetCheck->getValue() ? ~0 : 0);
    resolvedOriOffsetNode->setNodeMask(resolvedOriOffsetCheck->getValue() ? ~0 : 0);
    resolvedPosOriOffsetNode->setNodeMask(resolvedPosOriOffsetCheck->getValue() ? ~0 : 0);
  }
};

osg::Node* createNode(float s)
{
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  verts->push_back(osg::Vec3(0, 0, 0));
  verts->push_back(osg::Vec3(s, 0, 0));   // E
  verts->push_back(osg::Vec3(0, 0, 0));
  verts->push_back(osg::Vec3(0, s, 0));   // N
  verts->push_back(osg::Vec3(0, 0, 0));
  verts->push_back(osg::Vec3(0, 0, s));   // U

  osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array();
  colors->push_back(osg::Vec4(1, 0, 0, 1));
  colors->push_back(osg::Vec4(1, 0, 0, 1));  //RED
  colors->push_back(osg::Vec4(0, 1, 0, 1));
  colors->push_back(osg::Vec4(0, 1, 0, 1));  //GREEN
  colors->push_back(osg::Vec4(0, 1, 1, 1));
  colors->push_back(osg::Vec4(0, 1, 1, 1));  //CYAN

  osg::ref_ptr<osg::DrawArrays> da = new osg::DrawArrays(GL_LINES, 0, 6);

  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
  geom->setUseVertexBufferObjects(true);
  geom->setVertexArray(verts);
  geom->setColorArray(colors);
  geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
  geom->addPrimitiveSet(da);

  osg::ref_ptr<osg::StateSet> ss = geom->getOrCreateStateSet();
  simVis::setLighting(ss.get(), 0);
  ss->setMode(GL_DEPTH_TEST, 0);
  ss->setAttributeAndModes(new osg::LineWidth(2.0f), 1);

  // geode is returned to caller and owned by caller
  osg::Geode* geode = new osg::Geode();
  geode->addDrawable(geom);

  return geode;
}


#define SCALE 1e6


void setup(App& app)
{
  app.root = new simVis::Locator(app.mapSRS);
  app.rootNode = new simVis::LocatorNode(app.root, createNode(SCALE));
  app.rootNode->addChild(new LabelNode("root"));
  app.graph->addChild(app.rootNode);

  app.posOffset = new simVis::Locator(app.root);
  app.posOffsetNode = new simVis::LocatorNode(app.posOffset, createNode(SCALE));
  app.posOffsetNode->addChild(new LabelNode("posOffset"));
  app.graph->addChild(app.posOffsetNode);

  app.oriOffset = new simVis::Locator(app.root);
  app.oriOffsetNode = new simVis::LocatorNode(app.oriOffset, createNode(SCALE));
  app.oriOffsetNode->addChild(new LabelNode("oriOffset"));
  app.graph->addChild(app.oriOffsetNode);

  app.posOriOffset = new simVis::Locator(app.root);
  app.posOriOffsetNode = new simVis::LocatorNode(app.posOriOffset, createNode(SCALE));
  app.posOriOffsetNode->addChild(new LabelNode("posOriOffset"));
  app.graph->addChild(app.posOriOffsetNode);

  app.resolvedOriOffset = new simVis::ResolvedPositionLocator(app.oriOffset, simVis::Locator::COMP_ALL);
  app.resolvedOriOffsetNode = new simVis::LocatorNode(app.resolvedOriOffset, createNode(SCALE));
  app.resolvedOriOffsetNode->addChild(new LabelNode("resolvedOriOffset"));
  app.graph->addChild(app.resolvedOriOffsetNode);

  app.resolvedPosOriOffset = new simVis::ResolvedPositionLocator(app.posOriOffset, simVis::Locator::COMP_ALL);
  app.resolvedPosOriOffsetNode = new simVis::LocatorNode(app.resolvedPosOriOffset, createNode(SCALE));
  app.resolvedPosOriOffsetNode->addChild(new LabelNode("resolvedPosOriOffset"));
  app.graph->addChild(app.resolvedPosOriOffsetNode);
}


struct UpdateValue : public ControlEventHandler
{
  explicit UpdateValue(App& app) : app_(app) { }
  App& app_;
  void onValueChanged(Control* control) { app_.update(); }
};

struct ResetValue : public ControlEventHandler
{
  ResetValue(App& app, HSliderControl* slider, float value) : app_(app), slider_(slider), value_(value) { }
  App& app_;
  osg::ref_ptr<HSliderControl> slider_;
  float value_;
  void onClick(Control* control) { slider_->setValue(value_); }
};


void addCheck(App& app, Grid* g, const std::string& text, osg::ref_ptr<CheckBoxControl>& check, bool value)
{
  unsigned r = g->getNumRows();
  g->setControl(0, r, new LabelControl(text));
  check = g->setControl(1, r, new CheckBoxControl(value, new UpdateValue(app)));
}


void addSlider(App& app, Grid* g, const std::string& text, osg::ref_ptr<HSliderControl>& slider, float smin, float sset, float smax)
{
  unsigned r = g->getNumRows();
  g->setControl(0, r, new LabelControl(text));
  slider = g->setControl(1, r, new HSliderControl(smin, smax, sset, new UpdateValue(app)));
  slider->setHorizFill(true, 200);
  LabelControl* resetButton = g->setControl(2, r, new LabelControl("0"));
  resetButton->setBackColor(osg::Vec4(.4, .4, .4, 1));
  resetButton->setActiveColor(osg::Vec4(0, 1, 0, 1));
  resetButton->addEventHandler(new ResetValue(app, slider, sset));
  g->setControl(3, r, new LabelControl(slider));
}

Control* createUI(App& app)
{
  osg::ref_ptr<Grid> g = new Grid();
  g->setAbsorbEvents(true);
  g->setChildSpacing(5);

  addCheck(app, g, "Root", app.rootCheck, true);
  addCheck(app, g, "Pos Offset", app.posOffsetCheck, false);
  addCheck(app, g, "Ori Offset", app.oriOffsetCheck, false);
  addCheck(app, g, "Pos/Ori Offset", app.posOriOffsetCheck, false);
  addCheck(app, g, "Ori Offset (resolved)", app.resolvedOriOffsetCheck, false);
  addCheck(app, g, "Pos/Ori Offset (resolved)", app.resolvedPosOriOffsetCheck, false);

  addSlider(app, g, "Lat",  app.lat, -90, 0, 90);
  addSlider(app, g, "Long", app.lon, -180, 0, 180);
  addSlider(app, g, "Altitude", app.alt, 0, 0, 500000);
  addSlider(app, g, "Yaw",  app.yaw, -180, 0, 180);
  addSlider(app, g, "Pitch", app.pitch, -90, 0, 90);
  addSlider(app, g, "Roll", app.roll, -180, 0, 180);
  addSlider(app, g, "X Offset", app.xOffset, -500000, 0, 500000);
  addSlider(app, g, "Y Offset", app.yOffset, -500000, 0, 500000);
  addSlider(app, g, "Z Offset", app.zOffset, -500000, 0, 500000);
  addSlider(app, g, "Yaw Offset", app.yawOffset, -180, 0, 180);
  addSlider(app, g, "Pitch Offset", app.pitchOffset, -90, 0, 90);
  addSlider(app, g, "Roll Offset", app.rollOffset, -180, 0, 180);
  addSlider(app, g, "X Offset (rsv)", app.xOffset2, -500000, 0, 500000);
  addSlider(app, g, "Y Offset (rsv)", app.yOffset2, -500000, 0, 500000);
  addSlider(app, g, "Z Offset (rsv)", app.zOffset2, -500000, 0, 500000);
  addSlider(app, g, "Yaw Offset (rsv)", app.yawOffset2, -180, 0, 180);
  addSlider(app, g, "Pitch Offset (rsv)", app.pitchOffset2, -90, 0, 90);
  addSlider(app, g, "Roll Offset (rsv)", app.rollOffset2, -180, 0, 180);

  s_helpControl = g;
  return g;
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  osg::ArgumentParser argParse(&argc, argv);
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(argParse);
  viewer->setMap(simExamples::createDefaultExampleMap());
  viewer->installDebugHandlers();

  // add sky node
  simExamples::addDefaultSkyNode(viewer);

  App app;
  app.mapSRS = viewer->getSceneManager()->getMap()->getSRS();
  app.graph = new osg::Group();
  viewer->getSceneManager()->getScenario()->addChild(app.graph);

  setup(app);
  viewer->getMainView()->addOverlayControl(createUI(app));
  app.update();

  viewer->getMainView()->setViewpoint(Viewpoint(
    "Start", 0, 0, 0, -45.0, -45.0, 5e6));

  return viewer->run();
}

