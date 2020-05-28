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
#ifndef SS_VIEWERAPP_H
#define SS_VIEWERAPP_H

#include "osg/ref_ptr"
#include "simCore/String/TextReplacer.h"

namespace osg { class ArgumentParser; }
namespace osgEarth { namespace Util { namespace Controls {
  class Control;
} } }
namespace simCore { class ClockImpl; }
namespace simData {
  class DataStore;
  class Interpolator;
}
namespace simVis {
  class Compass;
  class SceneManager;
  class View;
  class ViewManager;
  class ViewManagerLogDbAdapter;
}
namespace simUtil {
  class StatusText;
  class TimeVariable;
}

namespace SimpleServer {

class DataEngine;

/** Responsible for glueing together the various SDK view elements with data providers */
class ViewerApp
{
public:
  explicit ViewerApp(osg::ArgumentParser& args);
  virtual ~ViewerApp();

  int run();

  void exit();
  void toggleDynamicScale();
  void toggleLabels();
  void centerNext();
  void toggleCompass();
  void toggleLogDb();
  void cycleTimeFormat();
  void toggleCockpit();
  void playPause();
  void toggleTextDeclutter();
  void toggleDeclutterTechnique();
  void cycleCalloutLineStyle();

private:
  void init_(osg::ArgumentParser& args);
  osgEarth::Util::Controls::Control* createHelp_() const;
  int loadGog_(const std::string& filename);

  osg::ref_ptr<simVis::SceneManager> sceneManager_;
  osg::ref_ptr<simVis::ViewManager> viewManager_;
  osg::ref_ptr<simVis::ViewManagerLogDbAdapter> logDb_;
  osg::ref_ptr<simVis::Compass> compass_;
  osg::ref_ptr<simVis::View> superHud_;
  osg::ref_ptr<simUtil::StatusText> cornerStatus_;
  osg::ref_ptr<DataEngine> engine_;
  simCore::ClockImpl* clock_;
  simCore::TextReplacerPtr textReplacer_;
  simData::DataStore* dataStore_;
  simData::Interpolator* interpolator_;
  simUtil::TimeVariable* timeVariable_;

  bool declutterOn_;
  int colorIndex_;
};

}

#endif /* SS_VIEWERAPP_H */
