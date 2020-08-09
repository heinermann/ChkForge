#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiArea>
#include <QIcon>
#include <DockManager.h>

#include "ui_mainwindow.h"

#include "layers.h"
#include "minimap.h"
#include "MapContext.h"

#include "itemtree.h"

class MapView;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; class StatusBar; class toolbars; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  Ui::StatusBar* statusBar_ui;
  Ui::toolbars* toolbars_ui;

  QWidget statusBar_container;
  QWidget toolbars_container;

  Minimap* minimap = new Minimap();
  QMdiArea* mdi = new QMdiArea();

  ads::CDockManager* m_DockManager;
  ads::CDockWidget* mdi_dock = new ads::CDockWidget("");

  std::vector<QAction*> layerOptions;
  std::vector<QAction*> mapAvailableActions;
  std::vector<QAction*> contextSensitiveActions;

  bool is_changing_layer = false;
  bool is_changing_zoom = false;

  QIcon black_ico;
private:
  template <typename Func>
  void connectTrigger(QAction* action, const Func& method) {
    this->connect(action, &QAction::triggered, this, method);
  }

  void toggleToolWindows(bool isOpen);

  void createNewMap(int tileWidth, int tileHeight, Sc::Terrain::Tileset tileset, int brush, int clutter);
  void createToolbars();
  void createStatusBar();
  void createMdiDockArea();
  void createToolWindows();
  void mapMenuActions();

  void createMapView(std::shared_ptr<ChkForge::MapContext> map);
  MapView* currentMapView();

  virtual void closeEvent(QCloseEvent* event) override;

  void updateMenusEnabled(bool enabled);

  virtual void keyPressEvent(QKeyEvent* event) override;

private slots:
  void mapMouseMoved(const QPoint& pos);
  void zoomChanged(int value);

  void on_action_file_new_triggered();
  void on_action_file_open_triggered();
  void on_action_file_save_triggered();
  void on_action_file_saveAs_triggered();
  void on_action_file_saveMapImage_triggered();
  void on_action_file_importSections_triggered();
  void on_action_file_exportSections_triggered();
  void on_action_edit_undo_triggered();
  void on_action_edit_redo_triggered();
  void on_action_edit_cut_triggered();
  void on_action_edit_copy_triggered();
  void on_action_edit_paste_triggered();
  void on_action_edit_delete_triggered();
  void on_action_edit_selectAll_triggered();
  void on_action_edit_properties_triggered();
  void on_action_view_gridSettings_triggered();
  void on_action_view_toggle_snapToGrid_triggered(bool checked);
  void on_action_view_toggle_showGrid_triggered(bool checked);
  void on_action_view_toggle_showLocations_triggered(bool checked);
  void on_action_view_cleanMap_triggered();
  void on_action_view_toggle_showUnitSize_triggered(bool checked);
  void on_action_view_toggle_showBuildingSize_triggered(bool checked);
  void on_action_view_toggle_showSightRange_triggered(bool checked);
  void on_action_view_toggle_showSeekAttackRange_triggered(bool checked);
  void on_action_view_toggle_showCreep_triggered(bool checked);
  void on_action_view_toggle_showPylonAura_triggered(bool checked);
  void on_action_view_toggle_showAddonNydusLinkage_triggered(bool checked);
  void on_action_view_toggle_showCollisions_triggered(bool checked);
  void on_action_view_toolwindows_showAll_triggered();
  void on_action_view_toolwindows_closeAll_triggered();
  void on_action_tools_mapRevealers_triggered();
  void on_action_tools_preferences_triggered();
  void on_action_scenario_players_triggered();
  void on_action_scenario_forces_triggered();
  void on_action_scenario_sounds_triggered();
  void on_action_scenario_triggers_triggered();
  void on_action_scenario_briefings_triggered();
  void on_action_scenario_strings_triggered();
  void on_action_scenario_unitSettings_triggered();
  void on_action_scenario_upgradeSettings_triggered();
  void on_action_scenario_techSettings_triggered();
  void on_action_scenario_description_triggered();
  void on_action_layer_options_triggered();
  void on_action_help_about_triggered();
  void on_action_help_report_triggered();
  void on_action_test_play_triggered();
  void on_action_test_advance1_triggered();
  void on_action_test_reset_triggered();
  void on_action_test_duplicate_triggered();
  void on_action_window_newMapView_triggered();
  void on_action_window_closeMapView_triggered();
  void on_action_window_closeAllMapViews_triggered();
  void on_action_window_cascade_triggered();
  void on_action_window_tile_triggered();

  void selectLayerIndex(int index);
  void selectPlayerIndex(int index);
  void toggleLayer(bool checked);

  void onMdiSubWindowActivated(QMdiSubWindow* window);

  void onItemTreeChanged(ItemTree::Category category, int id);
};
#endif // MAINWINDOW_H
