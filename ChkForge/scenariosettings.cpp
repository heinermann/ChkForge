#include "scenariosettings.h"
#include "ui_scenariosettings.h"
#include "strings.h"

#include <set>
#include <algorithm>

ScenarioSettings::ScenarioSettings(QWidget* parent, int startTab) :
  QDialog(parent),
  ui(new Ui::ScenarioSettings)
{
  ui->setupUi(this);

  ui->tabs->setCurrentIndex(startTab);
  on_tabs_currentChanged(startTab);
  init();
}

void ScenarioSettings::init() {
  // Set up Players tab
  ui->plyrList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->plyrList->header()->setSectionResizeMode(1, QHeaderView::Stretch);
  ui->plyrList->header()->setSectionsMovable(false);

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
  ui->btnGroupPlayerForce->setId(ui->radioForceNone, 4);

  // Set up Forces tab
  ui->forcesTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  for (int i = 0; i < 5; ++i) {
    QTreeWidgetItem* forceItem = ui->forcesTree->topLevelItem(i);
    forceItem->setFirstColumnSpanned(true);
  }

  for (int i = 0; i < Sc::Player::TotalSlots; ++i) {
    QTreeWidgetItem* playerItem = new QTreeWidgetItem(QStringList{ getGenericPlayerName(i), "a", "b", QString::number(i) });
    playerItem->setData(0, Qt::UserRole, i);
    playerItem->setFlags(Qt::ItemNeverHasChildren);
    playersUnderForces.append(playerItem);
  }
  ui->forcesTree->topLevelItem(4)->addChildren(playersUnderForces);
  ui->forcesTree->hideColumn(3);
  ui->forcesTree->expandAll();
}

ScenarioSettings::~ScenarioSettings()
{
  delete ui;
}

QString ScenarioSettings::getForceName(unsigned force) const {
  if (force >= 4 || settings.useDefaultForceNames[force]) {
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
    itm->setData(0, Qt::UserRole, i);
  }
  updateForcesTree();
}

void ScenarioSettings::setPlayerSlotEnabled(int slot, bool enabled) {
  for (int i = 0; i < playersUnderForces[slot]->columnCount(); ++i) {
    playersUnderForces[slot]->setTextColor(i, enabled ? QColorConstants::Black : QColorConstants::Gray);
  }
}

void ScenarioSettings::setForceSlotEnabled(int slot, bool enabled) {
  ui->forcesTree->topLevelItem(slot)->setTextColor(0, enabled ? QColorConstants::Black : QColorConstants::Gray);
}

void ScenarioSettings::updatePlayerSlotEnabled(int slot) {
  setPlayerSlotEnabled(slot, isPlayerSlotEnabled(slot));
}

bool ScenarioSettings::isPlayerSlotEnabled(int slot) {
  return settings.forc.playerForce[slot] < 4 &&
    (settings.ownr.slotType[slot] == Sc::Player::SlotType::Computer || settings.ownr.slotType[slot] == Sc::Player::SlotType::Human);
}

void ScenarioSettings::updateForceSlotEnabled(int slot) {
  QTreeWidgetItem* forceItem = ui->forcesTree->topLevelItem(slot);
  bool shouldBeEnabled = false;
  for (int i = 0; i < forceItem->childCount(); ++i) {
    if (isPlayerSlotEnabled(playerIdFrom(forceItem->child(i)))) {
      shouldBeEnabled = true;
      break;
    }
  }
  setForceSlotEnabled(slot, shouldBeEnabled);
}

bool ScenarioSettings::anyRemasteredColor() {
  for (Chk::PlayerColor color : settings.colr.playerColor) {
    if (color >= 16) return true;
  }
  return false;
}

void ScenarioSettings::updateForcesTree() {
  bool useRemasteredColorStr = anyRemasteredColor();

  for (int playerSlot = 0; playerSlot < playersUnderForces.size(); ++playerSlot) {
    QTreeWidgetItem* itm = playersUnderForces[playerSlot];

    itm->setText(0, getSlotOwnerName(settings.ownr.slotType[playerSlot]));
    itm->setText(1, getSlotRaceName(settings.side.playerRaces[playerSlot]));
    itm->setText(2, useRemasteredColorStr ? getSlotColorName(3, settings.colr.playerColor[playerSlot]) : tr("Map specified"));

    int index = itm->parent()->indexOfChild(itm);
    itm->parent()->takeChild(index);

    int force = std::clamp(settings.forc.playerForce[playerSlot], Chk::Force(0), Chk::Force(4));

    ui->forcesTree->topLevelItem(force)->addChild(itm);
    updatePlayerSlotEnabled(playerSlot);
  }

  for (int forceSlot = 0; forceSlot < 5; ++forceSlot) {
    updateForceSlotEnabled(forceSlot);
  }
  ui->forcesTree->expandAll();
}

void ScenarioSettings::syncUiWithData() {
  // Also calls updateForcesTree()
  updatePlayerTree();
  ui->plyrList->setItemSelected(ui->plyrList->topLevelItem(0), true);
  ui->forcesTree->setItemSelected(ui->forcesTree->topLevelItem(0), true);
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
    int player_id = playerIdFrom(itm);
    unsigned controller = settings.ownr.slotType[player_id];
    unsigned race = settings.side.playerRaces[player_id];
    unsigned force = settings.forc.playerForce[player_id];
    unsigned color = settings.colr.playerColor[player_id];

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

int ScenarioSettings::playerIdFrom(QTreeWidgetItem* itm) {
  return itm->data(0, Qt::UserRole).toInt();
}

void ScenarioSettings::on_btnGroupController_idClicked(int id) {
  for (QTreeWidgetItem* itm : ui->plyrList->selectedItems()) {
    int player_id = playerIdFrom(itm);
    settings.ownr.slotType[player_id] = Sc::Player::SlotType(id);
  }
  updatePlayerTree();
}

void ScenarioSettings::on_btnGroupRace_idClicked(int id) {
  for (QTreeWidgetItem* itm : ui->plyrList->selectedItems()) {
    int player_id = playerIdFrom(itm);
    settings.side.playerRaces[player_id] = Chk::Race(id);
  }
  updatePlayerTree();
}

void ScenarioSettings::on_btnGroupPlayerForce_idClicked(int id) {
  for (QTreeWidgetItem* itm : ui->plyrList->selectedItems()) {
    int player_id = playerIdFrom(itm);
    settings.forc.playerForce[player_id] = Chk::Force(id);
  }
  updatePlayerTree();
}

void ScenarioSettings::on_tabs_currentChanged(int index) {
  switch (index) {
  case 0: // players
    ui->plyrList->setFocus();
    break;
  case 1: // forces
    ui->forcesTree->setFocus();
    break;
  case 2: // units
    ui->unitTree->setFocus();
    break;
  case 3: // upgrades
    ui->upgradeTree->setFocus();
    break;
  case 4: // tech
    ui->techTree->setFocus();
    break;
  default:
    break;
  }
}
