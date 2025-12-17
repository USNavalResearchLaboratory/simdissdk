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
 * Locator Test
 *
 * A unit test program that validates the behavior of the Locator subsystem.
 */

#include "osgEarth/LineDrawable"
#include "osgEarth/StringUtils"
#include "osgEarth/Style"
#include "osgEarth/LatLongFormatter"
#include "osgEarth/MGRSFormatter"
#include "osgEarth/LabelNode"

#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"

/// storage of time/space data for our platform
#include "simData/MemoryDataStore.h"

#include "simVis/Locator.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"

using namespace osgEarth;
using namespace osgEarth::Util;

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif

osg::Node* createNode(float s)
{
  osgEarth::LineDrawable* geom = new osgEarth::LineDrawable(GL_LINES);
  geom->allocate(6);

  geom->setVertex(0, osg::Vec3());
  geom->setVertex(1, osg::Vec3(s, 0.f, 0.f));   // E
  geom->setVertex(2, osg::Vec3());
  geom->setVertex(3, osg::Vec3(0.f, s, 0.f));   // N
  geom->setVertex(4, osg::Vec3());
  geom->setVertex(5, osg::Vec3(0.f, 0.f, s));   // U
  geom->dirty();

  geom->setColor(0, simVis::Color::Red);
  geom->setColor(1, simVis::Color::Red);
  geom->setColor(2, simVis::Color::Lime);
  geom->setColor(3, simVis::Color::Lime);
  geom->setColor(4, simVis::Color::Aqua);
  geom->setColor(5, simVis::Color::Aqua);

  osg::ref_ptr<osg::StateSet> ss = geom->getOrCreateStateSet();
  simVis::setLighting(ss.get(), 0);
  ss->setMode(GL_DEPTH_TEST, 0);

  geom->setLineWidth(2.0f);

  return geom;
}


#define SCALE 1e6

#ifdef HAVE_IMGUI

// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(150); func("##" label, __VA_ARGS__)

struct ControlPanel : public simExamples::SimExamplesGui
{
  ControlPanel(const SpatialReference* mapSRS, osg::Group* graph)
    : simExamples::SimExamplesGui("Locator Test"),
    mapSRS_(mapSRS),
    graph_(graph)
  {
    root_ = new simVis::Locator();
    rootNode_ = new simVis::LocatorNode(root_.get(), createNode(SCALE));
    rootNode_->addChild(new LabelNode("root"));
    graph_->addChild(rootNode_.get());

    posOffset_ = new simVis::Locator(root_.get());
    posOffsetNode_ = new simVis::LocatorNode(posOffset_.get(), createNode(SCALE));
    posOffsetNode_->addChild(new LabelNode("posOffset"));
    graph_->addChild(posOffsetNode_.get());

    oriOffset_ = new simVis::Locator(root_.get());
    oriOffsetNode_ = new simVis::LocatorNode(oriOffset_.get(), createNode(SCALE));
    oriOffsetNode_->addChild(new LabelNode("oriOffset"));
    graph_->addChild(oriOffsetNode_.get());

    posOriOffset_ = new simVis::Locator(root_.get());
    posOriOffsetNode_ = new simVis::LocatorNode(posOriOffset_.get(), createNode(SCALE));
    posOriOffsetNode_->addChild(new LabelNode("posOriOffset"));
    graph_->addChild(posOriOffsetNode_.get());

    resolvedOriOffset_ = new simVis::ResolvedPositionLocator(oriOffset_.get(), simVis::Locator::COMP_ALL);
    resolvedOriOffsetNode_ = new simVis::LocatorNode(resolvedOriOffset_.get(), createNode(SCALE));
    resolvedOriOffsetNode_->addChild(new LabelNode("resolvedOriOffset"));
    graph_->addChild(resolvedOriOffsetNode_.get());

    resolvedPosOriOffset_ = new simVis::ResolvedPositionLocator(posOriOffset_.get(), simVis::Locator::COMP_ALL);
    resolvedPosOriOffsetNode_ = new simVis::LocatorNode(resolvedPosOriOffset_.get(), createNode(SCALE));
    resolvedPosOriOffsetNode_->addChild(new LabelNode("resolvedPosOriOffset"));
    graph_->addChild(resolvedPosOriOffsetNode_.get());

    update_();
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
      bool rootCheck = rootCheck_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Root", &rootCheck_);
      if (rootCheck != rootCheck_)
        needUpdate = true;

      bool posOffsetCheck = posOffsetCheck_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Pos Offset", &posOffsetCheck_);
      if (posOffsetCheck != posOffsetCheck_)
        needUpdate = true;

      bool oriOffsetCheck = oriOffsetCheck_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Ori Offset", &oriOffsetCheck_);
      if (oriOffsetCheck != oriOffsetCheck_)
        needUpdate = true;

      bool posOriOffsetCheck = posOriOffsetCheck_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Pos/Ori Offset", &posOriOffsetCheck_);
      if (posOriOffsetCheck != posOriOffsetCheck_)
        needUpdate = true;

      bool resolvedOriOffsetCheck = resolvedOriOffsetCheck_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Ori Offset (resolved)", &resolvedOriOffsetCheck_);
      if (resolvedOriOffsetCheck != resolvedOriOffsetCheck_)
        needUpdate = true;

      bool resolvedPosOriOffsetCheck = resolvedPosOriOffsetCheck_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Pos/Ori Offset (resolved)", &resolvedPosOriOffsetCheck_);
      if (resolvedPosOriOffsetCheck != resolvedPosOriOffsetCheck_)
        needUpdate = true;

      ImGui::NextColumn(); ImGui::Separator(); ImGui::NextColumn();

      float lat = lat_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Latitude", &lat_, -90.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##lat")) { lat_ = 0.f; needUpdate = true; }
      if (lat != lat_)
        needUpdate = true;

      float lon = lon_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Longitude", &lon_, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##lon")) { lon_ = 0.f; needUpdate = true; }
      if (lon != lon_)
        needUpdate = true;

      float alt = alt_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Altitude", &alt_, 0.f, 500000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##alt")) { alt_ = 0.f; needUpdate = true; }
      if (alt != alt_)
        needUpdate = true;

      ImGui::NextColumn(); ImGui::Separator(); ImGui::NextColumn();

      float yaw = yaw_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Yaw", &yaw_, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##yaw")) { yaw_ = 0.f; needUpdate = true; }
      if (yaw != yaw_)
        needUpdate = true;

      float pitch = pitch_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Pitch", &pitch_, -90.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##pitch")) { pitch_ = 0.f; needUpdate = true; }
      if (pitch != pitch_)
        needUpdate = true;

      float roll = roll_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Roll", &roll_, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##roll")) { roll_ = 0.f; needUpdate = true; }
      if (roll != roll_)
        needUpdate = true;

      ImGui::NextColumn(); ImGui::Separator(); ImGui::NextColumn();

      float xOffset = xOffset_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "X Offset", &xOffset_, -500000.f, 500000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##xOff")) { xOffset_ = 0.f; needUpdate = true; }
      if (xOffset != xOffset_)
        needUpdate = true;

      float yOffset = yOffset_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Y Offset", &yOffset_, -500000.f, 500000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##yOff")) { yOffset_ = 0.f; needUpdate = true; }
      if (yOffset != yOffset_)
        needUpdate = true;

      float zOffset = zOffset_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Z Offset", &zOffset_, -500000.f, 500000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##zOff")) { zOffset_ = 0.f; needUpdate = true; }
      if (zOffset != zOffset_)
        needUpdate = true;

      ImGui::NextColumn(); ImGui::Separator(); ImGui::NextColumn();

      float yawOffset = yawOffset_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Yaw Offset", &yawOffset_, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##yawOff")) { yawOffset_ = 0.f; needUpdate = true; }
      if (yawOffset != yawOffset_)
        needUpdate = true;

      float pitchOffset = pitchOffset_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Pitch Offset", &pitchOffset_, -90.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##pitchOff")) { pitchOffset_ = 0.f; needUpdate = true; }
      if (pitchOffset != pitchOffset_)
        needUpdate = true;

      float rollOffset = rollOffset_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Roll Offset", &rollOffset_, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##rollOff")) { rollOffset_ = 0.f; needUpdate = true; }
      if (rollOffset != rollOffset_)
        needUpdate = true;

      ImGui::NextColumn(); ImGui::Separator(); ImGui::NextColumn();

      float xOffset2 = xOffset2_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "X Offset (rsv)", &xOffset2_, -500000.f, 500000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##xOffRsv")) { xOffset2_ = 0.f; needUpdate = true; }
      if (xOffset2 != xOffset2_)
        needUpdate = true;

      float yOffset2 = yOffset2_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Y Offset (rsv)", &yOffset2_, -500000.f, 500000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##yOffRsv")) { yOffset2_ = 0.f; needUpdate = true; }
      if (yOffset2 != yOffset2_)
        needUpdate = true;

      float zOffset2 = zOffset2_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Z Offset (rsv)", &zOffset2_, -500000.f, 500000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##zOffRsv")) { zOffset2_ = 0.f; needUpdate = true; }
      if (zOffset2 != zOffset2_)
        needUpdate = true;

      ImGui::NextColumn(); ImGui::Separator(); ImGui::NextColumn();

      float yawOffset2 = yawOffset2_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Yaw Offset (rsv)", &yawOffset2_, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##yawOffRsv")) { yawOffset2_ = 0.f; needUpdate = true; }
      if (yawOffset2 != yawOffset2_)
        needUpdate = true;

      float pitchOffset2 = pitchOffset2_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Pitch Offset (rsv)", &pitchOffset2_, -90.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##pitchOffRsv")) { pitchOffset2_ = 0.f; needUpdate = true; }
      if (pitchOffset2 != pitchOffset2_)
        needUpdate = true;

      float rollOffset2 = rollOffset2_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Roll Offset (rsv)", &rollOffset2_, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine(); if (ImGui::Button("Reset##rollOffRsv")) { rollOffset2_ = 0.f; needUpdate = true; }
      if (rollOffset2 != rollOffset2_)
        needUpdate = true;

      if (needUpdate)
        update_();

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  void update_()
  {
    root_->setCoordinate(simCore::Coordinate(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD * lat_, simCore::DEG2RAD * lon_, alt_),
      simCore::Vec3(simCore::DEG2RAD * yaw_, simCore::DEG2RAD * pitch_, simCore::DEG2RAD * roll_)), 0.0);

    posOffset_->setLocalOffsets(
      simCore::Vec3(xOffset_, yOffset_, zOffset_),
      simCore::Vec3());

    oriOffset_->setLocalOffsets(
      simCore::Vec3(),
      simCore::Vec3(simCore::DEG2RAD * yawOffset_, simCore::DEG2RAD * pitchOffset_, simCore::DEG2RAD * rollOffset_));

    posOriOffset_->setLocalOffsets(
      simCore::Vec3(xOffset_, yOffset_, zOffset_),
      simCore::Vec3(simCore::DEG2RAD * yawOffset_, simCore::DEG2RAD * pitchOffset_, simCore::DEG2RAD * rollOffset_));

    resolvedOriOffset_->setLocalOffsets(
      simCore::Vec3(),
      simCore::Vec3(simCore::DEG2RAD * yawOffset2_, simCore::DEG2RAD * pitchOffset2_, simCore::DEG2RAD * rollOffset2_));

    resolvedPosOriOffset_->setLocalOffsets(
      simCore::Vec3(xOffset2_, yOffset2_, zOffset2_),
      simCore::Vec3(simCore::DEG2RAD * yawOffset2_, simCore::DEG2RAD * pitchOffset2_, simCore::DEG2RAD * rollOffset2_));

    rootNode_->setNodeMask(rootCheck_ ? ~0 : 0);
    posOffsetNode_->setNodeMask(posOffsetCheck_ ? ~0 : 0);
    posOriOffsetNode_->setNodeMask(posOriOffsetCheck_ ? ~0 : 0);
    oriOffsetNode_->setNodeMask(oriOffsetCheck_ ? ~0 : 0);
    resolvedOriOffsetNode_->setNodeMask(resolvedOriOffsetCheck_ ? ~0 : 0);
    resolvedPosOriOffsetNode_->setNodeMask(resolvedPosOriOffsetCheck_ ? ~0 : 0);
  }

  void addResetButton_(float& value)
  {
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
      value = 0.f;
      update_();
    }
  }

  osg::ref_ptr<simVis::Locator> root_;
  osg::ref_ptr<simVis::LocatorNode> rootNode_;
  bool rootCheck_ = true;

  osg::ref_ptr<simVis::Locator> posOffset_;
  osg::ref_ptr<simVis::LocatorNode> posOffsetNode_;
  bool posOffsetCheck_ = false;

  osg::ref_ptr<simVis::Locator> posOriOffset_;
  osg::ref_ptr<simVis::LocatorNode> posOriOffsetNode_;
  bool posOriOffsetCheck_ = false;

  osg::ref_ptr<simVis::Locator> oriOffset_;
  osg::ref_ptr<simVis::LocatorNode> oriOffsetNode_;
  bool oriOffsetCheck_ = false;

  osg::ref_ptr<simVis::Locator> resolvedOriOffset_;
  osg::ref_ptr<simVis::LocatorNode> resolvedOriOffsetNode_;
  bool resolvedOriOffsetCheck_ = false;

  osg::ref_ptr<simVis::Locator> resolvedPosOriOffset_;
  osg::ref_ptr<simVis::LocatorNode> resolvedPosOriOffsetNode_;
  bool resolvedPosOriOffsetCheck_ = false;

  float lat_ = 0.f;
  float lon_ = 0.f;
  float alt_ = 0.f;

  float xOffset_ = 0.f;
  float yOffset_ = 0.f;
  float zOffset_ = 0.f;

  float yaw_ = 0.f;
  float pitch_ = 0.f;
  float roll_ = 0.f;

  float yawOffset_ = 0.f;
  float pitchOffset_ = 0.f;
  float rollOffset_ = 0.f;

  float xOffset2_ = 0.f;
  float yOffset2_ = 0.f;
  float zOffset2_ = 0.f;

  float yawOffset2_ = 0.f;
  float pitchOffset2_ = 0.f;
  float rollOffset2_ = 0.f;

  const SpatialReference* mapSRS_;
  osg::ref_ptr<osg::Group> graph_;
};

#endif

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
  simExamples::addDefaultSkyNode(viewer.get());

  osg::ref_ptr<osg::Group> graph = new osg::Group;
  const auto* mapSRS = viewer->getSceneManager()->getMap()->getSRS();

#ifdef HAVE_IMGUI
  ::GUI::OsgImGuiHandler* gui = new ::GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(mapSRS, graph.get()));
#endif

  viewer->getSceneManager()->getScenario()->addChild(graph);
  viewer->getMainView()->setViewpoint(Viewpoint(
    "Start", 0, 0, 0, -45.0, -45.0, 5e6));

  return viewer->run();
}

