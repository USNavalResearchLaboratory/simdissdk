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
#include "simVis/CustomRendering.h"
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

#ifdef USE_DEPRECATED_SIMDISSDK_API
CenterEntity::CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, EntityTreeComposite& tree)
  : QObject(&tree),
    focusManager_(&focusManager),
    scenarioManager_(&scenarioManager)
{
  bindTo(tree);
}
#endif

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

#ifdef USE_DEPRECATED_SIMDISSDK_API
void CenterEntity::bindTo(EntityTreeComposite& tree)
{
  connect(&tree, SIGNAL(itemDoubleClicked(uint64_t)), this, SLOT(centerOnEntity(uint64_t)));
  tree.setExpandsOnDoubleClick(false); /// Turns off the tree expansion on double click.
}
#endif

void CenterEntity::centerOnEntity(uint64_t id)
{
  // Need the scenario and focus manager to continue
  if (!scenarioManager_ || !focusManager_)
    return;

  simVis::EntityNode* node = getViewCenterableNode(id);
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

simVis::EntityNode* CenterEntity::getViewCenterableNode(uint64_t id) const
{
  if (!scenarioManager_ || !focusManager_)
    return NULL;

  simVis::EntityNode* node = scenarioManager_->find(id);

  // tetherCamera works only with platforms, custom renderings and gates, so work up the chain until
  // a platform, custom rendering or gate is found.
  while ((node != NULL) && (dynamic_cast<simVis::PlatformNode*>(node) == NULL) &&
    (dynamic_cast<simVis::GateNode*>(node) == NULL) &&
    (dynamic_cast<simVis::CustomRenderingNode*>(node) == NULL))
  {
    uint64_t parentId;
    if (node->getHostId(parentId))
      node = scenarioManager_->find(parentId);
    else
      node = NULL;
  }

  return node;
}

//--------------------------------------------------------------------------------------------------

BindCenterEntityToEntityTreeComposite::BindCenterEntityToEntityTreeComposite(CenterEntity& centerEntity, EntityTreeComposite& tree, simData::DataStore& dataStore, QObject* parent)
  : QObject(parent),
    centerEntity_(centerEntity),
    tree_(tree),
    dataStore_(dataStore)
{
}

BindCenterEntityToEntityTreeComposite::~BindCenterEntityToEntityTreeComposite()
{
}

void BindCenterEntityToEntityTreeComposite::bind(bool centerOnDoubleClick)
{
  connect(&tree_, SIGNAL(rightClickMenuRequested()), this, SLOT(updateCenterEnable_()));
  connect(&tree_, SIGNAL(centerOnEntityRequested(uint64_t)), &centerEntity_, SLOT(centerOnEntity(uint64_t)));
  connect(&tree_, SIGNAL(centerOnSelectionRequested(QList<uint64_t>)), &centerEntity_, SLOT(centerOnSelection(QList<uint64_t>)));
  if (centerOnDoubleClick)
  {
    connect(&tree_, SIGNAL(itemDoubleClicked(uint64_t)), &centerEntity_, SLOT(centerOnEntity(uint64_t)));
    tree_.setExpandsOnDoubleClick(false); // Turns off the tree expansion on double click.
  }
}

void BindCenterEntityToEntityTreeComposite::updateCenterEnable_()
{
  QList<uint64_t> ids = tree_.selectedItems();
  if (ids.empty())
  {
    tree_.setUseCenterAction(false, "No entities selected");
    return;
  }

  for (auto it = ids.begin(); it != ids.end(); ++it)
  {
    auto node = centerEntity_.getViewCenterableNode(*it);
    if ((node == NULL) || !node->isActive() || !node->isVisible())
    {
      tree_.setUseCenterAction(false, "Inactive entity selected");
      return;
    }
  }

  tree_.setUseCenterAction(true);
}

}
