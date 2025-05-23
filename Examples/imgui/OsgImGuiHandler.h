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
#ifndef GUI_OSGIMGUIHANDLER_H
#define GUI_OSGIMGUIHANDLER_H

#include <memory>
#include <osgViewer/ViewerEventHandlers>
#include <osgEarth/Version>

namespace osg { class Camera; }
namespace osgEarth { namespace GUI { class BaseGUI; } }
namespace osgEarth { class ImGuiPanel; }

struct ImFont;
struct ImGuiSettingsHandler;

// TODO: update namespace to SimExamples
namespace GUI {

class BaseGui;

class OsgImGuiHandler : public osgGA::GUIEventHandler
{
public:
  OsgImGuiHandler();

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

  /** Add a GUI to the manager */
#if OSGEARTH_SOVERSION >= 159
  void add(osgEarth::ImGuiPanel* gui);
#else
  void add(osgEarth::GUI::BaseGUI* gui);
#endif

  /** Add deprecated GUI to the manager. TODO: Remove once ::GUI::BaseGUI is removed */
  void add(::GUI::BaseGui* gui);

  /** Get a pointer to the default font, may be NULL */
  ImFont* getDefaultFont() const;
  /** Get a pointer to the large font, may be NULL */
  ImFont* getLargeFont() const;

  /** True if the projection matrix should be auto-adjusted, e.g. during docking */
  bool getAutoAdjustProjectionMatrix() const;
  void setAutoAdjustProjectionMatrix(bool value);

protected:
  /// Initialize the ImGui environment
  void init_();
  // Put your ImGui code inside this function
  void draw_(osg::RenderInfo& renderInfo);

private:
  struct ImGuiNewFrameCallback;
  struct ImGuiRenderCallback;

  void setCameraCallbacks_(osg::Camera* camera);
  void newFrame_(osg::RenderInfo& renderInfo);
  void render_(osg::RenderInfo& renderInfo);

  double time_;
  bool initialized_;
  bool firstFrame_;
  bool firstDraw_;
  bool autoAdjustProjectionMatrix_;

#if OSGEARTH_SOVERSION >= 159
  std::map<std::string, std::vector<std::unique_ptr<osgEarth::ImGuiPanel> > > menus_;
#else
  std::map<std::string, std::vector<std::unique_ptr<osgEarth::GUI::BaseGUI> > > menus_;
#endif

  std::vector<std::unique_ptr<::GUI::BaseGui> > deprecatedGuis_;

  ImFont* defaultFont_;
  ImFont* largeFont_;
};

}

#endif
