#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_statusbar.h"
#include "ui_toolbars.h"

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
#include <QStandardPaths>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QVariant>

#include "MapContext.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , statusBar_ui(new Ui::StatusBar)
  , toolbars_ui(new Ui::toolbars)
{
  ui->setupUi(this);

  layerOptions = std::vector{
    ui->action_layer_selectBrush,
    ui->action_layer_terrain,
    ui->action_layer_doodads,
    ui->action_layer_sprites,
    ui->action_layer_units,
    ui->action_layer_locations,
    ui->action_layer_fog
  };

  createStatusBar();
  createMdiDockArea();
  createToolWindows();
  mapMenuActions();
  createToolbars();

  createNewMap(128, 128, Sc::Terrain::Tileset::Badlands, 2, 5);
}

void MainWindow::createToolbars() {
  toolbars_ui->setupUi(&toolbars_container);

  ui->layer_toolbar->addWidget(toolbars_ui->layer_toolbar);
  ui->zoom_toolbar->addWidget(toolbars_ui->zoom_toolbar);

  for (QAction* layer : layerOptions) {
    toolbars_ui->cmb_layer->addItem(layer->icon(), layer->text().remove('&'), QVariant::fromValue(layer));
  }

  connect(toolbars_ui->cmb_layer, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::selectLayerIndex);
}

void MainWindow::createStatusBar()
{
  statusBar_ui->setupUi(&statusBar_container);
  ui->statusbar->addPermanentWidget(statusBar_ui->blank);
  ui->statusbar->addPermanentWidget(statusBar_ui->layer_widget);
  ui->statusbar->addPermanentWidget(statusBar_ui->player_widget);
  ui->statusbar->addPermanentWidget(statusBar_ui->lbl_coordinates);
}

void MainWindow::createMdiDockArea()
{
  m_DockManager = new ads::CDockManager(this);
  m_DockManager->setStyleSheet(m_DockManager->styleSheet() + "\nads--CDockContainerWidget QSplitter::handle { background: palette(light); }");

  connect(mdi, &QMdiArea::subWindowActivated, this, &MainWindow::onMdiSubWindowActivated);

  mdi_dock->setWidget(mdi);
  mdi_dock->setFeature(ads::CDockWidget::DockWidgetFeature::DockWidgetClosable, false);
  mdi_dock->setFeature(ads::CDockWidget::DockWidgetFeature::DockWidgetFloatable, false);
  mdi_dock->setFeature(ads::CDockWidget::DockWidgetFeature::DockWidgetMovable, false);

  auto center_dock_area = m_DockManager->addDockWidget(ads::DockWidgetArea::CenterDockWidgetArea, mdi_dock);
  center_dock_area->setAllowedAreas(ads::DockWidgetArea::OuterDockAreas);
  center_dock_area->setHideSingleWidgetTitleBar(true);
}

void MainWindow::createNewMap(int tileWidth, int tileHeight, Sc::Terrain::Tileset tileset, int brush, int clutter)
{
  auto map = ChkForge::MapContext::create();
  map->new_map(tileWidth, tileHeight, tileset, brush, clutter);
  createMapView(map);
}

void MainWindow::createMapView(std::shared_ptr<ChkForge::MapContext> map)
{
  auto mapView = new MapView(map);
  auto subWindow = mdi->addSubWindow(mapView);
  subWindow->resize(QSize{ 640, 480 });
  mapView->showMaximized();
  mapView->setWindowTitle(QString::fromStdString(map->filename()));

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

  createNewMap(newMap.tile_width, newMap.tile_height, Sc::Terrain::Tileset(newMap.tileset->getTilesetId()), newMap.brush, newMap.clutter);
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
  QString documents = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation).first();
  QString result = QFileDialog::getOpenFileName(this, QString(), documents + "/Starcraft/maps", file_filter);
  if (result.isEmpty()) return;

  auto map = ChkForge::MapContext::create();
  std::string file_str = result.toStdString();
  if (map->load_map(file_str)) {
    createMapView(map);
  }
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
  auto* currentView = currentMapView();
  if (currentView != nullptr) {
    currentView->getMap()->select_all();
  }
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
  auto* currentView = currentMapView();
  if (currentView != nullptr) {
    createMapView(currentView->getMap());
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

void MainWindow::selectLayerIndex(int index) {
  if (is_changing_layer) return;
  is_changing_layer = true;

  for (QAction* layerAction : layerOptions) {
    layerAction->setChecked(false);
  }
  layerOptions[index]->setChecked(true);

  toolbars_ui->cmb_layer->setCurrentIndex(index);

  // TODO: Actual Layer stuff

  is_changing_layer = false;
}

void MainWindow::toggleLayer(bool checked)
{
  selectLayerIndex(sender()->property("layer_id").value<int>());
}

void MainWindow::onMdiSubWindowActivated(QMdiSubWindow* window)
{
  if (window == nullptr) {
    minimap->setActiveMapView(nullptr);
    map_mouse_connection = std::nullopt;
    statusBar_ui->lbl_coordinates->setText("");
    return;
  }

  MapView* map = qobject_cast<MapView*>(window);
  minimap->setActiveMapView(map);

  if (map_mouse_connection) disconnect(*map_mouse_connection);
  map_mouse_connection = connect(map, &MapView::mouseMove, this, &MainWindow::mapMouseMoved);

  // TODO: set for other tool windows (i.e. treeview)
}

void MainWindow::mapMouseMoved(const QPoint& pos)
{
  QPoint map_pos = currentMapView()->getScreenPos() + pos;
  statusBar_ui->lbl_coordinates->setText(QString("%1, %2 (%3, %4)").arg(map_pos.x()).arg(map_pos.y()).arg(map_pos.x() / 32).arg(map_pos.y() / 32));
}

MapView* MainWindow::currentMapView() {
  return qobject_cast<MapView*>(mdi->currentSubWindow());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  auto subwindows = mdi->subWindowList(QMdiArea::StackingOrder).toStdList();
  subwindows.reverse();
  for (auto* window : subwindows) {
    if (!window->close()) {
      event->ignore();
      return;
    }
  }
  event->accept();
}
