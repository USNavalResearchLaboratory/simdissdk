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

#include "simVis/CentroidManager.h"
#include "simVis/Scenario.h"
#include "simVis/Platform.h"
#include "simVis/Gate.h"
#include "simVis/View.h"

#include "simQt/EntityTreeComposite.h"
#include "simQt/CenterEntity.h"

namespace simQt {


CenterEntity::CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, QObject* parent)
  : QObject(parent),
    focusManager_(&focusManager),
    scenarioManager_(&scenarioManager)
{
}

CenterEntity::CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, EntityTreeComposite& tree)
  : QObject(&tree),
    focusManager_(&focusManager),
    scenarioManager_(&scenarioManager)
{
  bindTo(tree);
}

CenterEntity::~CenterEntity()
{
}

void CenterEntity::centerOnSelection(const QList<uint64_t>& ids)
{
  if (ids.empty())
    return;
  // Use center entity if only one id is selected
  if (ids.size() == 1)
    centerOnEntity(ids[0]);
  else
  {
    // Need the centroid, scenario, and focus managers to continue
    if (!centroidManager_ || !scenarioManager_ || !focusManager_)
    {
      return;
    }

    // Create a centroid node about the selected ids
    std::vector<simVis::EntityNode*> nodes;
    // Get the entity nodes involved
    Q_FOREACH(uint64_t id, ids)
    {
      simVis::EntityNode* node = scenarioManager_->find<simVis::EntityNode>(id);
      if (node)
        nodes.push_back(node);
    }
    // Center the focused view on the given ids
    centroidManager_->centerViewOn(nodes, focusManager_->getFocusedView());
  }
}

void CenterEntity::bindTo(EntityTreeComposite& tree)
{
  connect(&tree, SIGNAL(itemDoubleClicked(uint64_t)), this, SLOT(centerOnEntity(uint64_t)));
  tree.setExpandsOnDoubleClick(false); /// Turns off the tree expansion on double click.
}

void CenterEntity::centerOnEntity(uint64_t id)
{
  // Need the scenario and focus manager to continue
  if (!scenarioManager_ || !focusManager_)
  {
    return;
  }

  simVis::EntityNode* node = scenarioManager_->find(id);

  // tetherCamera works only with platforms and gates, so work up the chain until
  // a platform or gate is found.
  while ((node != NULL) && (dynamic_cast<simVis::PlatformNode*>(node) == NULL) &&
     (dynamic_cast<simVis::GateNode*>(node) == NULL))
  {
    uint64_t parentId;
    if (node->getHostId(parentId))
      node = scenarioManager_->find(parentId);
    else
      node = NULL;
  }

  if ((node != NULL) && node->isActive() && node->isVisible())
  {
    if (focusManager_->getFocusedView() != NULL)
    {
      focusManager_->getFocusedView()->tetherCamera(node);
    }
  }
}

void CenterEntity::setCentroidManager(simVis::CentroidManager* centroidManager)
{
  centroidManager_ = centroidManager;
}

}
