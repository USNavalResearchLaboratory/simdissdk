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
#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QDockWidget>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPointer>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include "simCore/System/Utils.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "simUtil/ExampleResources.h"
#include "simQt/QWidgetNode.h"
#include "simQt/ViewerWidgetAdapter.h"
#include "simQt/HudTextBinManager.h"

/**
 * Main window that shows a viewer widget and several overlays on the HUD
 * that are generated from QImage, QLabel, and other QWidget instances.
 */
class MainWindow : public QMainWindow
{
public:
  explicit MainWindow(simVis::View* mainView, simQt::HudTextBinManager* hudTextBinManager, QWidget* parent = nullptr)
    : QMainWindow(parent),
    hudTextBinManager_(hudTextBinManager)
  {
    // UI Setup
    setWindowTitle("Qt HUD Label Bins Example");
    simQt::ViewerWidgetAdapter* viewerWidget = new simQt::ViewerWidgetAdapter(simQt::GlImplementation::Widget, this);
    viewerWidget->setViewer(mainView->getViewerBase());
    setCentralWidget(viewerWidget);

    // Create the dock widget
    QDockWidget* dockWidget = new QDockWidget("Text Control", this);
    QWidget* dockContent = new QWidget;
    QVBoxLayout* dockLayout = new QVBoxLayout(dockContent);

    // Text Input
    textInput_ = new QLineEdit;
    connect(textInput_, &QLineEdit::returnPressed, this, &MainWindow::addText_);
    dockLayout->addWidget(textInput_);

    // Bin Selection
    binSelection_ = new QComboBox;
    binSelection_->addItem("Top Left", static_cast<int>(simData::TextAlignment::ALIGN_LEFT_TOP));
    binSelection_->addItem("Center Left", static_cast<int>(simData::TextAlignment::ALIGN_LEFT_CENTER));
    binSelection_->addItem("Bottom Left", static_cast<int>(simData::TextAlignment::ALIGN_LEFT_BOTTOM));
    binSelection_->addItem("Top Center", static_cast<int>(simData::TextAlignment::ALIGN_CENTER_TOP));
    binSelection_->addItem("Center Center", static_cast<int>(simData::TextAlignment::ALIGN_CENTER_CENTER));
    binSelection_->addItem("Bottom Center", static_cast<int>(simData::TextAlignment::ALIGN_CENTER_BOTTOM));
    binSelection_->addItem("Top Right", static_cast<int>(simData::TextAlignment::ALIGN_RIGHT_TOP));
    binSelection_->addItem("Center Right", static_cast<int>(simData::TextAlignment::ALIGN_RIGHT_CENTER));
    binSelection_->addItem("Bottom Right", static_cast<int>(simData::TextAlignment::ALIGN_RIGHT_BOTTOM));
    dockLayout->addWidget(binSelection_);

    // Add Button
    QPushButton* addButton = new QPushButton("Add Text");
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addText_);
    dockLayout->addWidget(addButton);

    // Text Size Spin Box
    auto* textSizeSpinBox = new QSpinBox;
    textSizeSpinBox->setRange(4, 48); // Set a reasonable range for text size
    textSizeSpinBox->setValue(18);
    connect(textSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::setTextSize_);
    dockLayout->addWidget(new QLabel("Text Size:"));
    dockLayout->addWidget(textSizeSpinBox);

    // Color Button
    auto* colorButton = new QPushButton("Change Color");
    connect(colorButton, &QPushButton::clicked, this, &MainWindow::setColor_);
    dockLayout->addWidget(new QLabel("Text Color:"));
    dockLayout->addWidget(colorButton);

    // Text List
    textModel_ = new QStandardItemModel(this);
    textTree_ = new QTreeView;
    textTree_->setRootIsDecorated(false);
    textTree_->setSelectionBehavior(QAbstractItemView::SelectRows);
    textTree_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    textTree_->setModel(textModel_);
    textModel_->setHorizontalHeaderLabels({ "ID", "Text", "Position" });
    // Shrink columns 0 and 2
    textTree_->resizeColumnToContents(0);
    textTree_->resizeColumnToContents(2);
    dockLayout->addWidget(textTree_);

    // Remove Button
    removeButton_ = new QPushButton("Remove Selected");
    connect(removeButton_, &QPushButton::clicked, this, &MainWindow::removeText_);
    dockLayout->addWidget(removeButton_);

    dockContent->setLayout(dockLayout);
    dockWidget->setWidget(dockContent);
    addDockWidget(Qt::RightDockWidgetArea, dockWidget);

    // Menu and Actions
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QAction* exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);
    addAction(exitAction);
    fileMenu->addAction(exitAction);
  }

  void addText(simQt::HudTextBinManager::BinId boxId, const QString& text)
  {
    const int boxIndex = static_cast<int>(boxId);
    const simQt::HudTextBinManager::TextId textId = hudTextBinManager_->addText(boxId, text.toStdString());

    QStandardItem* idItem = new QStandardItem(QString::number(textId)); // ID
    QStandardItem* textItem = new QStandardItem(text); // Text
    QStandardItem* positionItem = new QStandardItem(binSelection_->itemText(boxIndex)); // Position
    idItem->setData(static_cast<qulonglong>(textId), Qt::UserRole); // Store ID for removal
    textModel_->appendRow({ idItem, textItem, positionItem });

    textInput_->clear();
  }

private:
  void addText_()
  {
    const int boxIndex = binSelection_->currentIndex();
    const simData::TextAlignment boxId = static_cast<simData::TextAlignment>(binSelection_->itemData(boxIndex).toInt());
    addText(boxId, textInput_->text());
  }

  void removeText_()
  {
    const QModelIndexList selected = textTree_->selectionModel()->selectedRows();
    if (selected.isEmpty())
      return;

    // Get the selected index
    const QModelIndex selectedIndex = selected.first();
    if (!selectedIndex.isValid())
      return;

    // Get the textId from the first column of the selected row
    const QModelIndex idIndex = textModel_->index(selectedIndex.row(), 0);
    const simQt::HudTextBinManager::TextId textId = idIndex.data(Qt::UserRole).toULongLong();

    // Remove the text from the HudTextBinManager and model
    hudTextBinManager_->removeText(textId);
    textModel_->removeRow(selectedIndex.row());
  }

  void setTextSize_(int size)
  {
    const int boxIndex = binSelection_->currentIndex();
    const simData::TextAlignment boxId = static_cast<simData::TextAlignment>(binSelection_->itemData(boxIndex).toInt());
    hudTextBinManager_->setTextSize(boxId, size);
  }

  void setColor_()
  {
    const int boxIndex = binSelection_->currentIndex();
    const simData::TextAlignment boxId = static_cast<simData::TextAlignment>(binSelection_->itemData(boxIndex).toInt());

    QColor color = QColorDialog::getColor(hudTextBinManager_->color(boxId), this, "Select Text Color");
    if (color.isValid())
    {
      hudTextBinManager_->setColor(boxId, color);
    }
  }

  osg::ref_ptr<simQt::HudTextBinManager> hudTextBinManager_;

  // UI elements
  QLineEdit* textInput_ = nullptr;
  QComboBox* binSelection_ = nullptr;
  QStandardItemModel* textModel_ = nullptr;
  QTreeView* textTree_ = nullptr;
  QPushButton* removeButton_ = nullptr;
};

int main(int argc, char* argv[])
{
  simCore::initializeSimdisEnvironmentVariables();
  simExamples::configureSearchPaths();
  QApplication app(argc, argv);

  // a Map and a Scene Manager:
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(simExamples::createDefaultExampleMap());
  simExamples::addDefaultSkyNode(sceneMan.get());

  // Views to embed in our widgets
  osg::ref_ptr<simVis::View> mainView = new simVis::View();
  mainView->setSceneManager(sceneMan.get());
  mainView->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  mainView->installBasicDebugHandlers();

  // The ViewManager coordinates the rendering of all our views.
  osg::ref_ptr<simVis::ViewManager> viewMan = new simVis::ViewManager();
  viewMan->addView(mainView.get());

  // Create the binned text manager and add it to the HUD
  osg::ref_ptr<simQt::HudTextBinManager> hudTextBinManager = new simQt::HudTextBinManager;
  mainView->getOrCreateHUD()->addChild(hudTextBinManager.get());

  MainWindow mainWindow(mainView.get(), hudTextBinManager.get());
  mainWindow.resize(1024, 768);
  mainWindow.show();

  // Add some text strings to the HUD
  mainWindow.addText(simData::TextAlignment::ALIGN_LEFT_TOP, "Short text at top-left.");
  mainWindow.addText(simData::TextAlignment::ALIGN_LEFT_CENTER,
    "This is a very long text string that should wrap around to multiple lines "
    "within the left-center bin.  This is to test word wrapping.");
  mainWindow.addText(simData::TextAlignment::ALIGN_CENTER_TOP, "Centered\nTop\nMultiple Lines");
  mainWindow.addText(simData::TextAlignment::ALIGN_CENTER_CENTER,
    "A medium length string in the center of the screen.");
  mainWindow.addText(simData::TextAlignment::ALIGN_CENTER_BOTTOM, "Bottom Center");
  mainWindow.addText(simData::TextAlignment::ALIGN_RIGHT_TOP, "Right Top\nShort");
  mainWindow.addText(simData::TextAlignment::ALIGN_RIGHT_CENTER,
    "A very long string on the right side of the screen to check word wrapping.");
  mainWindow.addText(simData::TextAlignment::ALIGN_RIGHT_BOTTOM, "Short text at bottom-right.");
  mainWindow.addText(simData::TextAlignment::ALIGN_RIGHT_BOTTOM, "Second line.");
  mainWindow.addText(simData::TextAlignment::ALIGN_RIGHT_BOTTOM, "Third line.");

  hudTextBinManager->setColor(Qt::white);
  hudTextBinManager->setTextSize(18);

  // Center top and center-center are yellow and larger
  hudTextBinManager->setColor(simData::TextAlignment::ALIGN_CENTER_TOP, Qt::yellow);
  hudTextBinManager->setTextSize(simData::TextAlignment::ALIGN_CENTER_TOP, 24);
  hudTextBinManager->setColor(simData::TextAlignment::ALIGN_CENTER_CENTER, Qt::yellow);
  hudTextBinManager->setTextSize(simData::TextAlignment::ALIGN_CENTER_CENTER, 24);

  return app.exec();
}
