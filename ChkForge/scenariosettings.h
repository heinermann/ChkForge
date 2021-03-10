#ifndef SCENARIOSETTINGS_H
#define SCENARIOSETTINGS_H

#include <QDialog>
#include <cstdint>
#include <array>

#include <MappingCoreLib/Chk.h>
#include <MappingCoreLib/Sc.h>
#include <MappingCoreLib/MapFile.h>

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

  void init();

  void readFromMap(const MapFile& map);
  void writeToMap(MapFile& map) const;

  void syncUiWithData();
  void updatePlayerTree();

  QString getForceName(unsigned force) const;

private:
  Ui::ScenarioSettings *ui;

  struct Settings {
    Chk::SIDE side; // Races
    Chk::COLR colr; // Player colors
    Chk::FORC forc; // Forces
    Chk::OWNR ownr; // Slot owners
    Chk::IOWN iown; // Redundant slot owners

    Chk::UNIS unis; // Unit settings
    Chk::UNIx unix; // Expansion Unit Settings
    Chk::PUNI puni; // Unit availability
    Chk::UPGS upgs; // Upgrade costs
    Chk::UPGx upgx; // Expansion upgrade costs
    Chk::UPGR upgr; // Upgrade leveling
    Chk::PUPx pupx; // Expansion upgrade leveling
    Chk::TECS tecs; // Technology costs
    Chk::TECx tecx; // Expansion technology costs
    Chk::PTEC ptec; // Technology availability
    Chk::PTEx ptex; // Expansion technology availability

    std::array<bool, Chk::TotalForces> useDefaultForceNames;
    std::array<std::string, Chk::TotalForces> forceNames;
    
    std::array<bool, Sc::Unit::TotalTypes> useDefaultUnitNames;
    std::array<std::string, Sc::Unit::TotalTypes> unitNames;
  };

  Settings settings;

  static void setSelectedButtonGroup(QButtonGroup* btnGroup, int id);
  static void clearSelectedButtonGroup(QButtonGroup* btnGroup);
  static int playerIdFrom(QTreeWidgetItem* itm);
private slots:
  void on_plyrList_itemSelectionChanged();
  void on_btnGroupController_idClicked(int id);
  void on_btnGroupRace_idClicked(int id);
  void on_btnGroupPlayerForce_idClicked(int id);
};

#endif // SCENARIOSETTINGS_H
