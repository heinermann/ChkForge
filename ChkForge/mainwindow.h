#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiArea>
#include <DockManager.h>

#include "ui_mainwindow.h"

#include "layers.h"

class MapView;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow *ui;

  ads::CDockManager* m_DockManager;
  QMdiArea* mdi = new QMdiArea();
  ads::CDockWidget* mdi_dock = new ads::CDockWidget("");

  template <class T>
  ads::CDockAreaWidget* createToolWindow(ads::DockWidgetArea dockWidgetArea, ads::CDockAreaWidget* areaWidget = nullptr) {
    T* widget = new T();
    ui->menu_Tool_Windows->addAction(widget->toggleViewAction());
    return m_DockManager->addDockWidget(dockWidgetArea, widget, areaWidget);
  }

  template <typename Func>
  void connectTrigger(QAction* action, const Func& method) {
    this->connect(action, &QAction::triggered, this, method);
  }

  void toggleToolWindows(bool isOpen);

  std::vector<QAction*> layerOptions;
  ChkForge::Layer currentLayer = ChkForge::LAYER_SELECT;

private slots:
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
  void on_action_test_play_triggered();
  void on_action_test_advance1_triggered();
  void on_action_test_reset_triggered();
  void on_action_test_duplicate_triggered();

  void toggleLayer(bool checked);

};
#endif // MAINWINDOW_H
