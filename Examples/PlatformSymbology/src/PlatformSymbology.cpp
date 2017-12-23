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
/* -*-c++-*- */
/**
 * Platform Symbology Example
 *
 * Demonstrates basic platform creation and how to adjust platform, beam, and gate preferences.
 * Also useful as a combined test for platform/beam/gate rendering.
 */
#include <iomanip>
#include <sstream>

#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"

/// the simulator provides time/space data for our platform
#include "simUtil/PlatformSimulator.h"
#include "simData/MemoryDataStore.h"

/// include definitions for objects of interest
#include "simVis/Beam.h"
#include "simVis/Gate.h"
#include "simVis/LocalGrid.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/RocketBurn.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"

/// some basic components (mouse hover popups, scenario, utilities, camera controls)
#include "simVis/Popup.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"

#include "osgEarth/StringUtils"
#include "osgEarthSymbology/Style"
#include "osgEarthUtil/LatLongFormatter"
#include "osgEarthUtil/MGRSFormatter"

#include "google/protobuf/stubs/common.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;

namespace
{
  std::string SAYBOOL(bool x)
  {
    return x ? "ON" : "OFF";
  }
}

//----------------------------------------------------------------------------
/// create an overlay with some helpful information

/// first line, describe the program
static const std::string s_title = "Symbology Example";

/// later lines, document the keyboard controls
static const std::string s_help =
  " 0 : camera: toggle tethering to platform\n"
  " 1 : grid: cycle draw type\n"
  "\n"
  " 2 : model: change scale\n"
  " 3 : model: toggle auto-scale\n"
  " i : model: toggle model/icon\n"
  " j : model: cycle rotate-icon mode\n"
  " w : model: toggle override color\n"
  " h : model: toggle highlight\n"
  " p : model: cycle highlight color\n"
  "\n"
  " k : label: toggle text\n"
  " l : label: toggle text color\n"
  " m : label: toggle text outline\n"
  " n : label: toggle text outline color\n"
  " o : label: change text size\n"
  " r : label: change text backdrop type\n"
  " t : label: change text backdrop implementation\n"
  "\n"
  " 6 : beam: toggle lighting\n"
  " 7 : beam: toggle alpha blending\n"
  " 8 : beam: cycle draw modes\n"
  " a : beam: change cap resolution\n"
  " b : beam: change cone resolution\n"
  " c : beam: toggle scale (1, 2)\n"
  " d : beam: toggle auto-offset\n"
  "\n"
  " e : gate: toggle centroid display\n"
  " g : gate: cycle fill pattern\n"
  "\n"
  " x : laser: toggle display\n"
  " z : laser: toggle color\n"
  "\n"
  " O : toggle overhead mode\n"
  " C : toggle overhead clamping\n";

/// keep a handle, for toggling
static osg::ref_ptr<Control>      s_helpControl;
static osg::ref_ptr<LabelControl> s_action;

static Control* createHelp()
{
  VBox* vbox = new VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.4);
  vbox->addControl(new LabelControl(s_title, 20, osg::Vec4f(1, 1, 0, 1)));
  vbox->addControl(new LabelControl(s_help, 14, osg::Vec4f(.8, .8, .8, 1)));
  s_action = new LabelControl("", 16, osg::Vec4f(0, 1, 0, 1));
  vbox->addControl(s_action.get());
  s_helpControl = vbox;
  return vbox;
}

//----------------------------------------------------------------------------
/// event handler for keyboard commands to alter symbology at runtime
struct MenuHandler : public osgGA::GUIEventHandler
{
  /// constructor grabs all the state it needs for updating
  MenuHandler(
    simVis::ScenarioManager *scenario,
    simVis::View            *view,
    simData::DataStore      *dataStore,
    simData::ObjectId        platformId,
    simData::ObjectId        beamId,
    simData::ObjectId        gateId,
    simData::ObjectId        laserId
  )
  : scenario_(scenario),
    view_(view),
    tetherNode_(NULL),
    labelPos_(0),
    beamMode_(0),
    platformId_(platformId),
    beamId_(beamId),
    gateId_(gateId),
    laserId_(laserId),
    dataStore_(dataStore)
  {
  }

  /// Cycles colors through red, green, blue, and yellow
  unsigned cycleColorRgba(unsigned inColor) const
  {
    return inColor == 0xff0000ff ? 0x00ff00ff :  // Red to green
           inColor == 0x00ff00ff ? 0x0000ffff :  // Green to blue
           inColor == 0x0000ffff ? 0xffff00ff :  // Blue to yellow
           inColor == 0xffff00ff ? 0xffffffff :  // Yellow to white
                                   0xff0000ff;   // White to red
  }

  /// callback to process user input
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    /// only handle key down
    if (ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN)
      return false;

    bool handled = false;

    switch (ea.getKey())
    {
      case '!': // rotate|pan
        view_->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
        s_action->setText("Switched to NAVMODE_ROTATEPAN");
        handled = true;
        break;

      case '@': // globe spin
        view_->setNavigationMode(simVis::NAVMODE_GLOBESPIN);
        s_action->setText("Switched to NAVMODE_GLOBESPIN");
        handled = true;
        break;

      case '#': // zoom in|out
        view_->setNavigationMode(simVis::NAVMODE_ZOOM);
        s_action->setText("Switched to NAVMODE_ZOOM");
        handled = true;
        break;

      case '$': // center view
        view_->setNavigationMode(simVis::NAVMODE_CENTERVIEW);
        s_action->setText("Switched to NAVMODE_CENTERVIEW");
        handled = true;
        break;

      case '%': // center box zoom
        view_->setNavigationMode(simVis::NAVMODE_CENTERBOXZOOM);
        s_action->setText("Switched to NAVMODE_CENTERBOXZOOM");
        handled = true;
        break;

      case '^': // box zoom
        view_->setNavigationMode(simVis::NAVMODE_BOXZOOM);
        s_action->setText("Switched to NAVMODE_BOXZOOM");
        handled = true;
        break;

      case '1': // cycle through the local grid types
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs*  prefs     = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        simData::LocalGridPrefs* localgrid = prefs->mutable_commonprefs()->mutable_localgrid();

        localgrid->set_drawgrid(true);

        localgrid->set_gridtype(
          localgrid->gridtype() == simData::LocalGridPrefs_Type_CARTESIAN   ? simData::LocalGridPrefs_Type_POLAR :
          localgrid->gridtype() == simData::LocalGridPrefs_Type_POLAR       ? simData::LocalGridPrefs_Type_RANGE_RINGS :
          localgrid->gridtype() == simData::LocalGridPrefs_Type_RANGE_RINGS ? simData::LocalGridPrefs_Type_SPEED_RINGS :
          simData::LocalGridPrefs_Type_CARTESIAN);

        localgrid->set_followyaw(true);
        localgrid->set_followpitch(false);
        localgrid->set_followroll(true);

        s_action->setText(
          localgrid->gridtype() == simData::LocalGridPrefs_Type_CARTESIAN   ? "CARTESIAN" :
          localgrid->gridtype() == simData::LocalGridPrefs_Type_POLAR       ? "POLAR" :
          localgrid->gridtype() == simData::LocalGridPrefs_Type_RANGE_RINGS ? "RANGE RINGS" :
                                                                              "SPEED RINGS");

        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case '2': // toggle the scale factor on the model
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        double scale = prefs->scale() * 5;
        if (scale > 5000.0) scale = 1.0;
        prefs->set_scale(scale);
        s_action->setText(Stringify() << "Set scale factor to " << scale);
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case '3': // toggle dynamic scaling
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        prefs->set_dynamicscale(!prefs->dynamicscale());
        s_action->setText(Stringify() << "Set dynamic scale to " << SAYBOOL(prefs->dynamicscale()));
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case '6': // toggle lighting on the beam
      {
        simData::DataStore::Transaction xaction;
        simData::BeamPrefs* prefs = dataStore_->mutable_beamPrefs(beamId_, &xaction);
        prefs->set_shaded(!prefs->shaded());
        s_action->setText(Stringify() << "Set beam lighting to " << SAYBOOL(prefs->shaded()));
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case '7': // toggle blending on the beam
      {
        simData::DataStore::Transaction xaction;
        simData::BeamPrefs* prefs = dataStore_->mutable_beamPrefs(beamId_, &xaction);
        prefs->set_blended(!prefs->blended());
        s_action->setText(Stringify() << "Set beam blending to " << SAYBOOL(prefs->blended()));
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case '8': // cycle beam draw mode (solid, wire-on-solid, wire)
      {
        simData::DataStore::Transaction xaction;
        simData::BeamPrefs* prefs = dataStore_->mutable_beamPrefs(beamId_, &xaction);

        const simData::BeamPrefs::DrawMode oldMode = prefs->beamdrawmode();
        prefs->set_beamdrawmode(
          oldMode == simData::BeamPrefs::WIRE  ? simData::BeamPrefs::SOLID :
          oldMode == simData::BeamPrefs::SOLID ? simData::BeamPrefs::WIRE_ON_SOLID :
                                                 simData::BeamPrefs::WIRE);

        s_action->setText(Stringify() << "Set beam draw mode to " << (
          prefs->beamdrawmode() == simData::BeamPrefs::WIRE  ? "WIRE" :
          prefs->beamdrawmode() == simData::BeamPrefs::SOLID ? "SOLID" :
                                                           "WIRE_ON_SOLID"));

        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'a': // cycle the beam's cap resolution
      {
        simData::DataStore::Transaction xaction;
        simData::BeamPrefs* prefs = dataStore_->mutable_beamPrefs(beamId_, &xaction);
        unsigned capRes = prefs->capresolution();
        //capRes = 1 + (capRes+1)%10;
        capRes = 1 + (capRes+1)%30;
        prefs->set_capresolution(capRes);
        s_action->setText(Stringify() << "Set beam cap resolution to " << capRes);
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'b': // cycle through the beam's cone resolution
      {
        simData::DataStore::Transaction xaction;
        simData::BeamPrefs* prefs = dataStore_->mutable_beamPrefs(beamId_, &xaction);
        unsigned coneRes = prefs->coneresolution();
        coneRes = 4 + ((coneRes-4)+1)%36;
        prefs->set_coneresolution(coneRes);
        s_action->setText(Stringify() << "Set beam cone resolution to " << coneRes);
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'c': // cycle beam scale:
      {
        simData::DataStore::Transaction xaction;
        simData::BeamPrefs* prefs = dataStore_->mutable_beamPrefs(beamId_, &xaction);
        float scale = prefs->beamscale() * 2.f;
        if (scale > 16.0f) scale = 1.0f;
        prefs->set_beamscale(scale);
        s_action->setText(Stringify() << "Set beam scale to " << scale);
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'd': // toggle beam auto-offset:
      {
        simData::DataStore::Transaction xaction;
        simData::BeamPrefs* prefs = dataStore_->mutable_beamPrefs(beamId_, &xaction);
        prefs->set_useoffseticon(!prefs->useoffseticon());
        s_action->setText(Stringify() << "Set beam auto-offset to " << SAYBOOL(prefs->useoffseticon()));
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'e': // toggle gate centroid
      {
        simData::DataStore::Transaction xaction;
        simData::GatePrefs* prefs = dataStore_->mutable_gatePrefs(gateId_, &xaction);
        prefs->set_drawcentroid(!prefs->drawcentroid());
        s_action->setText(Stringify() << "Set draw gate centroid to " << SAYBOOL(prefs->drawcentroid()));
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'g': // cycle gate fill pattern
      {
        simData::DataStore::Transaction xaction;
        simData::GatePrefs* prefs = dataStore_->mutable_gatePrefs(gateId_, &xaction);
        const simData::GatePrefs::FillPattern old = prefs->fillpattern();

        prefs->set_fillpattern(
          old == simData::GatePrefs::CENTROID ? simData::GatePrefs::STIPPLE :
          old == simData::GatePrefs::STIPPLE  ? simData::GatePrefs::ALPHA :
          old == simData::GatePrefs::ALPHA    ? simData::GatePrefs::WIRE :
          /*old == simData::GatePrefs::WIRE   ? */ simData::GatePrefs::CENTROID);

        s_action->setText(Stringify() << "Set gate fill pattern to " << (
          prefs->fillpattern() == simData::GatePrefs::CENTROID ? "CENTROID" :
          prefs->fillpattern() == simData::GatePrefs::STIPPLE  ? "STIPPLE"  :
          prefs->fillpattern() == simData::GatePrefs::ALPHA    ? "APLHA"    :
                                                                 "WIRE"));

        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'h': // toggle circle highlight
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        prefs->set_drawcirclehilight(!prefs->drawcirclehilight());
        if (prefs->drawcirclehilight())
          s_action->setText("Turned Highlight: On");
        else
          s_action->setText("Turned Highlight: Off");
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'p': // cycle circle highlight color
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        prefs->set_circlehilightcolor(cycleColorRgba(prefs->circlehilightcolor()));
        std::stringstream ss;
        ss << "Highlight RGBA: 0x" << std::hex << std::setfill('0') << std::setw(8) <<
          prefs->circlehilightcolor();
        s_action->setText(ss.str());
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'i': // toggle between a 3D model and an Icon
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        if (prefs->icon() == EXAMPLE_AIRPLANE_ICON)
        {
          prefs->set_icon(EXAMPLE_IMAGE_ICON);
          s_action->setText("Switched to image icon");
        }
        else
        {
          prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
          s_action->setText("Switched to 3D model");
        }
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'j': // toggle auto-rotate-to-screen for model
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        simData::IconRotation old = prefs->rotateicons();
        prefs->set_rotateicons(
          old == simData::IR_2D_UP    ? simData::IR_2D_YAW :
          old == simData::IR_2D_YAW   ? simData::IR_3D_YPR :
          old == simData::IR_3D_YPR   ? simData::IR_3D_NORTH :
          old == simData::IR_3D_NORTH ? simData::IR_3D_YAW :
          /*old == simData::IR_3D_YAW ? */  simData::IR_2D_UP);

        s_action->setText(Stringify() << "Set icon rotate mode to " << (
          prefs->rotateicons() == simData::IR_2D_UP    ? "2D_UP (Billboard Pointing Up)" :
          prefs->rotateicons() == simData::IR_2D_YAW   ? "2D_YAW (Billboard with Yaw)" :
          prefs->rotateicons() == simData::IR_3D_YPR   ? "3D_YPR (Follow Platform)"   :
          prefs->rotateicons() == simData::IR_3D_NORTH ? "3D_NORTH (Flat Oriented North)" :
                                                         "3D_YAW (Flat with Yaw)"));

        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'w': // toggle override color on the model.
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        bool use = prefs->commonprefs().useoverridecolor();
        prefs->mutable_commonprefs()->set_useoverridecolor(!use);
        prefs->mutable_commonprefs()->set_overridecolor(simVis::Color::Cyan.as(simVis::Color::RGBA));
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'k': // toggle label
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(!prefs->commonprefs().labelprefs().draw());
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'l': // toggle label color
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        unsigned color = prefs->commonprefs().labelprefs().color();
        color = (color == 0xFFFFFFFF ? 0xFF7f00FF : 0xFFFFFFFF);  // white/orange
        prefs->mutable_commonprefs()->mutable_labelprefs()->set_color(color);
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'm': // toggle label outline
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        prefs->mutable_commonprefs()->mutable_labelprefs()->set_textoutline(
          prefs->commonprefs().labelprefs().textoutline() == simData::TO_NONE ? simData::TO_THIN :
          prefs->commonprefs().labelprefs().textoutline() == simData::TO_THIN ? simData::TO_THICK :
          simData::TO_NONE);
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'n': // label outline color
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        prefs->mutable_commonprefs()->mutable_labelprefs()->set_outlinecolor(
          prefs->commonprefs().labelprefs().outlinecolor() == 0x000000FF ? 0xFF0000FF :
          prefs->commonprefs().labelprefs().outlinecolor() == 0xFF0000FF ? 0x00FF00FF :
          0x000000FF);
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'o': // label font size
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        unsigned size = prefs->commonprefs().labelprefs().overlayfontpointsize();
        size =
          size < 12 ? 12 :
          size < 16 ? 16 :
          size < 20 ? 20 :
          size < 24 ? 24 :
          10;
        prefs->mutable_commonprefs()->mutable_labelprefs()->set_overlayfontpointsize(size);
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 'r':
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        simData::BackdropType type = prefs->commonprefs().labelprefs().backdroptype();
        // NOTE: There's also a BDT_NONE value which turns off the backdrop.  We don't need
        // to use that and let label outline drive turning on and off the outline.
        if (type == simData::BDT_SHADOW_BOTTOM_RIGHT)
          type = simData::BDT_SHADOW_CENTER_RIGHT;
        else if (type == simData::BDT_SHADOW_CENTER_RIGHT)
          type = simData::BDT_SHADOW_TOP_RIGHT;
        else if (type == simData::BDT_SHADOW_TOP_RIGHT)
          type = simData::BDT_SHADOW_BOTTOM_CENTER;
        else if (type == simData::BDT_SHADOW_BOTTOM_CENTER)
          type = simData::BDT_SHADOW_TOP_CENTER;
        else if (type == simData::BDT_SHADOW_TOP_CENTER)
          type = simData::BDT_SHADOW_BOTTOM_LEFT;
        else if (type == simData::BDT_SHADOW_BOTTOM_LEFT)
          type = simData::BDT_SHADOW_CENTER_LEFT;
        else if (type == simData::BDT_SHADOW_CENTER_LEFT)
          type = simData::BDT_SHADOW_TOP_LEFT;
        else if (type == simData::BDT_SHADOW_TOP_LEFT)
          type = simData::BDT_OUTLINE;
        else if (type == simData::BDT_OUTLINE)
          type = simData::BDT_SHADOW_BOTTOM_RIGHT;

        s_action->setText(Stringify() << "Set backdrop type to\n" << (
          type == simData::BDT_SHADOW_BOTTOM_RIGHT ?  "SHADOW_BOTTOM_RIGHT" :
          type == simData::BDT_SHADOW_CENTER_RIGHT ?  "SHADOW_CENTER_RIGHT" :
          type == simData::BDT_SHADOW_TOP_RIGHT ?     "SHADOW_TOP_RIGHT" :
          type == simData::BDT_SHADOW_BOTTOM_CENTER ? "SHADOW_BOTTOM_CENTER" :
          type == simData::BDT_SHADOW_TOP_CENTER ?    "SHADOW_TOP_CENTER" :
          type == simData::BDT_SHADOW_BOTTOM_LEFT ?   "SHADOW_BOTTOM_LEFT" :
          type == simData::BDT_SHADOW_CENTER_LEFT ?   "SHADOW_CENTER_LEFT" :
          type == simData::BDT_SHADOW_TOP_LEFT ?      "SHADOW_TOP_LEFT" :
                                                      "OUTLINE"));

        prefs->mutable_commonprefs()->mutable_labelprefs()->set_backdroptype(type);
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case 's':
        // s is standard osg frame rate statistics display
        handled = false;
        break;
      case 't':
      {
        simData::DataStore::Transaction xaction;
        simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
        simData::BackdropImplementation implementation = prefs->commonprefs().labelprefs().backdropimplementation();
        if (implementation == simData::BDI_POLYGON_OFFSET)
          implementation = simData::BDI_NO_DEPTH_BUFFER;
        else if (implementation == simData::BDI_NO_DEPTH_BUFFER)
          implementation = simData::BDI_DEPTH_RANGE;
        else if (implementation == simData::BDI_DEPTH_RANGE)
          implementation = simData::BDI_STENCIL_BUFFER;
        else if (implementation == simData::BDI_STENCIL_BUFFER)
          implementation = simData::BDI_DELAYED_DEPTH_WRITES;
        else if (implementation == simData::BDI_DELAYED_DEPTH_WRITES)
          implementation = simData::BDI_POLYGON_OFFSET;

        s_action->setText(Stringify() << "Set backdrop implementation to\n" << (
          implementation == simData::BDI_POLYGON_OFFSET ?  "POLYGON_OFFSET" :
          implementation == simData::BDI_NO_DEPTH_BUFFER ? "NO_DEPTH_BUFFER" :
          implementation == simData::BDI_DEPTH_RANGE ?     "DEPTH_RANGE" :
          implementation == simData::BDI_STENCIL_BUFFER ?  "STENCIL_BUFFER" :
                                                           "DELAYED_DEPTH_WRITES"));

        prefs->mutable_commonprefs()->mutable_labelprefs()->set_backdropimplementation(implementation);
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case '0': // toggle tethering
      {
        if (view_->getCameraTether())
        {
          // save the current tether for restoration
          tetherNode_ = view_->getCameraTether();
          view_->tetherCamera(NULL);
          s_action->setText(Stringify() << "Tether OFF");
        }
        else
        {
          view_->tetherCamera(tetherNode_.get());
          s_action->setText(Stringify() << "Tether ON");
        }

        handled = true;
      }
      break;

      case 'x': // toggle laser display
      {
        simData::DataStore::Transaction xaction;
        simData::LaserPrefs* prefs = dataStore_->mutable_laserPrefs(laserId_, &xaction);
        prefs->mutable_commonprefs()->set_draw(!prefs->commonprefs().draw());
        s_action->setText(Stringify() << "Set laser to " << SAYBOOL(prefs->commonprefs().draw()));
        xaction.complete(&prefs);
      }
      break;

      case 'z': // toggle laser color
      {
        simData::DataStore::Transaction xaction;
        simData::LaserPrefs* prefs = dataStore_->mutable_laserPrefs(laserId_, &xaction);
        unsigned color = prefs->commonprefs().color();
        unsigned new_color = cycleColorRgba(color);
        prefs->mutable_commonprefs()->set_color(new_color);
        s_action->setText("Changed laser color");
        xaction.complete(&prefs);
        handled = true;
      }
      break;

      case '?' : // toggle help
      {
        s_helpControl->setVisible(!s_helpControl->visible());
        handled = true;
      }
      break;
    }

    return handled;
  }

protected: // data
  osg::ref_ptr<simVis::ScenarioManager> scenario_;
  osg::ref_ptr<simVis::View>     view_; /// view to update

  /// internal state
  osg::ref_ptr<osg::Node> tetherNode_;
  int32_t    labelPos_;   /// label position state machine
  int        beamMode_;

  simData::ObjectId platformId_, beamId_, gateId_, laserId_;

  simData::DataStore *dataStore_;
};

//----------------------------------------------------------------------------

/// create a platform and add it to 'dataStore'
///@return id for the new platform
simData::ObjectId addPlatform(simData::DataStore &dataStore)
{
  /// all DataStore operations require a transaction (to avoid races)
  simData::DataStore::Transaction transaction;

  /// create the platform, and get the properties for it
  simData::PlatformProperties *newProps = dataStore.addPlatform(&transaction);

  /// save the platform id for our return value
  simData::ObjectId result = newProps->id();

  /// done
  transaction.complete(&newProps);
  return result;
}

/// create a beam and add it to 'dataStore'
///@return id for new beam
simData::ObjectId addBeam(simData::ObjectId hostId, simData::DataStore &dataStore)
{
  simData::DataStore::Transaction transaction;

  simData::BeamProperties *beamProps = dataStore.addBeam(&transaction);
  simData::ObjectId result = beamProps->id();
  beamProps->set_hostid(hostId);
  transaction.complete(&beamProps);

  simData::BeamPrefs *beamPrefs = dataStore.mutable_beamPrefs(result, &transaction);
  beamPrefs->set_azimuthoffset(osg::DegreesToRadians(0.0));
  beamPrefs->set_verticalwidth(osg::DegreesToRadians(30.0));
  beamPrefs->set_horizontalwidth(osg::DegreesToRadians(60.0));
  transaction.complete(&beamPrefs);

  return result;
}

/// create a gate and add it to 'dataStore'
///@return id for new gate
simData::ObjectId addGate(simData::ObjectId hostId, simData::DataStore &dataStore)
{
  simData::DataStore::Transaction transaction;

  simData::GateProperties *gateProps = dataStore.addGate(&transaction);
  simData::ObjectId result = gateProps->id();
  gateProps->set_hostid(hostId);
  transaction.complete(&gateProps);

  simData::GatePrefs *gatePrefs = dataStore.mutable_gatePrefs(result, &transaction);
  gatePrefs->set_gateazimuthoffset(osg::DegreesToRadians(0.0));
  transaction.complete(&gatePrefs);

  return result;
}

/// create a laser and add it to 'dataStore'
///@return id for new gate
simData::ObjectId addLaser(simData::ObjectId hostId, simData::DataStore &dataStore)
{
  // add the laser:
  simData::ObjectId id;
  {
    simData::DataStore::Transaction xaction;
    simData::LaserProperties* laserProps = dataStore.addLaser(&xaction);
    id = laserProps->id();
    laserProps->set_hostid(hostId);
    xaction.complete(&laserProps);
  }

  // set the initial prefs:
  {
    simData::DataStore::Transaction xaction;
    simData::LaserPrefs* prefs = dataStore.mutable_laserPrefs(id, &xaction);
    prefs->mutable_commonprefs()->set_datadraw(true);
    prefs->mutable_commonprefs()->set_draw(true);
    prefs->mutable_commonprefs()->set_color(0xFF0000FF); // red
    prefs->set_maxrange(14500000.0);
    prefs->set_laserwidth(4);
    xaction.complete(&prefs);
  }

  // Add an update:
  {
    simData::DataStore::Transaction xaction;
    simData::LaserUpdate* update = dataStore.addLaserUpdate(id, &xaction);
    update->set_time(0.0);
    update->mutable_orientation()->set_yaw(0.0);
    update->mutable_orientation()->set_pitch(0.0);
    update->mutable_orientation()->set_roll(0.0);
    xaction.complete(&update);
  }

  return id;
}

/// Sets up default prefs for a platform
void configurePlatformPrefs(simData::ObjectId platformId, simData::DataStore* dataStore, const std::string& name)
{
  simData::DataStore::Transaction xaction;
  simData::PlatformPrefs* prefs = dataStore->mutable_platformPrefs(platformId, &xaction);

  prefs->mutable_commonprefs()->set_name(name);
  prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
  prefs->set_scale(3.0f);
  prefs->set_dynamicscale(true);
  prefs->set_circlehilightcolor(0xffffffff);

  prefs->mutable_commonprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_overlayfontpointsize(14);

  prefs->mutable_commonprefs()->mutable_localgrid()->mutable_speedring()->set_timeformat(simData::ELAPSED_SECONDS);
  prefs->mutable_commonprefs()->mutable_localgrid()->mutable_speedring()->set_radius(2);

  xaction.complete(&prefs);
}

/// connect beam to platform, set some properties
void configurePrefs(simData::ObjectId   platformId,
                    simData::ObjectId   beamId,
                    simData::ObjectId   gateId,
                    simData::DataStore* dataStore)
{
  /// configure the platform
  configurePlatformPrefs(platformId, dataStore, "Simulated Platform");

  /// set up the beam
  {
    simData::DataStore::Transaction xaction;
    simData::BeamPrefs* prefs = dataStore->mutable_beamPrefs(beamId, &xaction);
    prefs->set_beamdrawmode(simData::BeamPrefs::WIRE_ON_SOLID);
    xaction.complete(&prefs);
  }

  /// set up the gate
  {
    simData::DataStore::Transaction xaction;
    simData::GatePrefs* prefs = dataStore->mutable_gatePrefs(gateId, &xaction);
    //TODO: set some gate prefs here
    xaction.complete(&prefs);
  }
}

//----------------------------------------------------------------------------

/**
 * Custom "popup" callback - Demonstrates how to customize the text displayed
 * in the mouse-over popup box. Also shows the usage of the LatLongFormatter
 * and the MGRSFormatter.
 */
struct MyPopupCallback : public simVis::PopupContentCallback
{
  explicit MyPopupCallback(const SpatialReference* srs) : srs_(srs) { }

  std::string createString(simVis::PlatformNode* platform)
  {
    simCore::Vec3 lla;
    platform->getLocator()->getLocatorPosition(&lla, simCore::COORD_SYS_LLA);

    GeoPoint pos(
      srs_,
      lla.lon()*simCore::RAD2DEG, lla.lat()*simCore::RAD2DEG, lla.alt()*simCore::RAD2DEG,
      ALTMODE_ABSOLUTE);

    int altP = pos.z() < 200.0 ? 1 : 0;

    static LatLongFormatter llf;
    static MGRSFormatter    mgrs;

    return osgEarth::Stringify()
      << std::fixed
      << "Lat: " << llf.format(pos.y(), 2) << '\n'
      << "Lon: " << llf.format(pos.x(), 2) << '\n'
      << std::setprecision(altP) << "Alt: " << pos.z() << "m" << '\n'
      << "MGRS: " << mgrs.format(pos);
  }

  const SpatialReference* srs_;
};

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  simCore::checkVersionThrow();
  /// set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  /// creates a world map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  /// Simdis viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  /// add in the platform and beam
  simData::ObjectId platformId = addPlatform(dataStore);
  simData::ObjectId     beamId = addBeam(platformId, dataStore);
  simData::ObjectId     gateId = addGate(beamId, dataStore);
  simData::ObjectId    laserId = addLaser(platformId, dataStore);

  /// connect them and add some additional settings
  configurePrefs(platformId, beamId, gateId, &dataStore);

  {
    simVis::PlatformNode *platNode = scene->getScenario()->find<simVis::PlatformNode>(platformId);
    osg::Texture2D* rocketBurnTexture = new osg::Texture2D();
    rocketBurnTexture->setImage(osgDB::readImageFile(EXAMPLE_ROCKET_BURN_TEXTURE));
    simVis::RocketBurn *rb = new simVis::RocketBurn(*platNode, *rocketBurnTexture);
    simVis::RocketBurn::ShapeData rocketBurnShape;
    rocketBurnShape.radiusFar = 0.001;
    rocketBurnShape.radiusNear = 1;
    rocketBurnShape.length = 10;
    rocketBurnShape.scaleAlpha = true;
    rb->update(rocketBurnShape);
    // move to back of host
    rb->setPositionOrientation(simCore::Vec3(0, -platNode->getActualSize().yMax(), 0), simCore::Vec3());
  }

  /// simulator will compute time-based updates for our platform (and any beams it is hosting)
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(platformId);

  /// create some waypoints (lat, lon, alt, duration)
  //sim->addWaypoint( simUtil::Waypoint( 0, 0, 30000, 200 ) );
  //sim->addWaypoint( simUtil::Waypoint( 0, -90, 30000, 200 ) );
  sim->addWaypoint(simUtil::Waypoint(51.5,   0.0, 30000, 200.0)); // London
  sim->addWaypoint(simUtil::Waypoint(38.8, -77.0, 30000, 200.0)); // DC
  sim->addWaypoint(simUtil::Waypoint(-33.4, -70.8, 30000, 200.0)); // Santiago
  sim->addWaypoint(simUtil::Waypoint(-34.0,  18.5, 30000, 200.0)); // Capetown

  sim->setSimulateRoll(true);
  sim->setSimulatePitch(false);

  /// Install frame update handler that will update track positions over time.
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);

#if 1
  /// Create a second platform to fly alongside the first
  simData::ObjectId platform2Id = addPlatform(dataStore);
  configurePlatformPrefs(platform2Id, &dataStore, "Secondary Platform");
  osg::ref_ptr<simUtil::PlatformSimulator> sim2 = new simUtil::PlatformSimulator(platform2Id);
  sim2->addWaypoint(simUtil::Waypoint(51.5,   0.0, 29990, 200.0)); // London
  sim2->addWaypoint(simUtil::Waypoint(38.8, -77.0, 29990, 200.0)); // DC
  sim2->setSimulateRoll(true);
  sim2->setSimulatePitch(false);
  simMgr->addSimulator(sim2.get());
#endif

  /// Start the simulation
  simMgr->addSimulator(sim.get());
  simMgr->simulate(0.0, 120.0, 60.0);

  /// Attach the simulation updater to OSG timer events
  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simMgr.get(), 0.0, 120.0);
  viewer->addEventHandler(simHandler.get());

  /// Tether camera to platform
  osg::ref_ptr<simVis::PlatformNode> platformNode = scene->getScenario()->find<simVis::PlatformNode>(platformId);
  viewer->getMainView()->tetherCamera(platformNode.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(0, -45, 4e5);

  /// handle key press events
  viewer->addEventHandler(
    new MenuHandler(
      scene->getScenario(),
      viewer->getMainView(),
      &dataStore,
      platformId,
      beamId,
      gateId,
      laserId));

  /// hovering the mouse over the platform should trigger a popup
  osg::ref_ptr<simVis::PopupHandler> popupHandler = new simVis::PopupHandler(scene.get());
  popupHandler->setContentCallback(new MyPopupCallback(map->getProfile()->getSRS()));
  viewer->addEventHandler(popupHandler.get());

  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createHelp());

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  /// overhead mode toggler.
  viewer->addEventHandler(new simVis::ToggleOverheadMode(viewer->getMainView(), 'O', 'C'));

  return viewer->run();
}

