#pragma once

#include <QDialog>
#include <QList>
#include <cstdint>
#include <array>
#include <memory>

#include <MappingCoreLib/Chk.h>
#include <MappingCoreLib/Sc.h>
#include <MappingCoreLib/MapFile.h>

#include "scenariosettingsdata.h"

namespace Ui {
  class ScenarioSettings;
}

class QTreeWidgetItem;
class QButtonGroup;

class ScenarioSettings : public QDialog
{
  Q_OBJECT

public:
  enum Tab {
    TAB_PLAYERS,
    TAB_FORCES,
    TAB_UNITS,
    TAB_UPGRADES,
    TAB_ABILITIES
  };

  explicit ScenarioSettings(QWidget *parent, int startTab);
  ~ScenarioSettings();

  void readFromMap(const std::shared_ptr<MapFile> map);
  void writeToMap(std::shared_ptr<MapFile> map) const;

  void syncUiWithData();

  QString getForceName(unsigned force) const;

private:
  std::unique_ptr<Ui::ScenarioSettings> ui;
  ScenarioSettingsData settings;

private slots:
  void on_tabs_currentChanged(int index);
};
