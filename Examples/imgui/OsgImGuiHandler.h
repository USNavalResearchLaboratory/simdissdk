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

namespace osg { class Camera; }
namespace osgEarth { namespace GUI { class BaseGUI; } }

struct ImFont;
struct ImGuiSettingsHandler;

// TODO: update namespace to SimExamples
namespace GUI {

class BaseGui;

class GlewInitOperation : public osg::Operation
{
public:
  GlewInitOperation();
  void operator()(osg::Object* object) override;
};

/////////////////////////////////////////////////////////////////////////

class OsgImGuiHandler : public osgGA::GUIEventHandler
{
public:
  OsgImGuiHandler();

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

  /** Add a GUI to the manager */
  void add(osgEarth::GUI::BaseGUI* gui);
  /** Add deprecated GUI to the manager. TODO: Remove once ::GUI::BaseGUI is removed */
  void add(::GUI::BaseGui* gui);

  class RealizeOperation : public ::GUI::GlewInitOperation
  {
  public:
    /** Constructor. If passed a valid operation for parentOp, its operator() will be called first */
    explicit RealizeOperation(osg::Operation* parentOp = nullptr)
      : ::GUI::GlewInitOperation(),
      parentOp_(parentOp)
    {
    }

  private:
    void operator()(osg::Object* object) override
    {
      if (parentOp_.valid())
        parentOp_->operator()(object);
      GlewInitOperation::operator()(object);
    }

    osg::ref_ptr<osg::Operation> parentOp_;
  };

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
  bool mousePressed_[3];
  bool mouseDoubleClicked_[3];
  float mouseWheel_;
  bool initialized_;
  bool firstFrame_;
  bool firstDraw_;
  bool autoAdjustProjectionMatrix_;

  std::map<std::string, std::vector<std::unique_ptr<osgEarth::GUI::BaseGUI> > > menus_;
  std::vector<std::unique_ptr<::GUI::BaseGui> > deprecatedGuis_;

  ImFont* defaultFont_;
  ImFont* largeFont_;
};

}

#endif
