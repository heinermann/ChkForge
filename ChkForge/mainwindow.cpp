#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "itemtree.h"
#include "minimap.h"
#include "terrainbrush.h"
#include "mapview.h"

#include "about.h"
#include "newmap.h"

#include <DockAreaWidget.h>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QMdiArea>
#include <QMdiSubwindow>

#include "MapContext.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  createMdiDockArea();
  createTestMap();
  createToolWindows();
}

void MainWindow::createMdiDockArea()
{
  m_DockManager = new ads::CDockManager(this);
  m_DockManager->setStyleSheet("ads--CDockContainerWidget QSplitter::handle { background: palette(light); }");

  connect(mdi, &QMdiArea::subWindowActivated, this, &MainWindow::onMdiSubWindowActivated);

  mdi_dock->setWidget(mdi);
  mdi_dock->setFeature(ads::CDockWidget::DockWidgetFeature::DockWidgetClosable, false);
  mdi_dock->setFeature(ads::CDockWidget::DockWidgetFeature::DockWidgetFloatable, false);
  mdi_dock->setFeature(ads::CDockWidget::DockWidgetFeature::DockWidgetMovable, false);

  auto center_dock_area = m_DockManager->addDockWidget(ads::DockWidgetArea::CenterDockWidgetArea, mdi_dock);
  center_dock_area->setAllowedAreas(ads::DockWidgetArea::OuterDockAreas);
  center_dock_area->setHideSingleWidgetTitleBar(true);
}

void MainWindow::createTestMap()
{
  auto map = ChkForge::MapContext::create();
  map->load_map("C:/Program Files (x86)/StarCraft/Maps/(2)Bottleneck.scm");

  createMapView(map);
}

void MainWindow::createMapView(std::shared_ptr<ChkForge::MapContext> map)
{
  auto mapView = new MapView(map);
  auto subWindow = mdi->addSubWindow(mapView);
  subWindow->resize(QSize{ 640, 480 });
  mapView->showMaximized();

  connect(mapView, &MapView::aboutToClose, minimap, &Minimap::onCloseMapView);
}

void MainWindow::createToolWindows()
{
  // Create the left panel with minimap and other widgets
  ui->menu_Tool_Windows->addAction(minimap->toggleViewAction());
  ads::CDockAreaWidget* leftPane = m_DockManager->addDockWidget(ads::LeftDockWidgetArea, minimap);

  this->createToolWindow<ItemTree>(ads::BottomDockWidgetArea, leftPane);
  this->createToolWindow<TerrainBrush>(ads::BottomDockWidgetArea, leftPane);
}

void MainWindow::mapMenuActions()
{
  // Map menu actions
  connectTrigger(ui->action_view_toolwindows_showAll, std::bind(&MainWindow::toggleToolWindows, this, true));
  connectTrigger(ui->action_view_toolwindows_closeAll, std::bind(&MainWindow::toggleToolWindows, this, false));

  layerOptions = std::vector{
    ui->action_layer_selectBrush,
    ui->action_layer_terrain,
    ui->action_layer_doodads,
    ui->action_layer_sprites,
    ui->action_layer_units,
    ui->action_layer_locations,
    ui->action_layer_fog
  };

  for (QAction* layerAction : layerOptions) {
    connect(layerAction, SIGNAL(triggered(bool)), this, SLOT(toggleLayer(bool)));
  }
}

void MainWindow::toggleToolWindows(bool isOpen)
{
  for (ads::CDockWidget* dockWidget : m_DockManager->dockWidgetsMap()) {
    if (dockWidget == mdi_dock) continue;
    dockWidget->toggleView(isOpen);
  }
}

MainWindow::~MainWindow()
{
  delete m_DockManager;
  delete ui;
}

void MainWindow::on_action_file_new_triggered()
{
  NewMap newMap(this);
  int result = newMap.exec();
  if (result != QDialog::Accepted) return;

  auto map = ChkForge::MapContext::create();
  map->new_map(newMap.tile_width, newMap.tile_height, static_cast<Sc::Terrain::Tileset>(newMap.tileset->getTilesetId()), newMap.brush, newMap.clutter);

  createMapView(map);
}

namespace {
  static const QString file_filter =
    QObject::tr("All Starcraft Maps") + " (*.scm *.scx *.rep);;" +
    QObject::tr("Vanilla Maps") + " (*.scm);;" +
    QObject::tr("Expansion Maps") + " (*.scx);;" +
    QObject::tr("Replays") + " (*.rep);;" +
    QObject::tr("All files") + " (*)";
}

void MainWindow::on_action_file_open_triggered()
{
  QString result = QFileDialog::getOpenFileName(this, QString(), QString(), file_filter);
  if (result.isEmpty()) return;

  auto map = ChkForge::MapContext::create();
  std::string file_str = result.toStdString();
  map->load_map(file_str);

  createMapView(map);
}

void MainWindow::on_action_file_save_triggered()
{
  // if (map was not saved before)
  on_action_file_saveAs_triggered();
}

void MainWindow::on_action_file_saveAs_triggered()
{
  QString result = QFileDialog::getSaveFileName(this, QString(), QString(), file_filter);
  if (result.isEmpty()) return;

  // TODO: Actual map save stuff
}

void MainWindow::on_action_file_saveMapImage_triggered()
{
}

void MainWindow::on_action_file_importSections_triggered()
{
}

void MainWindow::on_action_file_exportSections_triggered()
{
}

void MainWindow::on_action_edit_undo_triggered()
{
}

void MainWindow::on_action_edit_redo_triggered()
{
}

void MainWindow::on_action_edit_cut_triggered()
{
}

void MainWindow::on_action_edit_copy_triggered()
{
}

void MainWindow::on_action_edit_paste_triggered()
{
}

void MainWindow::on_action_edit_delete_triggered()
{
}

void MainWindow::on_action_edit_selectAll_triggered()
{
}

void MainWindow::on_action_edit_properties_triggered()
{
}

void MainWindow::on_action_view_gridSettings_triggered()
{
}

void MainWindow::on_action_view_toggle_snapToGrid_triggered(bool checked)
{
}

void MainWindow::on_action_view_toggle_showGrid_triggered(bool checked)
{
}

void MainWindow::on_action_view_toggle_showLocations_triggered(bool checked)
{
}

void MainWindow::on_action_view_cleanMap_triggered()
{
}

void MainWindow::on_action_view_toggle_showUnitSize_triggered(bool checked)
{
}

void MainWindow::on_action_view_toggle_showBuildingSize_triggered(bool checked)
{
}

void MainWindow::on_action_view_toggle_showSightRange_triggered(bool checked)
{
}

void MainWindow::on_action_view_toggle_showSeekAttackRange_triggered(bool checked)
{
}

void MainWindow::on_action_view_toggle_showCreep_triggered(bool checked)
{
}

void MainWindow::on_action_view_toggle_showPylonAura_triggered(bool checked)
{
}

void MainWindow::on_action_view_toggle_showAddonNydusLinkage_triggered(bool checked)
{
}

void MainWindow::on_action_view_toggle_showCollisions_triggered(bool checked)
{
}

void MainWindow::on_action_view_toolwindows_showAll_triggered()
{
}

void MainWindow::on_action_view_toolwindows_closeAll_triggered()
{
}

void MainWindow::on_action_tools_mapRevealers_triggered()
{
}

void MainWindow::on_action_tools_preferences_triggered()
{
}

void MainWindow::on_action_scenario_players_triggered()
{
}

void MainWindow::on_action_scenario_forces_triggered()
{
}

void MainWindow::on_action_scenario_sounds_triggered()
{
}

void MainWindow::on_action_scenario_triggers_triggered()
{
}

void MainWindow::on_action_scenario_briefings_triggered()
{
}

void MainWindow::on_action_scenario_strings_triggered()
{
}

void MainWindow::on_action_scenario_unitSettings_triggered()
{
}

void MainWindow::on_action_scenario_upgradeSettings_triggered()
{
}

void MainWindow::on_action_scenario_techSettings_triggered()
{
}

void MainWindow::on_action_scenario_description_triggered()
{
}

void MainWindow::on_action_layer_options_triggered()
{
}

void MainWindow::on_action_help_about_triggered()
{
  About(this).exec();
}

void MainWindow::on_action_test_play_triggered()
{
}

void MainWindow::on_action_test_advance1_triggered()
{
}

void MainWindow::on_action_test_reset_triggered()
{
}

void MainWindow::on_action_test_duplicate_triggered()
{
}

void MainWindow::on_action_window_newMapView_triggered()
{
  auto* currentView = mdi->currentSubWindow();
  if (currentView != nullptr) {
    createMapView(qobject_cast<MapView*>(currentView)->getMap());
  }
}

void MainWindow::on_action_window_closeMapView_triggered()
{
  if (mdi->currentSubWindow() != nullptr) {
    mdi->currentSubWindow()->close();
  }
}

void MainWindow::on_action_window_closeAllMapViews_triggered()
{
  mdi->closeAllSubWindows();
}

void MainWindow::on_action_window_cascade_triggered()
{
  mdi->cascadeSubWindows();
}

void MainWindow::on_action_window_tile_triggered()
{
  mdi->tileSubWindows();
}

void MainWindow::toggleLayer(bool checked)
{
  if (!checked) return;

  QAction* src = qobject_cast<QAction*>(sender());
  int layer_id = src->property("layer_id").value<int>();
  
  for (QAction* layerAction : layerOptions) {
    if (layerAction == src) continue;
    layerAction->setChecked(false);
  }

  // TODO: Actual Layer stuff
}

void MainWindow::onMdiSubWindowActivated(QMdiSubWindow* window)
{
  if (window == nullptr) {
    minimap->setActiveMapView(nullptr);
    return;
  }

  MapView* map = qobject_cast<MapView*>(window);
  minimap->setActiveMapView(map);

  // TODO: set for other tool windows (i.e. treeview)
}
