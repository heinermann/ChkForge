#include "scenariosettings.h"
#include "ui_scenariosettings.h"
#include "strings.h"

#include <set>

ScenarioSettings::ScenarioSettings(QWidget* parent, int startTab) :
  QDialog(parent),
  ui(new Ui::ScenarioSettings)
{
  ui->setupUi(this);

  ui->tabs->setCurrentIndex(startTab);
  init();
}

void ScenarioSettings::init() {
  ui->plyrList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  ui->btnGroupRace->setId(ui->radioRaceZerg, Chk::Race::Zerg);
  ui->btnGroupRace->setId(ui->radioRaceTerran, Chk::Race::Terran);
  ui->btnGroupRace->setId(ui->radioRaceProtoss, Chk::Race::Protoss);
  ui->btnGroupRace->setId(ui->radioRaceUserSelect, Chk::Race::UserSelectable);
  ui->btnGroupRace->setId(ui->radioRaceRandom, Chk::Race::Random);

  ui->btnGroupController->setId(ui->radioControlInactive, Sc::Player::SlotType::Inactive);
  ui->btnGroupController->setId(ui->radioControlOccupiedComputer, Sc::Player::SlotType::GameComputer);
  ui->btnGroupController->setId(ui->radioControlRescuable, Sc::Player::SlotType::RescuePassive);
  ui->btnGroupController->setId(ui->radioControlDummy, Sc::Player::SlotType::Unused);
  ui->btnGroupController->setId(ui->radioControlComputer, Sc::Player::SlotType::Computer);
  ui->btnGroupController->setId(ui->radioControlHuman, Sc::Player::SlotType::Human);
  ui->btnGroupController->setId(ui->radioControlNeutral, Sc::Player::SlotType::Neutral);
  ui->btnGroupController->setId(ui->radioControlClosed, Sc::Player::SlotType::GameClosed);

  ui->btnGroupPlayerForce->setId(ui->radioForce1, 0);
  ui->btnGroupPlayerForce->setId(ui->radioForce2, 1);
  ui->btnGroupPlayerForce->setId(ui->radioForce3, 2);
  ui->btnGroupPlayerForce->setId(ui->radioForce4, 3);
}

ScenarioSettings::~ScenarioSettings()
{
  delete ui;
}

QString ScenarioSettings::getForceName(int force) const {
  if (force >= 4) return "";

  if (settings.useDefaultForceNames[force]) {
    return getDefaultForceName(force);
  }
  return QString::fromStdString(settings.forceNames[force]);
}

void ScenarioSettings::updatePlayerTree() {
  for (int i = 0; i < Sc::Player::TotalSlots; ++i) {
    unsigned controller = settings.ownr.slotType[i];
    unsigned race = settings.side.playerRaces[i];
    unsigned force = settings.forc.playerForce[i];
    unsigned color = settings.colr.playerColor[i];

    QTreeWidgetItem* itm = ui->plyrList->topLevelItem(i);
    itm->setText(1, getPlayerName(i, race, controller));
    itm->setText(2, getColorName(color));
    itm->setText(3, getRaceName(race));
    itm->setText(4, getPlayerOwnerName(controller));
    itm->setText(5, getForceName(force));
    itm->setData(0, Qt::ItemDataRole::UserRole, i);
  }
  ui->plyrList->selectAll();
}

void ScenarioSettings::syncUiWithData() {
  updatePlayerTree();
}

void ScenarioSettings::readFromMap(const MapFile& map) {
  settings.side = map.players.side->get();
  settings.colr = map.players.colr->get();
  settings.forc = map.players.forc->get();
  settings.ownr = map.players.ownr->get();
  settings.iown = map.players.iown->get();

  settings.unis = map.properties.unis->get();
  settings.unix = map.properties.unix->get();
  settings.puni = map.properties.puni->get();
  settings.upgs = map.properties.upgs->get();
  settings.upgx = map.properties.upgx->get();
  settings.upgr = map.properties.upgr->get();
  settings.pupx = map.properties.pupx->get();
  settings.tecs = map.properties.tecs->get();
  settings.tecx = map.properties.tecx->get();
  settings.ptec = map.properties.ptec->get();
  settings.ptex = map.properties.ptex->get();

  for (unsigned i = 0; i < Chk::TotalForces; ++i) {
    Chk::Force force = Chk::Force(i);
    bool useDefaultForceName = map.strings.getForceNameStringId(force) == Chk::StringId::NoString;

    settings.useDefaultForceNames[i] = useDefaultForceName;
    if (!useDefaultForceName)
      settings.forceNames[i] = *map.strings.getForceName<RawString>(force);
  }

  for (unsigned i = 0; i < Sc::Unit::TotalTypes; ++i) {
    Sc::Unit::Type unitType = Sc::Unit::Type(i);
    bool useDefaultUnitName = map.strings.getUnitNameStringId(unitType) == Chk::StringId::NoString;

    settings.useDefaultUnitNames[i] = useDefaultUnitName;
    if (!useDefaultUnitName)
      settings.unitNames[i] = *map.strings.getUnitName<RawString>(unitType);
  }
  syncUiWithData();
}

void ScenarioSettings::writeToMap(MapFile& map) const {
  map.players.side->get() = settings.side;
  map.players.colr->get() = settings.colr;
  map.players.forc->get() = settings.forc;
  map.players.ownr->get() = settings.ownr;
  map.players.iown->get() = settings.iown;

  map.properties.unis->get() = settings.unis;
  map.properties.unix->get() = settings.unix;
  map.properties.puni->get() = settings.puni;
  map.properties.upgs->get() = settings.upgs;
  map.properties.upgx->get() = settings.upgx;
  map.properties.upgr->get() = settings.upgr;
  map.properties.pupx->get() = settings.pupx;
  map.properties.tecs->get() = settings.tecs;
  map.properties.tecx->get() = settings.tecx;
  map.properties.ptec->get() = settings.ptec;
  map.properties.ptex->get() = settings.ptex;

  for (unsigned i = 0; i < Chk::TotalForces; ++i) {
    Chk::Force force = Chk::Force(i);
    if (settings.useDefaultForceNames[i])
      map.strings.setForceNameStringId(force, Chk::StringId::NoString);
    else
      map.strings.setForceName(force, RawString(settings.forceNames[i]));
  }

  for (unsigned i = 0; i < Sc::Unit::TotalTypes; ++i) {
    Sc::Unit::Type unitType = Sc::Unit::Type(i);
    if (settings.useDefaultUnitNames[i])
      map.strings.setUnitNameStringId(unitType, Chk::StringId::NoString);
    else
      map.strings.setUnitName(unitType, RawString(settings.unitNames[i]));
  }
}

void ScenarioSettings::setSelectedButtonGroup(QButtonGroup* btnGroup, int id) {
  QAbstractButton* btn = btnGroup->button(id);
  if (btn != nullptr)
    btn->setChecked(true);
}

void ScenarioSettings::clearSelectedButtonGroup(QButtonGroup* btnGroup) {
  btnGroup->setExclusive(false);
  for (QAbstractButton* btn : btnGroup->buttons()) {
    btn->setChecked(false);
  }
  btnGroup->setExclusive(true);
}

void ScenarioSettings::on_plyrList_itemSelectionChanged() {
  bool anySelected = !ui->plyrList->selectedItems().empty();

  ui->playerOptionsWidget->setEnabled(anySelected);
  if (!anySelected) return;

  std::set<unsigned> uniqueControllers;
  std::set<unsigned> uniqueRaces;
  std::set<unsigned> uniqueForces;
  std::set<unsigned> uniqueColors;
  for (QTreeWidgetItem* itm : ui->plyrList->selectedItems()) {
    int id = itm->data(0, Qt::ItemDataRole::UserRole).toInt();
    unsigned controller = settings.ownr.slotType[id];
    unsigned race = settings.side.playerRaces[id];
    unsigned force = settings.forc.playerForce[id];
    unsigned color = settings.colr.playerColor[id];

    uniqueControllers.insert(controller);
    uniqueRaces.insert(race);
    uniqueForces.insert(force);
    uniqueColors.insert(color);
  }

  if (uniqueControllers.size() > 1) {
    clearSelectedButtonGroup(ui->btnGroupController);
  }
  else {
    setSelectedButtonGroup(ui->btnGroupController, *uniqueControllers.begin());
  }

  if (uniqueRaces.size() > 1) {
    clearSelectedButtonGroup(ui->btnGroupRace);
  }
  else {
    setSelectedButtonGroup(ui->btnGroupRace, *uniqueRaces.begin());
  }

  if (uniqueForces.size() > 1) {
    clearSelectedButtonGroup(ui->btnGroupPlayerForce);
  }
  else {
    setSelectedButtonGroup(ui->btnGroupPlayerForce, *uniqueForces.begin());
  }
}

