#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "itemtree.h"
#include "minimap.h"
#include "terrainbrush.h"
#include "mapview.h"

#include "newmap.h"

#include <DockAreaWidget.h>
#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  this->setStyleSheet("QMainWindow::separator{ width: 0px; height: 0px; }");

  // Create tool panels
  m_DockManager = new ads::CDockManager(this);

  Minimap* minimap = new Minimap();
  ui->menu_Tool_Windows->addAction(minimap->toggleViewAction());

  createMapView();

  ads::CDockAreaWidget* leftPane = m_DockManager->addDockWidget(ads::LeftDockWidgetArea, minimap);
  leftPane->setMaximumWidth(256);

  this->createToolWindow<ItemTree>(ads::BottomDockWidgetArea, leftPane);
  this->createToolWindow<TerrainBrush>(ads::BottomDockWidgetArea, leftPane);

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
  for (ads::CDockWidget* dockWidget : m_DockManager->dockWidgetsMap())
    dockWidget->toggleView(isOpen);
}

MapView* MainWindow::createMapView()
{
  MapView* widget = new MapView();
  ui->menu_Window->addAction(widget->toggleViewAction());

  m_DockManager->addDockWidget(ads::CenterDockWidgetArea, widget);

  widget->init();

  Minimap::g_minimap->setActiveMapView(widget);
  return widget;
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

  // TODO: Create new map (note: Need to pull data from ui-> in NewMap
}

void MainWindow::on_action_file_open_triggered()
{
}

void MainWindow::on_action_file_save_triggered()
{
}

void MainWindow::on_action_file_saveAs_triggered()
{
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

void MainWindow::on_action_file_exit_triggered()
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

void MainWindow::on_action_view_toggle_snapToGrid_triggered()
{
}

void MainWindow::on_action_view_toggle_showGrid_triggered()
{
}

void MainWindow::on_action_view_toggle_showLocations_triggered()
{
}

void MainWindow::on_action_view_cleanMap_triggered()
{
}

void MainWindow::on_action_view_toggle_showUnitSize_triggered()
{
}

void MainWindow::on_action_view_toggle_showBuildingSize_triggered()
{
}

void MainWindow::on_action_view_toggle_showSightRange_triggered()
{
}

void MainWindow::on_action_view_toggle_showSeekAttackRange_triggered()
{
}

void MainWindow::on_action_view_toggle_showCreep_triggered()
{
}

void MainWindow::on_action_view_toggle_showPylonAura_triggered()
{
}

void MainWindow::on_action_view_toggle_showAddonNydusLinkage_triggered()
{
}

void MainWindow::on_action_view_toggle_showCollisions_triggered()
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
