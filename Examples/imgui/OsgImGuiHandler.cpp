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
#include "GL/glew.h"
#include <osg/Camera>
#include <osg/RenderInfo>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#include "osgEarth/Version"
#include "osgEarth/ShaderLoader"
#include "osgEarth/VirtualProgram"

#if OSGEARTH_SOVERSION >= 159
  #include "osgEarthImGui/CameraGUI"
  #include "osgEarthImGui/EnvironmentGUI"
  using namespace osgEarth;
#else
  #include "osgEarth/ImGui/ImGui"
  #include "osgEarth/ImGui/CameraGUI"
  #include "osgEarth/ImGui/EnvironmentGUI"
  using namespace osgEarth::GUI;
#endif

#if OSGEARTH_SOVERSION < 146
#undef NOMINMAX
#endif

#if OSGEARTH_SOVERSION >= 148
  // Fix 3.4.0 bugs with namespace in osgEarth/ImGui/AnnotationsGUI
  #include "osgEarth/AnnotationData"
  using AnnotationData = osgEarth::AnnotationData;
  using EarthManipulator = osgEarth::EarthManipulator;

  #if OSGEARTH_SOVERSION >= 159
    #include "osgEarthImGui/AnnotationsGUI"
  #else
    #include "osgEarth/ImGui/AnnotationsGUI"
  #endif
#endif

#if 0
#include "osgEarth/ImGui/LayersGUI"
#endif

#if OSGEARTH_SOVERSION >= 159
  #include "osgEarthImGui/NetworkMonitorGUI"
  #include "osgEarthImGui/RenderingGUI"
  #include "osgEarthImGui/SceneGraphGUI"
  #include "osgEarthImGui/SystemGUI"
  #include "osgEarthImGui/TerrainGUI"
  #include "osgEarthImGui/TextureInspectorGUI"
  #include "osgEarthImGui/ViewpointsGUI"
#else
  #include "osgEarth/ImGui/NetworkMonitorGUI"
  #include "osgEarth/ImGui/RenderingGUI"
  #include "osgEarth/ImGui/SceneGraphGUI"
  #include "osgEarth/ImGui/SystemGUI"
  #include "osgEarth/ImGui/TerrainGUI"
  #include "osgEarth/ImGui/TextureInspectorGUI"
  #include "osgEarth/ImGui/ViewpointsGUI"
#endif

#include "simNotify/Notify.h"
#include "simCore/Calc/Interpolation.h"
#include "simVis/Registry.h"
#include "SimExamplesGui.h"

namespace GUI {

GlewInitOperation::GlewInitOperation()
  : osg::Operation("GlewInitCallback", false)
{
}

void GlewInitOperation::operator()(osg::Object* object)
{
  osg::GraphicsContext* context = dynamic_cast<osg::GraphicsContext*>(object);
  if (!context)
    return;

  if (glewInit() != GLEW_OK)
  {
    SIM_ERROR << "glewInit() failed" << std::endl;
  }
}

///////////////////////////////////////////////////////////////////////////////

struct OsgImGuiHandler::ImGuiNewFrameCallback : public osg::Camera::DrawCallback
{
  explicit ImGuiNewFrameCallback(OsgImGuiHandler& handler)
    : handler_(handler)
  {
  }

  void operator()(osg::RenderInfo& renderInfo) const override
  {
    handler_.newFrame_(renderInfo);
  }

private:
  OsgImGuiHandler& handler_;
};

///////////////////////////////////////////////////////////////////////////////

struct OsgImGuiHandler::ImGuiRenderCallback : public osg::Camera::DrawCallback
{
  explicit ImGuiRenderCallback(OsgImGuiHandler& handler)
    : handler_(handler)
  {
  }

  void operator()(osg::RenderInfo& renderInfo) const override
  {
    handler_.render_(renderInfo);
  }

private:
  OsgImGuiHandler& handler_;
};

///////////////////////////////////////////////////////////////////////////////

OsgImGuiHandler::OsgImGuiHandler()
  : time_(0.0f),
  initialized_(false),
  firstFrame_(true),
  firstDraw_(true),
  autoAdjustProjectionMatrix_(true)
{
#if OSGEARTH_SOVERSION >= 148
  menus_["Tools"].push_back(std::unique_ptr<AnnotationsGUI>(new AnnotationsGUI));
#endif
  menus_["Tools"].push_back(std::unique_ptr<CameraGUI>(new CameraGUI));
  menus_["Tools"].push_back(std::unique_ptr<EnvironmentGUI>(new EnvironmentGUI));
#if 0
  menus_["Tools"].push_back(std::unique_ptr<LayersGUI>(new LayersGUI));
#endif

  menus_["Tools"].push_back(std::unique_ptr<NetworkMonitorGUI>(new NetworkMonitorGUI));
  menus_["Tools"].push_back(std::unique_ptr<NVGLInspectorGUI>(new NVGLInspectorGUI));
  menus_["Tools"].push_back(std::unique_ptr<RenderingGUI>(new RenderingGUI));
  menus_["Tools"].push_back(std::unique_ptr<SceneGraphGUI>(new SceneGraphGUI));
  // Not including ShaderGUI as it expects command line arguments. Can be added later if needed
  menus_["Tools"].push_back(std::unique_ptr<SystemGUI>(new SystemGUI));
  menus_["Tools"].push_back(std::unique_ptr<TerrainGUI>(new TerrainGUI));
  menus_["Tools"].push_back(std::unique_ptr<TextureInspectorGUI>(new TextureInspectorGUI));
  menus_["Tools"].push_back(std::unique_ptr<ViewpointsGUI>(new ViewpointsGUI));
}

#if OSGEARTH_SOVERSION >= 159
void OsgImGuiHandler::add(osgEarth::ImGuiPanel* gui)
{
  if (gui != nullptr)
    menus_["User"].push_back(std::unique_ptr<ImGuiPanel>(gui));
}
#else
void OsgImGuiHandler::add(osgEarth::GUI::BaseGUI* gui)
{
  if (gui != nullptr)
    menus_["User"].push_back(std::unique_ptr<osgEarth::GUI::BaseGUI>(gui));
}
#endif

void OsgImGuiHandler::add(::GUI::BaseGui* gui)
{
  std::cerr << "GUI \"" << gui->name() << "\" is of a deprecated type (::GUI::BaseGui). Update to simExamples::SimExamplesGui\n";
  deprecatedGuis_.push_back(std::unique_ptr<::GUI::BaseGui>(gui));
}

static ImGuiKey convertKey(int c)
{
  // If you are holding CTRL, OSG remaps A-Z to 1-26. Undo that.
  if (c >= 1 && c <= 26)
    return static_cast<ImGuiKey>(static_cast<int>(ImGuiKey_A) + c - 1);

  if (c >= osgGA::GUIEventAdapter::KEY_0 && c <= osgGA::GUIEventAdapter::KEY_9)
    return static_cast<ImGuiKey>(static_cast<int>(ImGuiKey_0) + c - osgGA::GUIEventAdapter::KEY_0);

  if (c >= osgGA::GUIEventAdapter::KEY_A && c <= osgGA::GUIEventAdapter::KEY_Z)
    return static_cast<ImGuiKey>(static_cast<int>(ImGuiKey_A) + c - osgGA::GUIEventAdapter::KEY_A);

  switch (c)
  {
  case osgGA::GUIEventAdapter::KEY_Tab:
    return ImGuiKey_Tab;
  case osgGA::GUIEventAdapter::KEY_Left:
    return ImGuiKey_LeftArrow;
  case osgGA::GUIEventAdapter::KEY_Right:
    return ImGuiKey_RightArrow;
  case osgGA::GUIEventAdapter::KEY_Up:
    return ImGuiKey_UpArrow;
  case osgGA::GUIEventAdapter::KEY_Down:
    return ImGuiKey_DownArrow;
  case osgGA::GUIEventAdapter::KEY_Page_Up:
    return ImGuiKey_PageUp;
  case osgGA::GUIEventAdapter::KEY_Page_Down:
    return ImGuiKey_PageDown;
  case osgGA::GUIEventAdapter::KEY_Home:
    return ImGuiKey_Home;
  case osgGA::GUIEventAdapter::KEY_End:
    return ImGuiKey_End;
  case osgGA::GUIEventAdapter::KEY_Delete:
    return ImGuiKey_Delete;
  case osgGA::GUIEventAdapter::KEY_BackSpace:
    return ImGuiKey_Backspace;
  case osgGA::GUIEventAdapter::KEY_Return:
    return ImGuiKey_Enter;
  case osgGA::GUIEventAdapter::KEY_Escape:
    return ImGuiKey_Escape;
  case osgGA::GUIEventAdapter::KEY_Space:
    return ImGuiKey_Space;
  }

  return ImGuiKey_None;
}

static ImGuiButtonFlags convertMouseButton(int m)
{
  ImGuiButtonFlags flags = 0;
  if (m & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    flags |= ImGuiMouseButton_Left;
  if (m & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    flags |= ImGuiMouseButton_Right;
  if (m & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
    flags |= ImGuiMouseButton_Middle;
  return flags;
}

ImFont* OsgImGuiHandler::getDefaultFont() const
{
  return defaultFont_;
}

ImFont* OsgImGuiHandler::getLargeFont() const
{
  return largeFont_;
}

bool OsgImGuiHandler::getAutoAdjustProjectionMatrix() const
{
  return autoAdjustProjectionMatrix_;
}

void OsgImGuiHandler::setAutoAdjustProjectionMatrix(bool value)
{
  autoAdjustProjectionMatrix_ = value;
}

void OsgImGuiHandler::init_()
{
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  ImGui_ImplOpenGL3_Init();

  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  std::string font = simVis::Registry::instance()->findFontFile("droidsdans.ttf");
  // Attempt fallback to arial if droidsans isn't available
  if (font.empty())
    font = simVis::Registry::instance()->findFontFile("arial.ttf");
  if (!font.empty())
  {
    defaultFont_ = io.Fonts->AddFontFromFileTTF(font.c_str(), 14.f);
    largeFont_ = io.Fonts->AddFontFromFileTTF(font.c_str(), 24.f);
  }
}

void OsgImGuiHandler::setCameraCallbacks_(osg::Camera* camera)
{
  // potential gotcha, need to be chained with pre-existing callbacks
  camera->setPreDrawCallback(new ImGuiNewFrameCallback(*this));
  camera->setPostDrawCallback(new ImGuiRenderCallback(*this));
}

void OsgImGuiHandler::newFrame_(osg::RenderInfo& renderInfo)
{
  if (firstFrame_)
  {
    init_();
    firstFrame_ = false;
  }

  ImGui_ImplOpenGL3_NewFrame();

  ImGuiIO& io = ImGui::GetIO();

  io.DisplaySize = ImVec2(renderInfo.getCurrentCamera()->getGraphicsContext()->getTraits()->width, renderInfo.getCurrentCamera()->getGraphicsContext()->getTraits()->height);

  double currentTime = renderInfo.getView()->getFrameStamp()->getSimulationTime();
  io.DeltaTime = currentTime - time_ + 0.0000001;
  time_ = currentTime;

  ImGui::NewFrame();
}

void OsgImGuiHandler::render_(osg::RenderInfo& ri)
{
  auto camera = ri.getCurrentCamera();
  auto viewport = camera->getViewport();

  constexpr ImGuiDockNodeFlags dockspace_flags =
    ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode;

  auto dockSpaceId = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

  draw_(ri);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  auto centralNode = ImGui::DockBuilderGetCentralNode(dockSpaceId);

  auto io = ImGui::GetIO();
  const double newX = centralNode->Pos.x;
  const double newY = io.DisplaySize.y - centralNode->Size.y - centralNode->Pos.y;
  const double newWidth = centralNode->Size.x;
  const double newHeight = centralNode->Size.y;

  // If we do not adjust viewport, no need to adjust projection matrix
  if (osg::equivalent(viewport->x(), newX) && osg::equivalent(viewport->y(), newY) &&
    osg::equivalent(viewport->width(), newWidth) && osg::equivalent(viewport->height(), newHeight))
  {
    return;
  }

  // Make a copy of the viewport values before we change the positions; ortho calculations need these
  const double oldX = viewport->x();
  const double oldY = viewport->y();
  const double oldWidth = viewport->width();
  const double oldHeight = viewport->height();
  viewport->x() = newX;
  viewport->y() = newY;
  viewport->width() = newWidth;
  viewport->height() = newHeight;

  if (autoAdjustProjectionMatrix_)
  {
    const osg::Matrixd& proj = camera->getProjectionMatrix();
    const bool isOrtho = osg::equivalent(proj(3, 3), 1.0);
    if (!isOrtho)
    {
      double fovy, ar, znear, zfar;
      camera->getProjectionMatrixAsPerspective(fovy, ar, znear, zfar);
      camera->setProjectionMatrixAsPerspective(fovy, viewport->width() / viewport->height(), znear, zfar);
    }
    else
    {
      double left, right, bottom, top, znear, zfar;
      camera->getProjectionMatrixAsOrtho(left, right, bottom, top, znear, zfar);

      // Scale the projection matrix by the same ratio that the viewport gets adjusted. This is required
      // in order to deal with osgEarth EarthManipulator zoom in/out capabilities in ortho mode, where
      // the left/right/top/bottom values are not equal to viewport coordinates.
      auto mapX = [=](double x) -> double {
        return simCore::linearInterpolate(left, right, oldX, x, oldX + oldWidth);
      };
      auto mapY = [=](double y) -> double {
        return simCore::linearInterpolate(bottom, top, oldY, y, oldY + oldHeight);
      };
      const double newLeft = mapX(viewport->x());
      const double newRight = mapX(viewport->x() + viewport->width());
      const double newBottom = mapY(viewport->y());
      const double newTop = mapY(viewport->y() + viewport->height());
      camera->setProjectionMatrixAsOrtho(newLeft, newRight, newBottom, newTop, znear, zfar);
    }
  }
}

bool OsgImGuiHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (!initialized_)
  {
    auto view = aa.asView();
    if (view)
    {
      setCameraCallbacks_(view->getCamera());
      initialized_ = true;
      return false;
    }
  }

  ImGuiIO& io = ImGui::GetIO();

  switch (ea.getEventType())
  {
  case osgGA::GUIEventAdapter::KEYDOWN:
  case osgGA::GUIEventAdapter::KEYUP:
  {
    const bool isKeyDown = ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN;
    int c = ea.getKey();

    // Always update the mod key status
    io.AddKeyEvent(ImGuiMod_Ctrl, (ea.getModKeyMask() & ea.MODKEY_CTRL) != 0);
    io.AddKeyEvent(ImGuiMod_Shift, (ea.getModKeyMask() & ea.MODKEY_SHIFT) != 0);
    io.AddKeyEvent(ImGuiMod_Alt, (ea.getModKeyMask() & ea.MODKEY_ALT) != 0);
    io.AddKeyEvent(ImGuiMod_Super, (ea.getModKeyMask() & ea.MODKEY_SUPER) != 0);

    // ImGuiIo::AddKeyEvent() requires a "translated" key input,
    // so manually translate the OSG int key to ImGuiKey
    const ImGuiKey imguiKey = convertKey(c);
    io.AddKeyEvent(imguiKey, isKeyDown);

    // Send any raw ASCII characters to ImGui as input
    if (isKeyDown)
    {
      // Convert keypad numbers to their normal ASCII equivalents before sending
      if (c >= osgGA::GUIEventAdapter::KEY_KP_0 && c <= osgGA::GUIEventAdapter::KEY_KP_9)
        c = osgGA::GUIEventAdapter::KEY_0 + c - osgGA::GUIEventAdapter::KEY_KP_0;
      io.AddInputCharacter(static_cast<unsigned int>(c));
    }

    return io.WantCaptureKeyboard;
  }
  case (osgGA::GUIEventAdapter::PUSH):
  {
    if (io.WantCaptureMouse)
    {
      auto imguiButton = convertMouseButton(ea.getButtonMask());
      io.AddMousePosEvent(ea.getX(), io.DisplaySize.y - ea.getY());
      io.AddMouseButtonEvent(imguiButton, true); // true = push
    }
    return io.WantCaptureMouse;
  }
  case (osgGA::GUIEventAdapter::RELEASE):
  {
    if (io.WantCaptureMouse)
      io.AddMousePosEvent(ea.getX(), io.DisplaySize.y - ea.getY());

    const ImGuiButtonFlags imguiButton = convertMouseButton(ea.getButtonMask());
    io.AddMouseButtonEvent(imguiButton, false); // false = release

    return io.WantCaptureMouse;
  }
  case (osgGA::GUIEventAdapter::DRAG):
  case (osgGA::GUIEventAdapter::MOVE):
  {
    io.AddMousePosEvent(ea.getX(), io.DisplaySize.y - ea.getY());
    return io.WantCaptureMouse;
  }
  case (osgGA::GUIEventAdapter::SCROLL):
  {
    auto scrolling = ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP ? 1.0 : -1.0;
    io.AddMouseWheelEvent(0.0, io.MouseWheel += scrolling);
    return io.WantCaptureMouse;
  }
  default:
    break;
  }

  return false;
}

void OsgImGuiHandler::draw_(osg::RenderInfo& ri)
{
  // Build the menu bar
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      bool quit = false;
      ImGui::MenuItem("Quit", nullptr, &quit);
      if (quit) exit(0);
      ImGui::EndMenu();
    }

    for (auto& menuIter : menus_)
    {
      if (ImGui::BeginMenu(menuIter.first.c_str()))
      {
        for (auto& gui : menuIter.second)
          ImGui::MenuItem(gui->name(), nullptr, gui->visible());
        ImGui::EndMenu();
      }
    }
    ImGui::EndMainMenuBar();
  }

  // Draw each GUI
  for (auto& menuIter : menus_)
  {
    for (auto& gui : menuIter.second)
    {
      // Initialize fonts for SimExamplesGuis on first draw
      if (firstDraw_ && menuIter.first == "User")
      {
        simExamples::SimExamplesGui* seGui = dynamic_cast<simExamples::SimExamplesGui*>(gui.get());
        assert(seGui); // Expected that all values added using add() are simExamples::SimExamplesGui type
        if (seGui)
        {
          if (defaultFont_)
            seGui->setDefaultFont(defaultFont_);
          if (largeFont_)
            seGui->setLargeFont(largeFont_);
        }
      }
      gui->draw(ri);
    }
  }

  for (auto& gui : deprecatedGuis_)
  {
    if (firstDraw_)
    {
      if (defaultFont_)
        gui->setDefaultFont(defaultFont_);
      if (largeFont_)
        gui->setLargeFont(largeFont_);
    }
    gui->draw(ri);
  }

  firstDraw_ = false;
}

}
