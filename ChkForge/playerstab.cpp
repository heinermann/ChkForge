#include "playerstab.h"
#include "ui_playerstab.h"
#include "strings.h"

#include <MappingCoreLib/Chk.h>

#include <QButtonGroup>
#include <QTreeWidgetItem>
#include <set>

PlayersTab::PlayersTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(new Ui::PlayersTab)
{
  ui->setupUi(this);
  init();
}

void PlayersTab::init() {
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
}

PlayersTab::~PlayersTab() {
  delete ui;
}

void PlayersTab::setTabFocus() {
  ui->plyrList->setFocus();
}


void PlayersTab::setSelectedButtonGroup(QButtonGroup* btnGroup, int id) {
  QAbstractButton* btn = btnGroup->button(id);
  if (btn != nullptr)
    btn->setChecked(true);
}

void PlayersTab::clearSelectedButtonGroup(QButtonGroup* btnGroup) {
  btnGroup->setExclusive(false);
  for (QAbstractButton* btn : btnGroup->buttons()) {
    btn->setChecked(false);
  }
  btnGroup->setExclusive(true);
}

void PlayersTab::on_plyrList_itemSelectionChanged() {
  if (settings == nullptr) return;

  bool anySelected = !ui->plyrList->selectedItems().empty();

  ui->playerOptionsWidget->setEnabled(anySelected);
  if (!anySelected) return;

  std::set<unsigned> uniqueControllers;
  std::set<unsigned> uniqueRaces;
  std::set<unsigned> uniqueForces;
  std::set<unsigned> uniqueColors;
  for (QTreeWidgetItem* itm : ui->plyrList->selectedItems()) {
    int player_id = playerIdFrom(itm);
    unsigned controller = settings->ownr.slotType[player_id];
    unsigned race = settings->side.playerRaces[player_id];
    unsigned force = settings->forc.playerForce[player_id];
    unsigned color = settings->colr.playerColor[player_id];

    uniqueControllers.insert(controller);
    uniqueRaces.insert(race);
    uniqueForces.insert(force);
    uniqueColors.insert(color);
  }

  if (uniqueControllers.size() > 1)
    clearSelectedButtonGroup(ui->btnGroupController);
  else
    setSelectedButtonGroup(ui->btnGroupController, *uniqueControllers.begin());

  if (uniqueRaces.size() > 1)
    clearSelectedButtonGroup(ui->btnGroupRace);
  else
    setSelectedButtonGroup(ui->btnGroupRace, *uniqueRaces.begin());

  if (uniqueForces.size() > 1)
    clearSelectedButtonGroup(ui->btnGroupPlayerForce);
  else
    setSelectedButtonGroup(ui->btnGroupPlayerForce, *uniqueForces.begin());
}

int PlayersTab::playerIdFrom(QTreeWidgetItem* itm) {
  return itm->data(0, Qt::UserRole).toInt();
}

void PlayersTab::on_btnGroupController_idClicked(int id) {
  for (QTreeWidgetItem* itm : ui->plyrList->selectedItems()) {
    int player_id = playerIdFrom(itm);
    settings->ownr.slotType[player_id] = Sc::Player::SlotType(id);
  }
  updatePlayerTree();
}

void PlayersTab::on_btnGroupRace_idClicked(int id) {
  for (QTreeWidgetItem* itm : ui->plyrList->selectedItems()) {
    int player_id = playerIdFrom(itm);
    settings->side.playerRaces[player_id] = Chk::Race(id);
  }
  updatePlayerTree();
}

void PlayersTab::on_btnGroupPlayerForce_idClicked(int id) {
  for (QTreeWidgetItem* itm : ui->plyrList->selectedItems()) {
    int player_id = playerIdFrom(itm);
    settings->forc.playerForce[player_id] = Chk::Force(id);
  }
  updatePlayerTree();
}

void PlayersTab::updatePlayerTree() {
  for (int i = 0; i < Sc::Player::TotalSlots; ++i) {
    unsigned controller = settings->ownr.slotType[i];
    unsigned race = settings->side.playerRaces[i];
    unsigned force = settings->forc.playerForce[i];
    unsigned color = settings->colr.playerColor[i];

    QTreeWidgetItem* itm = ui->plyrList->topLevelItem(i);
    itm->setText(1, getPlayerName(i, race, controller));
    itm->setText(2, getColorName(color));
    itm->setText(3, getRaceName(race));
    itm->setText(4, getPlayerOwnerName(controller));
    itm->setText(5, getForceName(force));
    itm->setData(0, Qt::UserRole, i);
  }
  emit updateData();

  if (ui->plyrList->selectedItems().empty())
    ui->plyrList->topLevelItem(0)->setSelected(true);
}

QString PlayersTab::getForceName(unsigned force) const {
  if (force >= 4 || !settings->useCustomForceNames[force]) {
    return getDefaultForceName(force);
  }
  return QString::fromStdString(settings->forceNames[force]);
}
