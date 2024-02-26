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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
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
#include "osgEarth/ImGui/CameraGUI"
#include "osgEarth/ImGui/EnvironmentGUI"

#if OSGEARTH_SOVERSION < 146
#undef NOMINMAX
#endif

#if OSGEARTH_SOVERSION >= 148
 // Fix 3.4.0 bugs with namespace in osgEarth/ImGui/AnnotationsGUI
#include "osgEarth/AnnotationData"
using AnnotationData = osgEarth::AnnotationData;
using EarthManipulator = osgEarth::EarthManipulator;

#include "osgEarth/ImGui/AnnotationsGUI"
#endif

#if 0
#include "osgEarth/ImGui/LayersGUI"
#endif

#include "osgEarth/ImGui/NetworkMonitorGUI"
#include "osgEarth/ImGui/RenderingGUI"
#include "osgEarth/ImGui/SceneGraphGUI"

// SetItemTooltip() is not in ImGui 18204 shipped with SIMDIS SDK, but is
// used in osgEarth/ImGui/SystemGUI using ImGui 19013; it was added in 18964.
#if IMGUI_VERSION_NUM < 18964
// Replace SetItemTooltip() with a function that evaluates to a noop
#define SetItemTooltip(x) GetCurrentContext()
#endif

#include "osgEarth/ImGui/SystemGUI"

#if IMGUI_VERSION_NUM < 18964
#undef SetItemTooltip
#endif

#include "osgEarth/ImGui/TerrainGUI"
#include "osgEarth/ImGui/TextureInspectorGUI"
#include "osgEarth/ImGui/ViewpointsGUI"
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
  mousePressed_{false},
  mouseWheel_(0.0f),
  initialized_(false),
  firstFrame_(true),
  firstDraw_(true),
  autoAdjustProjectionMatrix_(true)
{
#if OSGEARTH_SOVERSION >= 148
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::AnnotationsGUI>(new osgEarth::GUI::AnnotationsGUI));
#endif
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::CameraGUI>(new osgEarth::GUI::CameraGUI));
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::EnvironmentGUI>(new osgEarth::GUI::EnvironmentGUI));
#if 0
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::LayersGUI>(new osgEarth::GUI::LayersGUI));
#endif

  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::NetworkMonitorGUI>(new osgEarth::GUI::NetworkMonitorGUI));
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::NVGLInspectorGUI>(new osgEarth::GUI::NVGLInspectorGUI));
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::RenderingGUI>(new osgEarth::GUI::RenderingGUI));
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::SceneGraphGUI>(new osgEarth::GUI::SceneGraphGUI));
  // Not including ShaderGUI as it expects command line arguments. Can be added later if needed
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::SystemGUI>(new osgEarth::GUI::SystemGUI));
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::TerrainGUI>(new osgEarth::GUI::TerrainGUI));
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::TextureInspectorGUI>(new osgEarth::GUI::TextureInspectorGUI));
  menus_["Tools"].push_back(std::unique_ptr<osgEarth::GUI::ViewpointsGUI>(new osgEarth::GUI::ViewpointsGUI));
}

void OsgImGuiHandler::add(osgEarth::GUI::BaseGUI* gui)
{
  if (gui != nullptr)
    menus_["User"].push_back(std::unique_ptr<osgEarth::GUI::BaseGUI>(gui));
}

void OsgImGuiHandler::add(::GUI::BaseGui* gui)
{
  std::cerr << "GUI \"" << gui->name() << "\" is of a deprecated type (::GUI::BaseGui). Update to simExamples::SimExamplesGui\n";
  deprecatedGuis_.push_back(std::unique_ptr<::GUI::BaseGui>(gui));
}

/**
 * Important Note: Dear ImGui expects the control Keys indices not to be
 * greater than 511. It actually uses an array of 512 elements. However,
 * OSG has indices greater than that. So here I do a conversion for special
 * keys between ImGui and OSG.
 */
static int ConvertFromOSGKey(int key)
{
  using KEY = osgGA::GUIEventAdapter::KeySymbol;

  switch (key)
  {
    case KEY::KEY_Tab:
        return ImGuiKey_Tab;
    case KEY::KEY_Left:
        return ImGuiKey_LeftArrow;
    case KEY::KEY_Right:
        return ImGuiKey_RightArrow;
    case KEY::KEY_Up:
        return ImGuiKey_UpArrow;
    case KEY::KEY_Down:
        return ImGuiKey_DownArrow;
    case KEY::KEY_Page_Up:
        return ImGuiKey_PageUp;
    case KEY::KEY_Page_Down:
        return ImGuiKey_PageDown;
    case KEY::KEY_Home:
        return ImGuiKey_Home;
    case KEY::KEY_End:
        return ImGuiKey_End;
    case KEY::KEY_Delete:
        return ImGuiKey_Delete;
    case KEY::KEY_BackSpace:
        return ImGuiKey_Backspace;
    case KEY::KEY_Return:
        return ImGuiKey_Enter;
    case KEY::KEY_Escape:
        return ImGuiKey_Escape;
    case 22:
        return osgGA::GUIEventAdapter::KeySymbol::KEY_V;
    case 3:
        return osgGA::GUIEventAdapter::KeySymbol::KEY_C;
    default: // Not found
        return key;
  }
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

  // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
  io.KeyMap[ImGuiKey_Tab] = ImGuiKey_Tab;
  io.KeyMap[ImGuiKey_LeftArrow] = ImGuiKey_LeftArrow;
  io.KeyMap[ImGuiKey_RightArrow] = ImGuiKey_RightArrow;
  io.KeyMap[ImGuiKey_UpArrow] = ImGuiKey_UpArrow;
  io.KeyMap[ImGuiKey_DownArrow] = ImGuiKey_DownArrow;
  io.KeyMap[ImGuiKey_PageUp] = ImGuiKey_PageUp;
  io.KeyMap[ImGuiKey_PageDown] = ImGuiKey_PageDown;
  io.KeyMap[ImGuiKey_Home] = ImGuiKey_Home;
  io.KeyMap[ImGuiKey_End] = ImGuiKey_End;
  io.KeyMap[ImGuiKey_Delete] = ImGuiKey_Delete;
  io.KeyMap[ImGuiKey_Backspace] = ImGuiKey_Backspace;
  io.KeyMap[ImGuiKey_Enter] = ImGuiKey_Enter;
  io.KeyMap[ImGuiKey_Escape] = ImGuiKey_Escape;
  io.KeyMap[ImGuiKey_A] = osgGA::GUIEventAdapter::KeySymbol::KEY_A;
  io.KeyMap[ImGuiKey_C] = osgGA::GUIEventAdapter::KeySymbol::KEY_C;
  io.KeyMap[ImGuiKey_V] = osgGA::GUIEventAdapter::KeySymbol::KEY_V;
  io.KeyMap[ImGuiKey_X] = osgGA::GUIEventAdapter::KeySymbol::KEY_X;
  io.KeyMap[ImGuiKey_Y] = osgGA::GUIEventAdapter::KeySymbol::KEY_Y;
  io.KeyMap[ImGuiKey_Z] = osgGA::GUIEventAdapter::KeySymbol::KEY_Z;

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

  for (int i = 0; i < 3; i++)
  {
    io.MouseDown[i] = mousePressed_[i];
  }

  for (int i = 0; i < 3; i++)
  {
    io.MouseDoubleClicked[i] = mouseDoubleClicked_[i];
  }

  io.MouseWheel = mouseWheel_;
  mouseWheel_ = 0.0f;

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
  const bool wantCaptureMouse = io.WantCaptureMouse;
  const bool wantCaptureKeyboard = io.WantCaptureKeyboard;

  switch (ea.getEventType())
  {
  case osgGA::GUIEventAdapter::KEYDOWN:
  case osgGA::GUIEventAdapter::KEYUP:
  {
    const bool isKeyDown = ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN;
    const int c = ea.getKey();

    // Always update the mod key status.
    io.KeyCtrl = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL;
    io.KeyShift = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT;
    io.KeyAlt = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT;
    io.KeySuper = ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SUPER;

    const int imgui_key = ConvertFromOSGKey(c);
    if (imgui_key > 0 && imgui_key < 512)
    {
      //assert((imgui_key >= 0 && imgui_key < 512) && "ImGui KeysMap is an array of 512");
      io.KeysDown[imgui_key] = isKeyDown;
    }

    // Not sure this < 512 is correct here....
    if (isKeyDown && imgui_key >= 32 && imgui_key < 512)
    {
      io.AddInputCharacter(static_cast<unsigned int>(c));
    }

    return wantCaptureKeyboard;
  }
  case osgGA::GUIEventAdapter::RELEASE:
  {
    io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
    mousePressed_[0] = ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
    mousePressed_[1] = ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
    mousePressed_[2] = ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;

    mouseDoubleClicked_[0] = ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
    mouseDoubleClicked_[1] = ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
    mouseDoubleClicked_[2] = ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;
    return wantCaptureMouse;
  }
  case osgGA::GUIEventAdapter::PUSH:
  {
    io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
    mousePressed_[0] = ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
    mousePressed_[1] = ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
    mousePressed_[2] = ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;
    return wantCaptureMouse;
  }
  case osgGA::GUIEventAdapter::DOUBLECLICK:
  {
    io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
    // Need to set mousePressed_ flags in addition to mouseDoubleClicked_ flags to satisfy
    // double click requirements of some ImGui elements like ImGui::TreeNodeEx
    mousePressed_[0] = ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
    mousePressed_[1] = ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
    mousePressed_[2] = ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;
    mouseDoubleClicked_[0] = ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
    mouseDoubleClicked_[1] = ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
    mouseDoubleClicked_[2] = ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;
    return wantCaptureMouse;
  }
  case osgGA::GUIEventAdapter::DRAG:
  case osgGA::GUIEventAdapter::MOVE:
  {
    io.MousePos = ImVec2(ea.getX(), io.DisplaySize.y - ea.getY());
    return wantCaptureMouse;
  }
  case osgGA::GUIEventAdapter::SCROLL:
  {
    mouseWheel_ = ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP ? 1.0 : -1.0;
    return wantCaptureMouse;
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
