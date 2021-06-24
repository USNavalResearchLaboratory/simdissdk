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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef GUI_OSGIMGUIHANDLER_H
#define GUI_OSGIMGUIHANDLER_H

#include <osgViewer/ViewerEventHandlers>

namespace osg { class Camera; }

struct ImGuiSettingsHandler;

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

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

  /** Add a GUI to the manager */
  void add(BaseGui* gui);

  static void init();

  class RealizeOperation : public GUI::GlewInitOperation
  {
    void operator()(osg::Object* object) override
    {
      GlewInitOperation::operator()(object);
      OsgImGuiHandler::init();
    }
  };

protected:
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
  std::vector<std::unique_ptr<BaseGui> > guis_;
};

}

#endif
