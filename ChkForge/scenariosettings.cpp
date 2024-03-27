#include "scenariosettings.h"
#include "ui_scenariosettings.h"
#include "strings.h"

#include "abilitiestab.h"

#include <QMessageBox>

#include <set>
#include <algorithm>

ScenarioSettings::ScenarioSettings(QWidget* parent, int startTab) :
  QDialog(parent),
  ui(std::make_unique<Ui::ScenarioSettings>())
{
  ui->setupUi(this);

  ui->playersTab->settings = &settings;
  ui->forcesTab->settings = &settings;

  ui->playersTab->setupDataMappers();
  ui->forcesTab->setupDataMappers();

  connect(ui->playersTab, &PlayersTab::updateData, ui->forcesTab, &ForcesTab::updateForcesTree);

  ui->tabs->setCurrentIndex(startTab);
  on_tabs_currentChanged(startTab);
}

ScenarioSettings::~ScenarioSettings() {}

QString ScenarioSettings::getForceName(unsigned force) const {
  if (force >= 4 || !settings.useCustomForceNames[force]) {
    return ChkForge::getDefaultForceName(force);
  }
  return QString::fromStdString(settings.forceNames[force]);
}

void ScenarioSettings::syncUiWithData() {
  // Also calls updateForcesTree()
  ui->playersTab->updatePlayerTree();
}

void ScenarioSettings::readFromMap(const std::shared_ptr<MapFile> map) {
  for (size_t i=0; i<Sc::Player::Total; ++i)
    settings.side.playerRaces[i] = map->playerRaces[i];

  for (size_t i=0; i<Sc::Player::TotalSlots; ++i)
    settings.colr.playerColor[i] = map->playerColors[i];

  settings.forc = map->forces;

  for (size_t i=0; i<Sc::Player::Total; ++i)
  {
    settings.ownr.slotType[i] = map->slotTypes[i];
    settings.iown.slotType[i] = map->iownSlotTypes[i];
  }

  settings.unis = map->origUnitSettings;
  settings.unix = map->unitSettings;
  settings.puni = map->unitAvailability;
  settings.upgs = map->origUpgradeCosts;
  settings.upgx = map->upgradeCosts;
  settings.upgr = map->origUpgradeLeveling;
  settings.pupx = map->upgradeLeveling;
  settings.tecs = map->origTechnologyCosts;
  settings.tecx = map->techCosts;
  settings.ptec = map->origTechnologyAvailability;
  settings.ptex = map->techAvailability;

  // TODO: Chk::Scope::Game is broken
  for (unsigned i = 0; i < Chk::TotalForces; ++i) {
    Chk::Force force = Chk::Force(i);
    bool useCustomForceName = map->getForceNameStringId(force) != Chk::StringId::NoString;

    settings.useCustomForceNames[i] = useCustomForceName;
    if (useCustomForceName)
      settings.forceNames[i] = *map->getForceName<RawString>(force);
  }

  for (unsigned i = 0; i < Sc::Unit::TotalTypes; ++i) {
    Sc::Unit::Type unitType = Sc::Unit::Type(i);
    bool useDefaultUnitName = map->getUnitNameStringId(unitType) == Chk::StringId::NoString;

    settings.useDefaultUnitNames[i] = useDefaultUnitName;
    if (!useDefaultUnitName)
      settings.unitNames[i] = *map->getUnitName<RawString>(unitType);
  }
  syncUiWithData();
}

void ScenarioSettings::writeToMap(std::shared_ptr<MapFile> map) const {
  for (size_t i=0; i<Sc::Player::Total; ++i)
    map->playerRaces[i] = settings.side.playerRaces[i];

  for (size_t i=0; i<Sc::Player::TotalSlots; ++i)
    map->playerColors[i] = settings.colr.playerColor[i];

  map->forces = settings.forc;

  for (size_t i=0; i<Sc::Player::Total; ++i)
  {
    map->slotTypes[i] = settings.ownr.slotType[i];
    map->iownSlotTypes[i] = settings.iown.slotType[i];
  }

  map->origUnitSettings = settings.unis;
  map->unitSettings = settings.unix;
  map->unitAvailability = settings.puni;
  map->origUpgradeCosts = settings.upgs;
  map->upgradeCosts = settings.upgx;
  map->origUpgradeLeveling = settings.upgr;
  map->upgradeLeveling = settings.pupx;
  map->origTechnologyCosts = settings.tecs;
  map->techCosts = settings.tecx;
  map->origTechnologyAvailability = settings.ptec;
  map->techAvailability = settings.ptex;

  for (unsigned i = 0; i < Chk::TotalForces; ++i) {
    Chk::Force force = Chk::Force(i);
    if (settings.useCustomForceNames[i])
      map->setForceName(force, RawString(settings.forceNames[i]));
    else
      map->setForceNameStringId(force, Chk::StringId::NoString);
  }

  for (unsigned i = 0; i < Sc::Unit::TotalTypes; ++i) {
    Sc::Unit::Type unitType = Sc::Unit::Type(i);
    if (settings.useDefaultUnitNames[i])
      map->setUnitNameStringId(unitType, Chk::StringId::NoString);
    else
      map->setUnitName(unitType, RawString(settings.unitNames[i]));
  }
}

void ScenarioSettings::on_tabs_currentChanged(int index) {
  ScenarioSettingsTab* tab = qobject_cast<ScenarioSettingsTab*>(ui->tabs->widget(index));
  tab->setTabFocus();
}
