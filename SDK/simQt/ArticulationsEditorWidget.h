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
#ifndef SIMQT_ARTICULATIONSEDITORWIDGET_H
#define SIMQT_ARTICULATIONSEDITORWIDGET_H

#include <memory>
#include <QWidget>
#include "osg/ref_ptr"
#include "osg/Node"
#include "osg/NodeVisitor"
#include "simCore/Common/Export.h"

class QStandardItemModel;
class QItemSelection;
class QDoubleSpinBox;
class QSlider;
class Ui_ArticulationsEditorWidget;

namespace osg { class Sequence; }
namespace osgSim {
  class DOFTransform;
  class MultiSwitch;
}

namespace simQt {

/** Articulation type enumeration, describing possible articulation types on a model */
enum class ArticulationType
{
  SEQUENCE,
  DOF_TRANSFORM,
  MULTI_SWITCH
};

/** Matches an ArticulationType and an osg::Node pointer holding the articulation node */
struct ArticulationItem
{
  ArticulationType articulationType_;
  osg::ref_ptr<osg::Node> articulationNode_;
};

/// Map for storing articulation name and type
using ArticulationMap = std::map<std::string, ArticulationItem>;

//////////////////////////////////////////////////////////////////////////////////////////////////

class SDKQT_EXPORT ArticulationsVisitor : public osg::NodeVisitor
{
public:
  ArticulationsVisitor();

  /** Override the apply() method to traverse the node and subgraph */
  virtual void apply(osg::Node& node);
  /** Retrieve articulation info collected from the model. */
  const simQt::ArticulationMap& getMap();

protected:
  /** Protect osg::Referenced-derived destructor */
  virtual ~ArticulationsVisitor() {}

private:
  /** Stores sequence node name and type to map. */
  void visit_(osg::Sequence* sequence);
  /** Stores DOF transform node name and type to map. */
  void visit_(osgSim::DOFTransform* dofTransform);
  /** Stores multi-switch node name and type to map. */
  void visit_(osgSim::MultiSwitch* multiSwitch);

  /// Map for holding articulation name and type
  simQt::ArticulationMap articulationMap_;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

/** Articulations widget is the graphical editor for a model's articulations */
class SDKQT_EXPORT ArticulationsEditorWidget : public QWidget
{
  Q_OBJECT;
public:
  /** Instantiates a window that can be used for editing articulations */
  explicit ArticulationsEditorWidget(QWidget* parent = nullptr);
  /** Clean up dynamic memory */
  virtual ~ArticulationsEditorWidget();

  /** Display articulation info for selected platform */
  void displayArticulationInfo(const std::map<std::string, ArticulationItem>& articulationMap);

private Q_SLOTS:
  /// Update articulation details to be shown in the stackedWidget
  void updateArticulationDetails_(const QItemSelection& selectedItems);
  /// Update multiswitch articulation node
  void updateMultiSwitch_(int multiSwitchId);
  /// Update sequence articulation node
  void updateSequence_(int state);

  /// Update spin box heading for DOFTransform
  void setSpinBoxCurrentHeading_(double val);
  /// Update slider heading for DOFTransform
  void setSliderCurrentHeading_(int val);
  /// Update spin box pitch for DOFTransform
  void setSpinBoxCurrentPitch_(double val);
  /// Update slider pitch for DOFTransform
  void setSliderCurrentPitch_(int val);
  /// Update spin box roll for DOFTransform
  void setSpinBoxCurrentRoll_(double val);
  /// Update slider roll for DOFTransform
  void setSliderCurrentRoll_(int val);
  /// Update translate x-value for DOFTransform
  void setCurrentTranslateX_(double val);
  /// Update translate y-value for DOFTransform
  void setCurrentTranslateY_(double val);
  /// Update translate z-value for DOFTransform
  void setCurrentTranslateZ_(double val);
  /// Update scale x-value for DOFTransform
  void setCurrentScaleX_(double val);
  /// Update scale y-value for DOFTransform
  void setCurrentScaleY_(double val);
  /// Update scale z-value for DOFTransform
  void setCurrentScaleZ_(double val);

  /// Reset entity articulation info from model and set stackedWidget to invisible
  void resetArticulationsInfo_();

private:

  /// Enumeration to determine which articulation needs to be updated
  enum class DofType
  {
    HEADING,
    PITCH,
    ROLL,
    TRANSLATE_X,
    TRANSLATE_Y,
    TRANSLATE_Z,
    SCALE_X,
    SCALE_Y,
    SCALE_Z
  };

  /** Convert articulation type to QString */
  QString articulationTypeToQString_(ArticulationType type) const;

  /**
   * Set minimum, maximum, and current values for a given spin box
   * @param box Spin box to update
   * @param minVal Minimum value allowed for spin box
   * @param maxVal Maximum value allowed for spin box
   * @param currentVal Current value to be set for spin box
   * @param type Degree-of-freedom type
   */
  void updateSpinBox_(QDoubleSpinBox* box, double minVal, double maxVal, double currentVal, DofType type) const;

  /**
   * Set minimum, maximum, and current values for a given slider
   * @param slider Slider to update
   * @param minVal Minimum value allowed for slider
   * @param maxVal Maximum value allowed for slider
   * @param currentVal Current value to be set for slider
   */
  void updateSlider_(QSlider* slider, double minVal, double maxVal, double currentVal);

  /**
   * Update DOFTransform node value, changing only one degree-of-freedom at a time
   * @param type Enumeration describing which DOFTransform articulation will be updated
   * @param val Value to set current articulation value to
   */
  void setDofTransformValue_(DofType type, double val);

  std::unique_ptr<Ui_ArticulationsEditorWidget> ui_;
  QStandardItemModel* itemModel_;
  /// Current articulated node being displayed
  osg::ref_ptr<osg::Node> activeNode_;
};

}

#endif /* SIMQT_ARTICULATIONSEDITORWIDGET_H */
