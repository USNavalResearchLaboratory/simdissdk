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

// NOTE: Using Triton may require an installation of the DirectX End-User Runtime Web Installer:
//   https://www.microsoft.com/en-US/Download/details.aspx?id=35
// Triton depends on an older DirectX implementation that may not come preinstalled on all machines.
// If you try to use Triton and get an error about d3dx9_43.dll missing, you need this download.

/**
 * OCEAN EXAMPLE - SIMDIS SDK
 *
 * Loads a terrain altitude set and enables the osgEarth Ocean processing,
 * demonstrating the sea surface features.
 *
 * NOTE:  An Internet connection is required for this example.
 */
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simData/MemoryDataStore.h"

#include "simVis/BathymetryGenerator.h"
#include "simVis/osgEarthVersion.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simVis/Constants.h"
#include "simVis/OverheadMode.h"
#include "simUtil/ExampleResources.h"

#include "osg/Depth"

#include "osgEarth/SimpleOceanLayer"
#include "osgEarth/MBTiles"
#include "osgEarth/Sky"
#include "osgEarth/TMS"
#include "osgEarth/Version"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif

#ifdef HAVE_OSGEARTH_TRITON
#include "osgEarthTriton/TritonLayer"
#include "simUtil/TritonSettings.h"
#endif

#ifdef HAVE_OSGEARTH_SILVERLINING
#include "osgEarthSilverLining/SilverLiningNode"
#include "simUtil/SilverLiningSettings.h"
#endif

// Hawaii near Kauai:
#define LAT             21.937611
#define LON           -159.793521
#define ALT              0.0
#define PLATFORM_SHIP "Ship"

using namespace osgEarth::Util;

#if OSGEARTH_VERSION_LESS_THAN(3,0,0)
using namespace osgEarth::Drivers;
namespace osgEarth { namespace Triton { class TritonLayer; } }
#endif

static simCore::Coordinate s_shipPosOri(simCore::COORD_SYS_LLA,
                                        simCore::Vec3(simCore::DEG2RAD*(LAT), simCore::DEG2RAD*(LON), ALT),
                                        simCore::Vec3(0.0, 0.0, 0.0));

static simData::ObjectId     s_shipId;


#ifdef HAVE_OSGEARTH_TRITON
// cull callback that adds buoyancy to a platform
// using its offset transform - this is not really
// appropriate in the long run since the offset xform
// is used for other things, but it works for a quick demo
class PlatformBuoyancyCallback : public osg::NodeCallback
{
public:
  explicit PlatformBuoyancyCallback(osgEarth::Triton::TritonLayer* triton) :
    triton_(triton),
    enabled_(false),
    reset_(false)
  {
    srs_ = osgEarth::SpatialReference::get("wgs84");
    isect_ = new osgEarth::Triton::TritonIntersections();
    isect_->addLocalPoint(osg::Vec3d(0, 0, 0));
    if (triton_.valid())
      triton_->addIntersections(isect_.get());
  }

  void setEnabled(bool enable)
  {
    if (enabled_ && !enable)
    {
      reset_ = true;
    }
    enabled_ = enable;
  }

  bool enabled() const
  {
    return enabled_;
  }

  virtual ~PlatformBuoyancyCallback()
  {
    // removeIntersections is not currently exposed in the Triton Layer API
    //osg::ref_ptr<osgEarth::Triton::TritonLayer> triton;
    //if (triton_.lock(triton))
    //  triton_->removeIntersections(isect_.get());
  }

  void operator()(osg::Node* node, osg::NodeVisitor* nv)
  {
    if (enabled_ || reset_)
    {
      simVis::PlatformNode* platform = dynamic_cast<simVis::PlatformNode*>(node);
      if (platform)
      {
        osg::MatrixTransform* xform = dynamic_cast<osg::MatrixTransform*>(platform->getModel()->offsetNode());
        if (reset_)
        {
          xform->setMatrix(osg::Matrix::identity());
          reset_ = false;
        }
        else
        {
          simCore::Vec3 pos;
          platform->getPosition(&pos, simCore::COORD_SYS_LLA);
          osgEarth::GeoPoint anchor(srs_.get(), simCore::RAD2DEG*pos.lon(), simCore::RAD2DEG*pos.lat(), 0);
          isect_->setAnchor(anchor);

          xform->setMatrix(
            osg::Matrix::translate(0, 0, isect_->getHeights()[0]) *
            osg::Matrix::rotate(osg::Vec3d(0,0,1), isect_->getNormals()[0]) );
        }
      }
    }

    traverse(node, nv);
  }

  osg::ref_ptr<const osgEarth::SpatialReference> srs_;
  osg::ref_ptr<osgEarth::Triton::TritonIntersections> isect_;
  osg::observer_ptr<osgEarth::Triton::TritonLayer> triton_;
  bool enabled_;
  bool reset_;
};
#else
// Provide a typedef for the callback since Triton isn't available
typedef osg::NodeCallback PlatformBuoyancyCallback;
#endif

// An event handler to assist in testing Ocean
struct MenuHandler : public osgGA::GUIEventHandler
{
  osg::observer_ptr<simVis::Viewer> viewer_;
  osg::observer_ptr<simVis::SceneManager> scene_;
  MenuHandler(simVis::Viewer* viewer, simVis::SceneManager* scene)
    : viewer_(viewer),
    scene_(scene)
  { }

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    bool handled = false;

    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
      case '0':
      {
        osg::observer_ptr<simVis::PlatformNode> ship = scene_.valid() ? scene_->getScenario()->find<simVis::PlatformNode>(s_shipId) : nullptr;
        if (ship != nullptr)
        {
          if (viewer_.valid())
          {
            viewer_->getMainView()->tetherCamera(ship.get());
            viewer_->getMainView()->setFocalOffsets(0, -10.0, 20000.0, 2.5);
          }
        }
        else
        {
          SIM_WARN << "Cannot find the Ship's platform" << std::endl;
        }
        break;
      }
      case '1':
        if (viewer_.valid())
          viewer_->getMainView()->tetherCamera(nullptr);
        break;
      case 'a':
        if (viewer_.valid())
          viewer_->setLogarithmicDepthBufferEnabled(!viewer_->isLogarithmicDepthBufferEnabled());
        break;
      }
    }
    return handled;
  }
};

static simData::ObjectId createShip(simData::DataStore& dataStore)
{
  simData::ObjectId result;

  // create the platform in the database:
  simData::DataStore::Transaction transaction;
  {
    simData::PlatformProperties* props = dataStore.addPlatform(&transaction);
    result = props->id();
    transaction.complete(&props);
  }

  // initialize the prefs:
  {
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(result, &transaction);
    prefs->mutable_commonprefs()->set_name(PLATFORM_SHIP);
    prefs->set_icon(EXAMPLE_SHIP_ICON);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    transaction.complete(&prefs);
  }

  // give it a starting position:
  {
    // convert to ECEF:
    simCore::Coordinate ecef;
    simCore::CoordinateConverter::convertGeodeticToEcef(s_shipPosOri, ecef);

    simData::PlatformUpdate* newUpdate = dataStore.addPlatformUpdate(result, &transaction);
    newUpdate->set_x(ecef.x());
    newUpdate->set_y(ecef.y());
    newUpdate->set_z(ecef.z());
    newUpdate->set_psi(ecef.psi());
    newUpdate->set_theta(ecef.theta());
    newUpdate->set_phi(ecef.phi());
    newUpdate->set_time(0.0);
    transaction.complete(&newUpdate);
  }

  dataStore.update(0.0);

  return result;
}

#ifdef HAVE_OSGEARTH_TRITON
static osg::ref_ptr<simUtil::TritonSettingsAdapter> s_TritonSettings(new simUtil::TritonSettingsAdapter);
#endif /* HAVE_OSGEARTH_TRITON */

namespace
{

#ifdef HAVE_OSGEARTH_SILVERLINING
  osg::ref_ptr<simUtil::SilverLiningSettingsAdapter> s_SlSettings = new simUtil::SilverLiningSettingsAdapter;

  /** Adds and removes clouds in the SL Callback code */
  class CloudManager : public simUtil::SilverLiningValue
  {
  public:
    /** Remove all clouds on next round */
    void clearClouds()
    {
      clearClouds_ = true;
      setShouldApply_();
    }
    /** Add a new cloud type on next round */
    void addCloudType(SilverLining::CloudTypes cloudType)
    {
      addCloud_ = cloudType;
      setShouldApply_();
    }

  protected:
    virtual ~CloudManager() {}

    /** Clears out then applies clouds as needed */
    virtual void apply_(osgEarth::SilverLining::Atmosphere& atmosphere)
    {
      if (clearClouds_.isSet())
      {
        atmosphere.GetConditions().RemoveAllCloudLayers();
        clearClouds_.unset();
      }
      if (addCloud_.isSet())
      {
        addCloudLayer_(atmosphere, addCloud_.get());
        addCloud_.unset();
      }
    }

  private:
    /** Adds a reasonably initialized cloud layer */
    void addCloudLayer_(SilverLining::Atmosphere& atmosphere, SilverLining::CloudTypes cloudType)
    {
#if OSGEARTH_VERSION_GREATER_OR_EQUAL(3,1,0)
      SilverLining::CloudLayer cloudLayer = SilverLining::CloudLayerFactory::Create(cloudType, atmosphere);
#else
      SilverLining::CloudLayer cloudLayer = SilverLining::CloudLayerFactory::Create(cloudType);
#endif
      cloudLayer.SetIsInfinite(true);
      cloudLayer.SetThickness(50);
      switch (cloudType)
      {
      case SilverLining::CUMULONIMBUS_CAPPILATUS:
        cloudLayer.SetBaseWidth(5000);
        cloudLayer.SetBaseLength(5000);
        break;
      default:
        cloudLayer.SetBaseWidth(100000);
        cloudLayer.SetBaseLength(100000);
        break;
      }
      switch (cloudType)
      {
      case SilverLining::CUMULUS_CONGESTUS:
      case SilverLining::CUMULUS_CONGESTUS_HI_RES:
      case SilverLining::CUMULUS_MEDIOCRIS:
        cloudLayer.SetBaseAltitude(2500);
        break;
      case SilverLining::CUMULONIMBUS_CAPPILATUS:
        cloudLayer.SetBaseAltitude(400);
        break;
      case SilverLining::STRATUS:
        cloudLayer.SetBaseAltitude(1250);
        cloudLayer.SetThickness(1500);
        break;
      case SilverLining::STRATOCUMULUS:
        cloudLayer.SetBaseAltitude(1250);
        cloudLayer.SetThickness(3000);
        break;
      case SilverLining::CIRRUS_FIBRATUS:
        cloudLayer.SetBaseAltitude(8000);
        break;
      case SilverLining::SANDSTORM:
        cloudLayer.SetBaseAltitude(0.0);
        cloudLayer.SetThickness(0);
        break;
      default:
        cloudLayer.SetBaseAltitude(3000);
        break;
      }
      cloudLayer.SetDensity(1.0);
      cloudLayer.SetLayerPosition(0, 0);
      cloudLayer.SetFadeTowardEdges(true);
      cloudLayer.SetAlpha(0.8);
      cloudLayer.SetCloudAnimationEffects(0.1, false, 0, 0);
      cloudLayer.SeedClouds(atmosphere);
      atmosphere.GetConditions().AddCloudLayer(cloudLayer);
    }

    optional<bool> clearClouds_;
    optional<SilverLining::CloudTypes> addCloud_;
  };
  osg::ref_ptr<CloudManager> s_CloudManager = new CloudManager;
#endif

#ifdef HAVE_IMGUI
  // ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
  // the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
  // while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(250); func("##" label, __VA_ARGS__)

  class ControlPanel : public simExamples::SimExamplesGui
  {
  public:
    ControlPanel(osgEarth::SimpleOceanLayer* simpleOceanLayer, osgEarth::VisibleLayer* tritonLayer,
      PlatformBuoyancyCallback* buoyancyCallback, SkyNode* skyNode, simVis::View* view,
      bool useTriton, bool useSilverLining)
      : simExamples::SimExamplesGui("Ocean Demo"),
      simpleOceanLayer_(simpleOceanLayer),
      tritonLayer_(tritonLayer),
      buoyancyCallback_(buoyancyCallback),
      skyNode_(skyNode),
      view_(view),
      useTriton_(useTriton),
      useSilverLining_(useSilverLining)
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

    ImGui::Text("0: reset view (to ship)");
    ImGui::Text("1: untether camera");

    if (ImGui::BeginTable("Table", 2))
    {
      // Opacity
      float opacity = opacity_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Opacity", &opacity_, 0.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
      if (opacity != opacity_)
      {
        if (simpleOceanLayer_.get())
          simpleOceanLayer_->setOpacity(opacity * 0.01f);
        else if (tritonLayer_.get())
          tritonLayer_->setOpacity(opacity * 0.01f);
      }

      // Lighting
      bool lighting = lighting_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Lighting", &lighting_);
      if (lighting != lighting_)
      {
        if (skyNode_.get())
          skyNode_->setLighting(osg::StateAttribute::OVERRIDE | (lighting_ ? osg::StateAttribute::ON : osg::StateAttribute::OFF));
      }

      // Overhead mode
      if (view_.valid())
      {
        bool overhead = view_->isOverheadEnabled();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Overhead Mode", &overhead);
        if (view_->isOverheadEnabled() != overhead)
          view_->enableOverheadMode(overhead);
      }

#ifdef HAVE_OSGEARTH_TRITON
      if (useTriton_)
      {
        ImGui::TableNextColumn();
        pushLargeFont_();
        ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Triton"); // Yellow
        popLargeFont_();
        ImGui::TableNextColumn();

        // Choppiness
        double choppiness = s_TritonSettings->choppiness()->value();
        const double minChop = 0.0;
        const double maxChop = 3.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Choppiness", ImGuiDataType_Double, &choppiness, &minChop, &maxChop, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (choppiness != s_TritonSettings->choppiness()->value())
          s_TritonSettings->choppiness()->set(choppiness);

        // Note: simUtil::TritonSettingsAdapter's quality setter is a no-op, so don't provide the user a control for it

        // Wind direction
        double direction = s_TritonSettings->seaState()->windDirection();
        const double minDir = -180.0;
        const double maxDir = 180.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Wind Direction", ImGuiDataType_Double, &direction, &minDir, &maxDir, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (direction != s_TritonSettings->seaState()->windDirection())
          s_TritonSettings->seaState()->setWindDirection(direction);

        // Sea state
        double seaState = s_TritonSettings->seaState()->seaState();
        const double minSeaState = 0.0;
        const double maxSeaState = 12.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Sea State", ImGuiDataType_Double, &seaState, &minSeaState, &maxSeaState, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (seaState != s_TritonSettings->seaState()->seaState())
          s_TritonSettings->seaState()->setSeaState(seaState);

        // Sun Intensity
        double sunIntensity = s_TritonSettings->sunIntensity()->value();
        const double minIntense = 0.0;
        const double maxIntense = 1.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Sun Intensity", ImGuiDataType_Double, &sunIntensity, &minIntense, &maxIntense, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (sunIntensity != s_TritonSettings->sunIntensity()->value())
          s_TritonSettings->sunIntensity()->set(sunIntensity);

        // Spray
        bool spray = s_TritonSettings->enableSpray()->value();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Spray", &spray);
        if (s_TritonSettings->enableSpray()->value() != spray)
          s_TritonSettings->enableSpray()->set(spray);

        // Wireframe
        bool wireframe = s_TritonSettings->enableWireframe()->value();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Wireframe", &wireframe);
        if (s_TritonSettings->enableWireframe()->value() != wireframe)
          s_TritonSettings->enableWireframe()->set(wireframe);

        // God rays
        bool godRays = s_TritonSettings->enableGodRays()->value();
        IMGUI_ADD_ROW(ImGui::Checkbox, "God Rays", &godRays);
        if (s_TritonSettings->enableGodRays()->value() != godRays)
          s_TritonSettings->enableGodRays()->set(godRays);

        // God rays fade
        double godRaysFade = s_TritonSettings->godRaysFade()->value();
        const double minFade = 0.0;
        const double maxFade = 1.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "God Rays Fade", ImGuiDataType_Double, &godRaysFade, &minFade, &maxFade, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (godRaysFade != s_TritonSettings->godRaysFade()->value())
          s_TritonSettings->godRaysFade()->set(godRaysFade);

        // Platform Buoyancy
        bool platformBouyancy = buoyancyCallback_->enabled();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Platform Buoyancy", &platformBouyancy);
        if (buoyancyCallback_->enabled() != platformBouyancy)
          buoyancyCallback_->setEnabled(platformBouyancy);
      }
#endif /* HAVE_OSGEARTH_TRITON */
#ifdef HAVE_OSGEARTH_SILVERLINING
      if (useSilverLining_)
      {
        ImGui::TableNextColumn();
        pushLargeFont_();
        ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "SilverLining"); // Yellow
        popLargeFont_();
        ImGui::TableNextColumn();

        // Visibility
        double visibility = s_SlSettings->visibility()->value();
        const double minVis = 100.0;
        const double maxVis = 100000.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Visibility", ImGuiDataType_Double, &visibility, &minVis, &maxVis, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (visibility != s_SlSettings->visibility()->value())
          s_SlSettings->visibility()->set(visibility);

        // Turbidity
        double turbidity = s_SlSettings->turbidity()->value();
        const double minTurbidity = 1.8;
        const double maxTurbidity = 8.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Turbidity", ImGuiDataType_Double,&turbidity, &minTurbidity, &maxTurbidity, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (turbidity != s_SlSettings->turbidity()->value())
          s_SlSettings->turbidity()->set(turbidity);

        // Light Pollution
        double lightPollution = s_SlSettings->lightPollution()->value();
        const double minPollution = 0.0;
        const double maxPollution = 0.01;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Light Pollution", ImGuiDataType_Double, &lightPollution, &minPollution, &maxPollution, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (lightPollution != static_cast<float>(s_SlSettings->lightPollution()->value()))
          s_SlSettings->lightPollution()->set(lightPollution);

        // Rain
        double rainRate = s_SlSettings->rainRate()->value();
        const double minRain = 0.0;
        const double maxRain = 30.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Rain", ImGuiDataType_Double, &rainRate, &minRain, &maxRain, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (rainRate != s_SlSettings->rainRate()->value())
          s_SlSettings->rainRate()->set(rainRate);

        // Snow
        double snowRate = s_SlSettings->snowRate()->rate();
        const double minSnow = 0.0;
        const double maxSnow = 30.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Snow", ImGuiDataType_Double, &snowRate, &minSnow, &maxSnow, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (snowRate != s_SlSettings->snowRate()->rate())
          s_SlSettings->snowRate()->setRate(snowRate);

        // Wet Snow
        bool wetSnow = s_SlSettings->snowRate()->isWet();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Wet Snow", &wetSnow);
        if (s_SlSettings->snowRate()->isWet() != wetSnow)
          s_SlSettings->snowRate()->setWet(wetSnow);

        // Sleet
        double sleetRate = s_SlSettings->sleetRate()->value();
        const double minSleet = 0.0;
        const double maxSleet = 30.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Sleet", ImGuiDataType_Double, &sleetRate, &minSleet, &maxSleet, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (sleetRate != static_cast<float>(s_SlSettings->sleetRate()->value()))
          s_SlSettings->sleetRate()->set(sleetRate);

        // Lens Flare
        bool lensFlare = s_SlSettings->lensFlare()->value();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Lens Flare", &lensFlare);
        if (s_SlSettings->lensFlare()->value() != lensFlare)
          s_SlSettings->lensFlare()->set(lensFlare);

        // Gamma
        double gamma = s_SlSettings->gamma()->value();
        const double minGamma = 0.0;
        const double maxGamma = 6.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Sleet", ImGuiDataType_Double, &gamma, &minGamma, &maxGamma, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (gamma != s_SlSettings->gamma()->value())
          s_SlSettings->gamma()->set(gamma);

        // Wind Speed
        double windSpeed = s_SlSettings->wind()->speed();
        const double minWindSpeed = 0.0;
        const double maxWindSpeed = 75.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Wind Speed", ImGuiDataType_Double, &windSpeed, &minWindSpeed, &maxWindSpeed, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (windSpeed != s_SlSettings->wind()->speed())
          s_SlSettings->wind()->setSpeed(windSpeed);

        // Wind Direction
        double windDirection = s_SlSettings->wind()->direction();
        const double minWindDirection = -180.0;
        const double maxWindDirection = 180.0;
        IMGUI_ADD_ROW(ImGui::SliderScalar, "Wind Direction", ImGuiDataType_Double, &windDirection, &minWindDirection, &maxWindDirection, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (windDirection != s_SlSettings->wind()->direction())
          s_SlSettings->wind()->setDirection(windDirection);

        // Infrared
        bool infrared = s_SlSettings->infrared()->value();
        IMGUI_ADD_ROW(ImGui::Checkbox, "Infrared", &infrared);
        if (s_SlSettings->infrared()->value() != infrared)
          s_SlSettings->infrared()->set(infrared);

        // Hosek-Wilkie
        osgEarth::SilverLining::Atmosphere::SkyModel skyModel = static_cast<osgEarth::SilverLining::Atmosphere::SkyModel>(s_SlSettings->skyModel()->value());
        bool hosekWilkie = (skyModel == osgEarth::SilverLining::Atmosphere::HOSEK_WILKIE);
        bool newValue = hosekWilkie;
        IMGUI_ADD_ROW(ImGui::Checkbox, "Hosek-Wilkie", &newValue);
        if (hosekWilkie != newValue)
          s_SlSettings->skyModel()->set(newValue ? osgEarth::SilverLining::Atmosphere::HOSEK_WILKIE : osgEarth::SilverLining::Atmosphere::PREETHAM);

        ImGui::TableNextColumn();
        ImGui::Text("Add Clouds");
        ImGui::TableNextColumn();

        // Cloud type combo box
        static const char* CLOUDTYPES[] = { "Cirrocumulus", "Cirrus Fibratus", "Stratus", "Cumulus Mediocris", "Cumulus Congestus",
        "Cumulus Congestus HiRes", "Cumulonimbus Cappilatus", "Stratocumulus", "Towering Cumulus", "Sandstorm" };
        static int currentCloudTypeIdx = 0;
        if (ImGui::BeginCombo("##cloudtype", CLOUDTYPES[currentCloudTypeIdx], 0))
        {
          for (int i = 0; i < IM_ARRAYSIZE(CLOUDTYPES); i++)
          {
            const bool isSelected = (currentCloudTypeIdx == i);
            if (ImGui::Selectable(CLOUDTYPES[i], isSelected))
              currentCloudTypeIdx = i;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }

        ImGui::SameLine();
        // Add clouds button
        if (ImGui::Button("Add"))
          s_CloudManager->addCloudType(static_cast<SilverLining::CloudTypes>(currentCloudTypeIdx));
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
          s_CloudManager->clearClouds();

        ImGui::TableNextColumn();
        ImGui::Text("Presets");
        ImGui::TableNextColumn();
        // Presets combo box
        static const char* PRESETS[] = { "Fair", "Partly Cloudy", "Mostly Cloudy", "Overcast" };
        static int currentPresetIdx = 0;
        if (ImGui::BeginCombo("##presets", PRESETS[currentPresetIdx], 0))
        {
          for (int i = 0; i < IM_ARRAYSIZE(PRESETS); i++)
          {
            const bool isSelected = (currentPresetIdx == i);
            if (ImGui::Selectable(PRESETS[i], isSelected))
              currentPresetIdx = i;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }

        ImGui::SameLine();
        // Apply preset button
        if (ImGui::Button("Apply"))
          s_SlSettings->conditionPreset()->set(static_cast<osgEarth::SilverLining::AtmosphericConditions::ConditionPresets>(currentPresetIdx));
      }
#endif /* HAVE_OSGEARTH_SILVERLINING */
      ImGui::EndTable();
    }

    ImGui::End();
  }

  private:
    osg::observer_ptr<osgEarth::SimpleOceanLayer> simpleOceanLayer_;
    osg::observer_ptr<osgEarth::VisibleLayer> tritonLayer_;
    PlatformBuoyancyCallback* buoyancyCallback_;
    osg::observer_ptr<SkyNode> skyNode_;
    osg::observer_ptr<simVis::View> view_;
    bool useTriton_ = false;
    bool useSilverLining_ = false;
    float opacity_ = 80.f;
    bool lighting_ = true;
};

#endif /* HAVE_IMGUI */

  /** Factory for a sky node */
  SkyNode* makeSky(simVis::SceneManager* scene, bool useSilverLining, const std::string& slUser = "", const std::string& slLicense = "", const std::string& resourcePath = "")
  {
#ifdef HAVE_OSGEARTH_SILVERLINING
    if (useSilverLining)
    {
      osgEarth::SilverLining::SilverLiningOptions skyOptions;
      if (!slUser.empty())
        skyOptions.user() = slUser;
      if (!slLicense.empty())
        skyOptions.licenseCode() = slLicense;
      if (!resourcePath.empty())
        skyOptions.resourcePath() = resourcePath;
      skyOptions.drawClouds() = true;
      skyOptions.cloudsMaxAltitude() = 100000.0;

      s_SlSettings->lensFlare()->set(true);

      // Configure clouds with the SilverLining callback settings
      s_SlSettings->addValue(s_CloudManager.get());
      return new SilverLining::SilverLiningNode(skyOptions, s_SlSettings.get());
    }
    else
    {
      return SkyNode::create();
    }
#else
    return SkyNode::create();
#endif /* HAVE_OSGEARTH_SILVERLINING */
  }

#ifdef HAVE_OSGEARTH_TRITON
  osgEarth::Triton::TritonLayer* makeTriton(const std::string& tritonUser = "", const std::string& tritonLicense = "", const std::string& resourcePath = "")
  {
    osgEarth::Triton::TritonLayer* rv = new osgEarth::Triton::TritonLayer();
    rv->setUserName(tritonUser);
    rv->setLicenseCode(tritonLicense);
    rv->setResourcePath(resourcePath);
    rv->setUseHeightMap(false);
    rv->setMaxAltitude(30000.0f);
    rv->setRenderBinNumber(simVis::BIN_OCEAN);
    rv->setOpacity(0.8f);
#if OSGEARTH_SOVERSION >= 154
    // render Triton on the ellipsoid (old default)
    rv->setVerticalDatum("");
#endif
    simVis::OverheadMode::configureOceanLayer(rv);
    return rv;
  }
#endif

  /** Factory an ocean node */
  osgEarth::SimpleOceanLayer* makeSimpleOcean()
  {
    osgEarth::SimpleOceanLayer* ocean = new osgEarth::SimpleOceanLayer();
    ocean->setUseBathymetry(false);
    ocean->setMaxAltitude(30000.0f);
    ocean->setOpacity(0.8f);
    osg::StateSet* stateSet = ocean->getOrCreateStateSet();
    stateSet->setRenderBinDetails(simVis::BIN_OCEAN, simVis::BIN_GLOBAL_SIMSDK);
    stateSet->setDefine("SIMVIS_IGNORE_BATHYMETRY_GEN");
    return ocean;
  }
}

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();

  // check for ocean and sky options.
  osg::ArgumentParser ap(&argc, argv);

  // Read licensing arguments for SilverLining
  std::string sluser;
  std::string sllicense;
  std::string slpath = simExamples::getSilverLiningResourcesPath(); // SilverLining Resource Path
  osg::ArgumentParser::Parameter sluserArg(sluser);
  osg::ArgumentParser::Parameter sllicenseArg(sllicense);
  osg::ArgumentParser::Parameter slpathArg(slpath);
  bool useSilverLining = ap.read("--silverlining");
  ap.read("--sluser", sluserArg);
  ap.read("--sllicense", sllicenseArg);
  ap.read("--slpath", slpathArg);

  // Read licensing arguments for Triton
  std::string tritonuser;
  std::string tritonlicense;
  std::string tritonpath = simExamples::getTritonResourcesPath(); // Triton Resource Path
  osg::ArgumentParser::Parameter tritonuserArg(tritonuser);
  osg::ArgumentParser::Parameter tritonlicenseArg(tritonlicense);
  osg::ArgumentParser::Parameter tritonpathArg(tritonpath);
  bool useTriton = ap.read("--triton");
  ap.read("--tritonuser", tritonuserArg);
  ap.read("--tritonlicense", tritonlicenseArg);
  ap.read("--tritonpath", tritonpathArg);

  // Offset the bathymetry?
  float bathymetryOffset = 0.0f;
  ap.read("--bathymetryoffset", bathymetryOffset);

#ifdef HAVE_OSGEARTH_TRITON
  // Set a default level for Triton
  if (ap.read("--good")) // default
    s_TritonSettings->quality()->set(osgEarth::Triton::GOOD);
  else if (ap.read("--better"))
    s_TritonSettings->quality()->set(osgEarth::Triton::BETTER);
  else if (ap.read("--best"))
    s_TritonSettings->quality()->set(osgEarth::Triton::BEST);
#endif /* HAVE_OSGEARTH_TRITON */

  // Set up the search paths
  simExamples::configureSearchPaths();

  // start up a SIMDIS viewer, prior to creating the map, to make sure they destruct in proper order
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  osg::ref_ptr<osgEarth::Map> map = new Map();

  // worldwide imagery layer:
  {
    TMSImageLayer* layer = new TMSImageLayer();
    layer->setName("simdis.imagery");
    layer->setURL(EXAMPLE_GLOBAL_IMAGERY_LAYER_TMS);
    map->addLayer(layer);
  }

  // If we are testing the bathymetry offset, only load a Kauai inset.
  if (bathymetryOffset != 0.0f)
  {
    MBTilesElevationLayer* layer = new MBTilesElevationLayer();
    layer->setName("simdis.elevation.no.bathy");
    layer->setURL(simExamples::getSampleDataPath() + "/terrain/" + EXAMPLE_ELEVATION_LAYER_DB);
    map->addLayer(layer);
  }
  else
  {
    TMSElevationLayer* layer = new TMSElevationLayer();
    layer->setName("simdis.elevation");
    layer->setURL(EXAMPLE_ELEVATION_LAYER_TMS);
    map->addLayer(layer);
  }

  viewer->setMap(map.get());
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  if (!scene->getMapNode() || !scene->getMapNode()->getTerrainEngine())
  {
    // this example can't even limp along without a terrain engine.
    SIM_ERROR << "Ocean example cannot continue with no terrain engine.\n";
    return 1;
  }

  // the data store houses the entity data model:
  simData::MemoryDataStore dataStore;
  simVis::ScenarioDataStoreAdapter adapter(&dataStore, scene->getScenario());

  // create our ship:
  s_shipId = createShip(dataStore);
  osg::ref_ptr<simVis::PlatformNode> shipNode = scene->getScenario()->find<simVis::PlatformNode>(s_shipId);

  // add a sky to the scene.
  osg::ref_ptr<SkyNode> sky = makeSky(scene.get(), useSilverLining, sluser, sllicense, slpath);
  sky->attach(viewer->getMainView());
  sky->setDateTime(osgEarth::DateTime(2014, 4, 22, 16.5));
  sky->getSunLight()->setAmbient(simVis::Color::Gray);
  scene->setSkyNode(sky.get());

  // add an ocean surface to the scene.
  osg::ref_ptr<osgEarth::SimpleOceanLayer> simpleOceanLayer;
  osg::ref_ptr<osgEarth::VisibleLayer> tritonLayer;
  osg::ref_ptr<PlatformBuoyancyCallback> buoyancyCallback;

#ifdef HAVE_OSGEARTH_TRITON
  if (useTriton)
  {
    osgEarth::Triton::TritonLayer* triton = makeTriton(tritonuser, tritonlicense, tritonpath);
    triton->setUserCallback(s_TritonSettings.get());
    viewer->getSceneManager()->getMap()->addLayer(triton);
    tritonLayer = triton;
    buoyancyCallback = new PlatformBuoyancyCallback(triton);
    shipNode->addCullCallback(buoyancyCallback.get());
  }
  else
#endif
  {
    simpleOceanLayer = makeSimpleOcean();
    if (simpleOceanLayer.valid())
        viewer->getSceneManager()->getMap()->addLayer(simpleOceanLayer.get());
  }

  // if we're using Triton, install a module to "sink" the MSL=0 terrain down, creating makeshift bathymetry
  if (bathymetryOffset != 0.0f && scene->getMapNode()->getTerrainEngine())
  {
    SIM_NOTICE << "Bathymetry offset = " << -bathymetryOffset << "\n";
    simVis::BathymetryGenerator* bgen = new simVis::BathymetryGenerator();
    bgen->setOffset(-bathymetryOffset);
    scene->getMapNode()->getTerrainEngine()->addEffect(bgen);
  }

  // zoom to the starting AOI:
  viewer->getMainView()->tetherCamera(shipNode.get());
  viewer->getMainView()->setFocalOffsets(80.0, -10.0, 2000.0);

#ifdef HAVE_IMGUI
  ::GUI::OsgImGuiHandler* gui = new ::GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(simpleOceanLayer.get(), tritonLayer.get(), buoyancyCallback.get(),
    sky.get(), viewer->getMainView(), useTriton, useSilverLining));
  viewer->addEventHandler(new MenuHandler(viewer.get(), scene.get()));
#endif

  viewer->installDebugHandlers();
  return viewer->run();
}
