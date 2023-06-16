#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_statusbar.h"
#include "ui_toolbars.h"

#include "itemtree.h"
#include "minimap.h"
#include "terrainbrush.h"
#include "mapview.h"

#include "about.h"
#include "appsettings.h"
#include "newmap.h"
#include "scenariodescription.h"
#include "scenariosettings.h"
#include "strings.h"

#include <MappingCoreLib/Sc.h>

#include <DockAreaWidget.h>
#include <QLabel>
#include <QMessageBox>
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
#include <QLineEdit>
#include <QShortcut>

#include <filesystem>

#include "MapContext.h"
#include "language.h"
#include "OpenSave.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(std::make_unique<Ui::MainWindow>())
  , statusBar_ui(std::make_unique<Ui::StatusBar>())
  , toolbars_ui(std::make_unique<Ui::toolbars>())
  , scmd_pluginManager(this)
{
  QLocale lang = settings.value("language", QLocale()).toLocale();
  ChkForge::SetLanguage(lang);

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
  applyTranslations();
}

void MainWindow::applyTranslations() {
  // Pull Qt-standardized translations from QLineEdit

  //: DO NOT TRANSLATE
  ui->action_edit_cut->setText(QLineEdit::tr("Cu&t"));
  //: DO NOT TRANSLATE
  ui->action_edit_copy->setText(QLineEdit::tr("&Copy"));
  //: DO NOT TRANSLATE
  ui->action_edit_paste->setText(QLineEdit::tr("&Paste"));
  //: DO NOT TRANSLATE
  ui->action_edit_undo->setText(QLineEdit::tr("&Undo"));
  //: DO NOT TRANSLATE
  ui->action_edit_redo->setText(QLineEdit::tr("&Redo"));
  //: DO NOT TRANSLATE
  ui->action_edit_delete->setText(QLineEdit::tr("Delete"));
  //: DO NOT TRANSLATE
  ui->action_edit_selectAll->setText(QLineEdit::tr("Select All"));

  //: DO NOT TRANSLATE
  toolbars_ui->label_zoom->setText(QShortcut::tr("Zoom"));
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
    toolbars_ui->cmb_player->addItem(QIcon(pixmap), ChkForge::getGenericPlayerName(i));
  }

  connect(toolbars_ui->cmb_layer, &QComboBox::currentIndexChanged, this, &MainWindow::selectLayerIndex);
  connect(toolbars_ui->cmb_player, &QComboBox::currentIndexChanged, this, &MainWindow::selectPlayerIndex);
  connect(toolbars_ui->spn_zoom, &QSpinBox::valueChanged, this, &MainWindow::zoomChanged);
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
  m_DockManager = std::make_unique<ads::CDockManager>(this);
  m_DockManager->setStyleSheet(m_DockManager->styleSheet() + "\nads--CDockContainerWidget QSplitter::handle { background: palette(light); }");

  connect(mdi, &QMdiArea::subWindowActivated, this, &MainWindow::onMdiSubWindowActivated);

  // Do something about tabs since mdi windows are not very visible
  //mdi->setViewMode(QMdiArea::ViewMode::TabbedView);
  //mdi->setTabsMovable(true);
  //mdi->setTabsClosable(true);

  mdi_dock->setWidget(mdi);
  mdi_dock->setFeature(ads::CDockWidget::DockWidgetFeature::NoTab, true);
  mdi_dock->setFeature(ads::CDockWidget::DockWidgetFeature::DockWidgetClosable, false);
  mdi_dock->setFeature(ads::CDockWidget::DockWidgetFeature::DockWidgetFloatable, false);
  mdi_dock->setFeature(ads::CDockWidget::DockWidgetFeature::DockWidgetMovable, false);

  auto center_dock_area = m_DockManager->setCentralWidget(mdi_dock);
  center_dock_area->setAllowedAreas(ads::DockWidgetArea::OuterDockAreas);
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
  mapView->updateTitle();

  QIcon map_icon = map_filename.isEmpty() ? QIcon(":/icons/scx.png") : icon_provider.icon(QFileInfo(map_filename));
  mapView->setWindowIcon(map_icon);

  connect(mapView, &MapView::aboutToClose, minimap, &Minimap::onCloseMapView);
  connect(mapView, &MapView::setItemTreeSelectionSignal, itemTree, &ItemTree::set_item);
}

void MainWindow::createToolWindows()
{
  // Create the left panel with minimap and other widgets
  ui->menu_Tool_Windows->addAction(minimap->toggleViewAction());
  ads::CDockAreaWidget* leftPane = m_DockManager->addDockWidget(ads::LeftDockWidgetArea, minimap);

  itemTree = new ItemTree(this);
  ui->menu_Tool_Windows->addAction(itemTree->toggleViewAction());
  m_DockManager->addDockWidget(ads::BottomDockWidgetArea, itemTree, leftPane);

  TerrainBrush* terrainBrush = new TerrainBrush(this);
  ui->menu_Tool_Windows->addAction(terrainBrush->toggleViewAction());
  m_DockManager->addDockWidget(ads::BottomDockWidgetArea, terrainBrush, leftPane);

  connect(itemTree, &ItemTree::itemTreeChanged, this, &MainWindow::onItemTreeChanged);
}

void MainWindow::mapMenuActions()
{
  // Map menu actions
  connectTrigger(ui->action_view_toolwindows_showAll, [=] { this->toggleToolWindows(true); });
  connectTrigger(ui->action_view_toolwindows_closeAll, [=] { this->toggleToolWindows(false); });

  for (QAction* layerAction : layerOptions) {
    connect(layerAction, &QAction::triggered, this, &MainWindow::toggleLayer);
  }

  ui->action_help_about->setShortcuts(QKeySequence::HelpContents);

  ui->action_file_open->setShortcuts(QKeySequence::Open);
  ui->action_file_save->setShortcuts(QKeySequence::Save);
  ui->action_file_new->setShortcuts(QKeySequence::New);
  ui->action_file_exit->setShortcuts(QKeySequence::Quit);
  ui->action_file_saveMapImage->setShortcuts(QKeySequence::Print);

  ui->action_edit_delete->setShortcuts(QKeySequence::Delete);
  ui->action_edit_cut->setShortcuts(QKeySequence::Cut);
  ui->action_edit_copy->setShortcuts(QKeySequence::Copy);
  ui->action_edit_paste->setShortcuts(QKeySequence::Paste);
  ui->action_edit_undo->setShortcuts(QKeySequence::Undo);
  ui->action_edit_redo->setShortcuts(QKeySequence::Redo);
  ui->action_edit_selectAll->setShortcuts(QKeySequence::SelectAll);
  ui->action_edit_properties->setShortcut(QKeySequence(Qt::Key_Enter));

  ui->action_test_reset->setShortcuts({ QKeySequence(QKeySequence::Refresh), QKeyCombination(Qt::CTRL, Qt::Key_R) });
  ui->action_test_play->setShortcut(QKeySequence(Qt::Key_Space));
  ui->action_test_advance1->setShortcut(QKeyCombination(Qt::SHIFT, Qt::Key_Space));

  ui->action_window_newMapView->setShortcuts(QKeySequence::AddTab);

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
    ui->action_tools_stackUnits,
    ui->action_window_cascade,
    ui->action_window_closeAllMapViews,
    ui->action_window_closeMapView,
    ui->action_window_newMapView,
    ui->action_window_tile,
    ui->action_test_play,
    ui->action_test_duplicate,
  };

  contextSensitiveActions = std::vector<QAction*>{
    ui->action_edit_undo,
    ui->action_edit_redo,
    ui->action_edit_cut,
    ui->action_edit_copy,
    ui->action_edit_paste,
    ui->action_edit_delete,
    ui->action_test_advance1,
    ui->action_test_reset,
  };

  selectionActions = std::vector<QAction*>{
    ui->action_edit_cut,
    ui->action_edit_copy,
    ui->action_edit_delete,
    ui->action_tools_stackUnits,
  };
}

void MainWindow::initRecentFiles() {
  recent_files = settings.value("recentFiles").toStringList().mid(0, max_recent_files);
  resetRecentFileMenu();
}

void MainWindow::resetRecentFileMenu() {
  ui->menu_recentFiles->clear();
  for (const QString& file : recent_files) {
    ui->menu_recentFiles->addAction(file, this, &MainWindow::on_recent_file_triggered);
  }
}

void MainWindow::on_recent_file_triggered() {
  QAction* action = qobject_cast<QAction*>(sender());
  open_map(action->text().toStdString());
}

void MainWindow::addRecentFile(std::filesystem::path filename) {
  filename.make_preferred();

  QString str = QString::fromStdString(filename.string());
  recent_files.removeAll(str);
  recent_files.prepend(str);
  recent_files.removeDuplicates();
  recent_files = recent_files.mid(0, max_recent_files);

  settings.setValue("recentFiles", recent_files);
  resetRecentFileMenu();
}

void MainWindow::toggleToolWindows(bool isOpen)
{
  for (ads::CDockWidget* dockWidget : m_DockManager->dockWidgetsMap()) {
    if (dockWidget == mdi_dock) continue;
    dockWidget->toggleView(isOpen);
  }
}

MainWindow::~MainWindow() {}

void MainWindow::on_action_file_new_triggered()
{
  NewMap newMap(this);
  int result = newMap.exec();
  if (result != QDialog::Accepted) return;

  createNewMap(newMap.tile_width, newMap.tile_height, Sc::Terrain::Tileset(newMap.tileset->getTilesetId()), newMap.brush, newMap.clutter);
}

bool MainWindow::open_map(const std::filesystem::path& map_filename)
{
  if (map_filename.empty()) return false;

  auto map = ChkForge::MapContext::create();
  if (map->load_map(map_filename)) {
    createMapView(map);
    addRecentFile(map_filename);
    return true;
  }
  return false;
}

void MainWindow::on_action_file_open_triggered()
{
  auto path = OpenSave::getMapOpenFilename(this);

  open_map(path);
}

void MainWindow::on_action_file_save_triggered()
{
  if (!currentMap()->save()) {
    on_action_file_saveAs_triggered();
  }
}

void MainWindow::on_action_file_saveAs_triggered()
{
  auto map = currentMap();
  if (map == nullptr) return;

  auto path = OpenSave::getMapSaveFilename(QString::fromStdString(map->filepath()), this);
  if (path.empty()) return;

  map->saveAs(path);

  addRecentFile(path);
}

void MainWindow::on_action_file_saveMapImage_triggered()
{
}

void MainWindow::on_action_file_settings_triggered()
{
  AppSettings settingsUI(this);
  int result = settingsUI.exec();
  if (result != QDialog::Accepted) return;

  // Retranslate UI
  QLocale language = settingsUI.language();
  ChkForge::SetLanguage(language);

  ui->retranslateUi(this);
  toolbars_ui->retranslateUi(&toolbars_container);
  statusBar_ui->retranslateUi(&statusBar_container);
  applyTranslations();

  settings.setValue("language", language);

  // TODO Apply other global settings from dialog here
}

void MainWindow::on_action_file_importSections_triggered()
{
}

void MainWindow::on_action_file_exportSections_triggered()
{
}

void MainWindow::on_action_edit_undo_triggered()
{
  MapView* map = currentMapView();
  if (map) {
    map->getMap()->actions.undo();
    onUndoRedoUpdated();
  }
}

void MainWindow::on_action_edit_redo_triggered()
{
  MapView* map = currentMapView();
  if (map) {
    map->getMap()->actions.redo();
    onUndoRedoUpdated();
  }
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
  if (currentView) {
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

void MainWindow::on_action_tools_stackUnits_triggered()
{
}

void MainWindow::on_action_tools_preferences_triggered()
{
}

void MainWindow::launchScenarioSettings(int startTab) {
  if (currentMapView() == nullptr) return;
  auto map = currentMapView()->getMap();

  auto settings = std::make_unique<ScenarioSettings>(this, startTab);

  settings->readFromMap(map->chk);

  int result = settings->exec();
  if (result == QDialog::Accepted) {
    settings->writeToMap(map->chk);
  }
}

void MainWindow::on_action_scenario_players_triggered()
{
  launchScenarioSettings(ScenarioSettings::TAB_PLAYERS);
}

void MainWindow::on_action_scenario_forces_triggered()
{
  launchScenarioSettings(ScenarioSettings::TAB_FORCES);
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
  launchScenarioSettings(ScenarioSettings::TAB_UNITS);
}

void MainWindow::on_action_scenario_upgradeSettings_triggered()
{
  launchScenarioSettings(ScenarioSettings::TAB_UPGRADES);
}

void MainWindow::on_action_scenario_techSettings_triggered()
{
  launchScenarioSettings(ScenarioSettings::TAB_ABILITIES);
}

void MainWindow::on_action_scenario_description_triggered()
{
  auto map = currentMap();
  if (!map) return;

  ScenarioDescription scenarioDlg(this);
  RawString name, desc;
  name = desc = map->chk->getFileName();
  if (map->chk->strings.getScenarioNameStringId() != 0) {
    name = *map->chk->strings.getScenarioName<RawString>();
  }
  if (map->chk->strings.getScenarioDescriptionStringId() != 0) {
    desc = *map->chk->strings.getScenarioDescription<RawString>();
  }

  scenarioDlg.name = QString::fromUtf8(name.data(), name.size());
  scenarioDlg.description = QString::fromUtf8(desc.data(), desc.size());

  int result = scenarioDlg.exec();
  if (result != QDialog::Accepted) return;

  map->chk->strings.setScenarioName(RawString(scenarioDlg.name.toUtf8()));
  map->chk->strings.setScenarioDescription(RawString(scenarioDlg.description.toUtf8()));
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
  auto map = currentMap();
  if (!map) return;

  if (!map->is_testing()) {
    map->start_playback();
  }
  else {
    map->toggle_pause();
  }
  updatePlaybackState();
}

void MainWindow::updatePlaybackState() {
  auto map = currentMap();
  if (!map) return;

  bool is_editing = !map->is_testing();

  if (is_editing || map->is_paused()) {
    ui->action_test_play->setText(tr("&Play"));
    ui->action_test_play->setIcon(QIcon(":/themes/oxygen-icons-png/oxygen/48x48/actions/media-playback-start.png"));
  }
  else {
    ui->action_test_play->setText(tr("&Pause"));
    ui->action_test_play->setIcon(QIcon(":/themes/oxygen-icons-png/oxygen/48x48/actions/media-playback-pause.png"));
  }

  // TODO: deduplicate code?
  toolbars_ui->cmb_layer->setEnabled(is_editing);
  ui->menu_Layer->menuAction()->setVisible(is_editing);
  ui->menu_Edit->menuAction()->setVisible(is_editing);
  ui->menu_Tools->menuAction()->setVisible(is_editing);

  ui->action_test_advance1->setEnabled(map->is_testing() && map->is_paused());
  ui->action_test_reset->setEnabled(map->is_testing());
}

void MainWindow::on_action_test_advance1_triggered()
{
  auto map = currentMap();
  if (!map) return;

  map->frame_advance();
}

void MainWindow::on_action_test_reset_triggered()
{
  auto map = currentMap();
  if (!map) return;

  map->stop_playback();
  updatePlaybackState();
}

void MainWindow::on_action_test_duplicate_triggered()
{
}

void MainWindow::on_action_window_newMapView_triggered()
{
  MapView* map = currentMapView();
  if (map) createMapView(map->getMap());
}

void MainWindow::on_action_window_closeMapView_triggered()
{
  QMdiSubWindow* map = mdi->currentSubWindow();
  if (map) map->close();
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
    auto player_text = ChkForge::getGenericPlayerName(index);
    cmb_player->addItem(black_ico, player_text);
    cmb_player->setCurrentIndex(12);
    statusBar_ui->player_color->setPixmap(black_ico.pixmap(16, 16));
    statusBar_ui->lbl_player->setText(player_text);
  }
  
  MapView* map = currentMapView();
  if (map != nullptr) {
    map->getMap()->set_player(index);
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
  disconnect(this, SLOT(onUndoRedoUpdated));
  disconnect(toolbars_ui->spn_zoom, SLOT(setValue(int)));

  updateMenusEnabled(true);
  onUndoRedoUpdated();
  toolbars_ui->spn_zoom->setValue(map->getViewScale() * 100);

  connect(map, SIGNAL(mouseMove(const QPoint&)), this, SLOT(mapMouseMoved(const QPoint&)));
  connect(&*map->getMap(), &ChkForge::MapContext::triggerUndoRedoChanged, this, &MainWindow::onUndoRedoUpdated);
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

  scmd_pluginManager.setTrackingMap(map->getMap());

  this->itemTree->update_tileset(map->getMap()->tileset());
}

void MainWindow::mapMouseMoved(const QPoint& pos)
{
  QPoint map_pos = currentMapView()->pointToMap(pos);
  statusBar_ui->lbl_coordinates->setText(QString("%1, %2 (%3, %4)").arg(map_pos.x()).arg(map_pos.y()).arg(map_pos.x() / 32).arg(map_pos.y() / 32));
}

MapView* MainWindow::currentMapView() {
  return qobject_cast<MapView*>(mdi->currentSubWindow());
}

std::shared_ptr<ChkForge::MapContext> MainWindow::currentMap() {
  MapView* map = currentMapView();
  if (map == nullptr) return nullptr;
  return map->getMap();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  auto subwindows = mdi->subWindowList(QMdiArea::StackingOrder);
  std::reverse(subwindows.begin(), subwindows.end());

  for (auto window : subwindows) {
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
  auto map = currentMap();
  if (map == nullptr) return;

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

  ui->menu_Edit->menuAction()->setVisible(enabled);
  ui->menu_View->menuAction()->setVisible(enabled);
  ui->menu_Layer->menuAction()->setVisible(enabled);
  ui->menu_Scenario->menuAction()->setVisible(enabled);
  ui->menu_Test->menuAction()->setVisible(enabled);
  ui->menu_Tools->menuAction()->setVisible(enabled);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
  event->ignore();

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
    event->accept();
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
      std::filesystem::path path = url.toLocalFile().toStdString();
      if (open_map(path)) {
        event->acceptProposedAction();
      }
    }
  }
}

void MainWindow::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);

  scmd_pluginManager.loadPlugins();

  auto newOptions = scmd_pluginManager.addMenuOptions(ui->menu_Tools);
  mapAvailableActions.insert(mapAvailableActions.end(), newOptions.begin(), newOptions.end());

  updateMenusEnabled(false);
}

void MainWindow::onUndoRedoUpdated()
{
  MapView* map = currentMapView();
  if (map == nullptr) {
    ui->action_edit_undo->setEnabled(false);
    ui->action_edit_redo->setEnabled(false);
    return;
  }

  ui->action_edit_undo->setEnabled(map->getMap()->actions.hasUndo());
  ui->action_edit_redo->setEnabled(map->getMap()->actions.hasRedo());
}
