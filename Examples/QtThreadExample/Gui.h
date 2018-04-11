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

#ifndef SDK_QTHREAD_EXAMPLE_GUI
#define SDK_QTHREAD_EXAMPLE_GUI

#include <QDialog>

class Ui_ThreadExample;

namespace SdkQThreadExample
{
/** Dialog for displaying Start button, Stop button and status */
class Gui : public QDialog
{
  Q_OBJECT;

public:
  explicit Gui(QWidget* parent = NULL);
  virtual ~Gui();

  /** Update the text for the number of updates processed */
  void updateNumberProcessed(unsigned int number);

signals:
  void startClicked();
  void stopClicked();

private:
  Ui_ThreadExample * ui_;
};

}

#endif
