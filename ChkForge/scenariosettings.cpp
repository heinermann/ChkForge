#include "scenariosettings.h"
#include "ui_scenariosettings.h"
#include "strings.h"

#include "abilitiestab.h"

#include <QMessageBox>

#include <set>
#include <algorithm>

ScenarioSettings::ScenarioSettings(QWidget* parent, int startTab) :
  QDialog(parent),
  ui(new Ui::ScenarioSettings)
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

ScenarioSettings::~ScenarioSettings()
{
  delete ui;
}

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
  settings.side = map->players.side->get();
  settings.colr = map->players.colr->get();
  settings.forc = map->players.forc->get();
  settings.ownr = map->players.ownr->get();
  settings.iown = map->players.iown->get();

  settings.unis = map->properties.unis->get();
  settings.unix = map->properties.unix->get();
  settings.puni = map->properties.puni->get();
  settings.upgs = map->properties.upgs->get();
  settings.upgx = map->properties.upgx->get();
  settings.upgr = map->properties.upgr->get();
  settings.pupx = map->properties.pupx->get();
  settings.tecs = map->properties.tecs->get();
  settings.tecx = map->properties.tecx->get();
  settings.ptec = map->properties.ptec->get();
  settings.ptex = map->properties.ptex->get();

  // TODO: Chk::Scope::Game is broken
  for (unsigned i = 0; i < Chk::TotalForces; ++i) {
    Chk::Force force = Chk::Force(i);
    bool useCustomForceName = map->strings.getForceNameStringId(force) != Chk::StringId::NoString;

    settings.useCustomForceNames[i] = useCustomForceName;
    if (useCustomForceName)
      settings.forceNames[i] = *map->strings.getForceName<RawString>(force);
  }

  for (unsigned i = 0; i < Sc::Unit::TotalTypes; ++i) {
    Sc::Unit::Type unitType = Sc::Unit::Type(i);
    bool useDefaultUnitName = map->strings.getUnitNameStringId(unitType) == Chk::StringId::NoString;

    settings.useDefaultUnitNames[i] = useDefaultUnitName;
    if (!useDefaultUnitName)
      settings.unitNames[i] = *map->strings.getUnitName<RawString>(unitType);
  }
  syncUiWithData();
}

void ScenarioSettings::writeToMap(std::shared_ptr<MapFile> map) const {
  map->players.side->get() = settings.side;
  map->players.colr->get() = settings.colr;
  map->players.forc->get() = settings.forc;
  map->players.ownr->get() = settings.ownr;
  map->players.iown->get() = settings.iown;

  map->properties.unis->get() = settings.unis;
  map->properties.unix->get() = settings.unix;
  map->properties.puni->get() = settings.puni;
  map->properties.upgs->get() = settings.upgs;
  map->properties.upgx->get() = settings.upgx;
  map->properties.upgr->get() = settings.upgr;
  map->properties.pupx->get() = settings.pupx;
  map->properties.tecs->get() = settings.tecs;
  map->properties.tecx->get() = settings.tecx;
  map->properties.ptec->get() = settings.ptec;
  map->properties.ptex->get() = settings.ptex;

  for (unsigned i = 0; i < Chk::TotalForces; ++i) {
    Chk::Force force = Chk::Force(i);
    if (settings.useCustomForceNames[i])
      map->strings.setForceName(force, RawString(settings.forceNames[i]));
    else
      map->strings.setForceNameStringId(force, Chk::StringId::NoString);
  }

  for (unsigned i = 0; i < Sc::Unit::TotalTypes; ++i) {
    Sc::Unit::Type unitType = Sc::Unit::Type(i);
    if (settings.useDefaultUnitNames[i])
      map->strings.setUnitNameStringId(unitType, Chk::StringId::NoString);
    else
      map->strings.setUnitName(unitType, RawString(settings.unitNames[i]));
  }
}

void ScenarioSettings::on_tabs_currentChanged(int index) {
  ScenarioSettingsTab* tab = qobject_cast<ScenarioSettingsTab*>(ui->tabs->widget(index));
  tab->setTabFocus();
}
