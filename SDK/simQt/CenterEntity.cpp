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

#include "simVis/Scenario.h"
#include "simVis/Platform.h"
#include "simVis/View.h"

#include "simQt/EntityTreeComposite.h"
#include "simQt/CenterEntity.h"

namespace simQt {


CenterEntity::CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, QObject* parent)
  : QObject(parent),
    focusManager_(focusManager),
    scenarioManager_(scenarioManager)
{
}

CenterEntity::CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, EntityTreeComposite& tree)
  : QObject(&tree),
    focusManager_(focusManager),
    scenarioManager_(scenarioManager)
{
  bindTo(tree);
}

CenterEntity::~CenterEntity()
{
}

void CenterEntity::bindTo(EntityTreeComposite& tree)
{
  connect(&tree, SIGNAL(itemDoubleClicked(uint64_t)), this, SLOT(centerOnEntity(uint64_t)));
  tree.setExpandsOnDoubleClick(false); /// Turns off the tree expansion on double click.
}

void CenterEntity::centerOnEntity(uint64_t id)
{
  simVis::EntityNode* node = scenarioManager_.find(id);

  // tetherCamera works only with platforms so work up the chain until a platform is found
  while ((node != NULL) && (dynamic_cast<simVis::PlatformNode*>(node) == NULL))
  {
    uint64_t parentId;
    if (node->getHostId(parentId))
      node = scenarioManager_.find(parentId);
    else
      node = NULL;
  }

  if ((node != NULL) && node->isActive() && node->isVisible())
  {
    if (focusManager_.getFocusedView() != NULL)
    {
      focusManager_.getFocusedView()->tetherCamera(node);
    }
  }
}


}

