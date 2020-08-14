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
#include <QDesktopServices>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QMimeData>

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

  QPixmap black_pixmap{ 16,16 };
  black_pixmap.fill(Qt::GlobalColor::black);
  black_ico = QIcon(black_pixmap);

  createStatusBar();
  createMdiDockArea();
  createToolWindows();
  mapMenuActions();
  createToolbars();
  initRecentFiles();

  selectLayerIndex(0);
  selectPlayerIndex(0);

  updateMenusEnabled(false);

  //createNewMap(128, 128, Sc::Terrain::Tileset::Badlands, 2, 5);
}

namespace {
  QRgb default_player_color[16] = {
    qRgb(244,4,4), // 111
    qRgb(12,72,204), // 165
    qRgb(44,180,148), // 159
    qRgb(136,64,156), // 164
    qRgb(248,140,20), // 156
    qRgb(112,48,20), // 19
    qRgb(204,224,208), // 84
    qRgb(252,252,56), // 135
    qRgb(8,128,8), // 185
    qRgb(252,252,124), // 136
    qRgb(236,196,176), // 134
    qRgb(64,104,212), // 51
    qRgb(116,164,124), // 77
    qRgb(144,144,184), // 154
    qRgb(0,228,252), // 128
    qRgb(0,0,0) // default
  };
}
void MainWindow::createToolbars() {
  toolbars_ui->setupUi(&toolbars_container);

  ui->layer_toolbar->addWidget(toolbars_ui->layer_toolbar);
  ui->zoom_toolbar->addWidget(toolbars_ui->zoom_toolbar);

  for (QAction* layer : layerOptions) {
    toolbars_ui->cmb_layer->addItem(layer->icon(), layer->iconText(), QVariant::fromValue(layer));
  }

  for (int i = 0; i < Sc::Player::Total; ++i) {
    auto pixmap = QPixmap(16, 16);
    pixmap.fill(default_player_color[i]);
    toolbars_ui->cmb_player->addItem(QIcon(pixmap), tr("Player %1").arg(i + 1));
  }

  connect(toolbars_ui->cmb_layer, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::selectLayerIndex);
  connect(toolbars_ui->cmb_player, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::selectPlayerIndex);
  connect(toolbars_ui->spn_zoom, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::zoomChanged);
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
  static QFileIconProvider icon_provider{};

  auto mapView = new MapView(map);
  mdi->addSubWindow(mapView);
  mapView->showMaximized();

  QString map_filename = QString::fromStdString(map->filename());
  mapView->setWindowTitle(map_filename);

  QIcon map_icon = map_filename.isEmpty() ? QIcon(":/icons/scx.png") : icon_provider.icon(QFileInfo(map_filename));
  mapView->setWindowIcon(map_icon);

  connect(mapView, &MapView::aboutToClose, minimap, &Minimap::onCloseMapView);
}

void MainWindow::createToolWindows()
{
  // Create the left panel with minimap and other widgets
  ui->menu_Tool_Windows->addAction(minimap->toggleViewAction());
  ads::CDockAreaWidget* leftPane = m_DockManager->addDockWidget(ads::LeftDockWidgetArea, minimap);

  ItemTree* itemTree = new ItemTree();
  ui->menu_Tool_Windows->addAction(itemTree->toggleViewAction());
  m_DockManager->addDockWidget(ads::BottomDockWidgetArea, itemTree, leftPane);

  TerrainBrush* terrainBrush = new TerrainBrush();
  ui->menu_Tool_Windows->addAction(terrainBrush->toggleViewAction());
  m_DockManager->addDockWidget(ads::BottomDockWidgetArea, terrainBrush, leftPane);

  connect(itemTree, &ItemTree::itemTreeChanged, this, &MainWindow::onItemTreeChanged);
}

void MainWindow::mapMenuActions()
{
  // Map menu actions
  connectTrigger(ui->action_view_toolwindows_showAll, std::bind(&MainWindow::toggleToolWindows, this, true));
  connectTrigger(ui->action_view_toolwindows_closeAll, std::bind(&MainWindow::toggleToolWindows, this, false));

  for (QAction* layerAction : layerOptions) {
    connect(layerAction, &QAction::triggered, this, &MainWindow::toggleLayer);
  }

  mapAvailableActions = std::vector<QAction*>{
    ui->action_file_save,
    ui->action_file_saveAs,
    ui->action_file_importSections,
    ui->action_file_exportSections,
    ui->action_file_saveMapImage,
    ui->action_edit_selectAll,
    ui->action_edit_properties,
    ui->action_view_gridSettings,
    ui->action_view_cleanMap,
    ui->action_view_toggle_showAddonNydusLinkage,
    ui->action_view_toggle_showBuildingSize,
    ui->action_view_toggle_showCollisions,
    ui->action_view_toggle_showCreep,
    ui->action_view_toggle_showGrid,
    ui->action_view_toggle_showLocations,
    ui->action_view_toggle_showPylonAura,
    ui->action_view_toggle_showSeekAttackRange,
    ui->action_view_toggle_showSightRange,
    ui->action_view_toggle_showUnitSize,
    ui->action_view_toggle_snapToGrid,
    ui->action_tools_mapRevealers,
    ui->action_window_cascade,
    ui->action_window_closeAllMapViews,
    ui->action_window_closeMapView,
    ui->action_window_newMapView,
    ui->action_window_tile,
    ui->action_test_play,
    ui->action_test_duplicate,
    ui->action_test_advance1,
    ui->action_test_reset
  };

  contextSensitiveActions = std::vector<QAction*>{
    ui->action_edit_undo,
    ui->action_edit_redo,
    ui->action_edit_cut,
    ui->action_edit_copy,
    ui->action_edit_paste,
    ui->action_edit_delete
  };
}

void MainWindow::initRecentFiles() {
  recent_files = settings.value("recentFiles").toStringList().mid(0, max_recent_files);
  resetRecentFileMenu();
}

void MainWindow::resetRecentFileMenu() {
  ui->menu_recentFiles->clear();
  for (QString& file : recent_files) {
    ui->menu_recentFiles->addAction(file, this, &MainWindow::on_recent_file_triggered)->setParent(ui->menu_recentFiles);
  }
}

void MainWindow::on_recent_file_triggered() {
  QAction* action = qobject_cast<QAction*>(sender());
  open_map(action->text());
}

void MainWindow::addRecentFile(const QString& filename) {
  recent_files.removeAll(filename);
  recent_files.prepend(filename);
  recent_files.removeDuplicates();
  recent_files = recent_files.mid(0, max_recent_files);

  settings.setValue("recentFiles", QVariant{ recent_files });

  resetRecentFileMenu();
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

bool MainWindow::open_map(const QString& map_filename)
{
  if (map_filename.isEmpty()) return false;

  auto map = ChkForge::MapContext::create();
  std::string file_str = map_filename.toStdString();
  if (map->load_map(file_str)) {
    createMapView(map);
    addRecentFile(map_filename);
    return true;
  }
  return false;
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

  open_map(result);
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

  addRecentFile(result);
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

void MainWindow::on_action_help_report_triggered()
{
  QDesktopServices::openUrl(QUrl("https://github.com/heinermann/ChkForge/issues"));
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

  statusBar_ui->layer_icon->setPixmap(layerOptions[index]->icon().pixmap(16, 16));
  statusBar_ui->lbl_layer->setText(layerOptions[index]->iconText());

  MapView* mapView = currentMapView();
  if (mapView != nullptr) {
    mapView->getMap()->set_layer(ChkForge::Layer_t(index));
  }

  is_changing_layer = false;
}

void MainWindow::toggleLayer(bool checked)
{
  selectLayerIndex(sender()->property("layer_id").value<int>());
}

void MainWindow::selectPlayerIndex(int index) {

  auto cmb_player = toolbars_ui->cmb_player;

  if (cmb_player->count() > 12) {
    cmb_player->removeItem(12);
  }

  if (index < cmb_player->count()) {
    cmb_player->setCurrentIndex(index);
    statusBar_ui->player_color->setPixmap(cmb_player->itemIcon(index).pixmap(16, 16));
    statusBar_ui->lbl_player->setText(cmb_player->itemText(index));
  }
  else {
    auto player_text = tr("Player %1").arg(index + 1);
    cmb_player->addItem(black_ico, player_text);
    cmb_player->setCurrentIndex(12);
    statusBar_ui->player_color->setPixmap(black_ico.pixmap(16, 16));
    statusBar_ui->lbl_player->setText(player_text);
  }
}


void MainWindow::onMdiSubWindowActivated(QMdiSubWindow* window)
{
  if (window == nullptr) {
    minimap->setActiveMapView(nullptr);
    statusBar_ui->lbl_coordinates->setText("");
    updateMenusEnabled(false);
    return;
  }

  MapView* map = qobject_cast<MapView*>(window);
  minimap->setActiveMapView(map);

  disconnect(this, SLOT(mapMouseMoved(const QPoint&)));
  disconnect(toolbars_ui->spn_zoom, SLOT(setValue(int)));

  updateMenusEnabled(true);
  toolbars_ui->spn_zoom->setValue(map->getViewScale() * 100);

  connect(map, SIGNAL(mouseMove(const QPoint&)), this, SLOT(mapMouseMoved(const QPoint&)));
  connect(map, SIGNAL(scaleChangedPercent(int)), toolbars_ui->spn_zoom, SLOT(setValue(int)));

  // Update player colours
  for (int i = 0; i < 8; ++i) {
    QRgb player_color = map->getMap()->player_color(i);
    auto pixmap = QPixmap(16, 16);
    pixmap.fill(player_color);

    toolbars_ui->cmb_player->setItemIcon(i, QIcon(pixmap));
  }
  selectPlayerIndex(toolbars_ui->cmb_player->currentIndex());

  // Update layer to whatever the map has selected
  this->selectLayerIndex(map->getMap()->get_layer()->getLayerId());
}

void MainWindow::mapMouseMoved(const QPoint& pos)
{
  QPoint map_pos = currentMapView()->pointToMap(pos);
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

void MainWindow::zoomChanged(int value)
{
  if (is_changing_zoom) return;
  is_changing_zoom = true;
  MapView* map = currentMapView();
  if (map) {
    map->setViewScalePercent(value);
  }
  is_changing_zoom = false;
}

void MainWindow::onItemTreeChanged(ItemTree::Category category, int id)
{
  MapView* mapView = currentMapView();
  if (mapView == nullptr) return;

  auto map = mapView->getMap();

  switch (category) {
    case ItemTree::CAT_NONE:
      break;
    case ItemTree::CAT_TERRAIN:
      selectLayerIndex(ChkForge::Layer_t::LAYER_TERRAIN);
      break;
    case ItemTree::CAT_DOODAD:
      selectLayerIndex(ChkForge::Layer_t::LAYER_DOODAD);
      break;
    case ItemTree::CAT_UNIT:
      selectLayerIndex(ChkForge::Layer_t::LAYER_UNIT);
      map->set_layer_unit_type(Sc::Unit::Type(id));
      break;
    case ItemTree::CAT_SPRITE:
      selectLayerIndex(ChkForge::Layer_t::LAYER_SPRITE);
      map->set_layer_sprite_type(Sc::Sprite::Type(id));
      break;
    case ItemTree::CAT_UNITSPRITE:
      selectLayerIndex(ChkForge::Layer_t::LAYER_SPRITE);
      map->set_layer_sprite_unit_type(Sc::Unit::Type(id));
      break;
    case ItemTree::CAT_LOCATION:
      selectLayerIndex(ChkForge::Layer_t::LAYER_LOCATION);
      break;
    case ItemTree::CAT_BRUSH:
      selectLayerIndex(ChkForge::Layer_t::LAYER_SELECT);
      break;
  }
}

void MainWindow::updateMenusEnabled(bool enabled)
{
  for (QAction* action : mapAvailableActions) {
    action->setEnabled(enabled);
  }

  toolbars_ui->cmb_layer->setEnabled(enabled);
  toolbars_ui->cmb_player->setEnabled(enabled);
  toolbars_ui->spn_zoom->setEnabled(enabled);

  if (!enabled) {
    for (QAction* action : contextSensitiveActions) {
      action->setEnabled(enabled);
    }
  }

  ui->menu_Layer->menuAction()->setVisible(enabled);
  ui->menu_Scenario->menuAction()->setVisible(enabled);
  ui->menu_Test->menuAction()->setVisible(enabled);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
  static const std::map<int, int> key_player_map = {
    {Qt::Key::Key_1, 0},
    {Qt::Key::Key_2, 1},
    {Qt::Key::Key_3, 2},
    {Qt::Key::Key_4, 3},
    {Qt::Key::Key_5, 4},
    {Qt::Key::Key_6, 5},
    {Qt::Key::Key_7, 6},
    {Qt::Key::Key_8, 7},
    {Qt::Key::Key_9, 8},
    {Qt::Key::Key_0, 9},
    {Qt::Key::Key_Minus, 10},
    {Qt::Key::Key_Equal, 11}
  };

  if (event->modifiers() == Qt::NoModifier && key_player_map.count(event->key()) > 0) {
    this->selectPlayerIndex(key_player_map.at(event->key()));
    event->accept();
  }
  else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key::Key_0) {
    MapView* map = currentMapView();
    if (map) map->setViewScalePercent(100);
  }
}

bool MainWindow::isValidFormat(QString filename) const
{
  static const QStringList valid_formats = { ".chk", ".scm", ".scx", ".rep" };
  for (auto& fmt : valid_formats) {
    if (filename.endsWith(fmt))
      return true;
  }
  return false;
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
  if (!event->mimeData()->hasUrls()) return;

  for (auto& url : event->mimeData()->urls()) {
    if (isValidFormat(url.toLocalFile())) {
      event->acceptProposedAction();
      return;
    }
  }
}

void MainWindow::dropEvent(QDropEvent* event)
{
  if (!event->mimeData()->hasUrls()) return;

  for (auto& url : event->mimeData()->urls()) {
    if (isValidFormat(url.toLocalFile())) {
      if (open_map(url.toLocalFile())) {
        event->acceptProposedAction();
      }
    }
  }
}
