#include "forcestab.h"
#include "ui_forcestab.h"
#include "strings.h"

#include <MappingCoreLib/Sc.h>
#include <set>

ForcesTab::ForcesTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(std::make_unique<Ui::ForcesTab>())
{
  ui->setupUi(this);
  init();
}

void ForcesTab::init() {
  // Set up Forces tab
  ui->forcesTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  for (int i = 0; i < 5; ++i) {
    QTreeWidgetItem* forceItem = ui->forcesTree->topLevelItem(i);
    forceItem->setData(0, Qt::UserRole, i);
    forceItem->setFirstColumnSpanned(true);
  }

  for (int i = 0; i < Sc::Player::TotalSlots; ++i) {
    QTreeWidgetItem* playerItem = new QTreeWidgetItem(QStringList{ ChkForge::getGenericPlayerName(i), "a", "b", QString::number(i) });
    playerItem->setData(0, Qt::UserRole, i);
    playerItem->setFlags(Qt::ItemNeverHasChildren);
    playersUnderForces.append(playerItem);
  }
  ui->forcesTree->topLevelItem(4)->addChildren(playersUnderForces);
  ui->forcesTree->hideColumn(3);
  ui->forcesTree->expandAll();
}

void ForcesTab::setupDataMappers() {
  chkAllies = std::make_shared<ChkForge::CheckboxDataMapper<u8>>(ui->forcesTree, ui->chkForceAllies, settings->forc.flags, Chk::ForceFlags::RandomAllies);
  chkAlliedVictory = std::make_shared<ChkForge::CheckboxDataMapper<u8>>(ui->forcesTree, ui->chkForceAlliedVictory, settings->forc.flags, Chk::ForceFlags::AlliedVictory);
  chkRandomStartLocation = std::make_shared<ChkForge::CheckboxDataMapper<u8>>(ui->forcesTree, ui->chkForceRandomStartLocation, settings->forc.flags, Chk::ForceFlags::RandomizeStartLocation);
  chkSharedVision = std::make_shared<ChkForge::CheckboxDataMapper<u8>>(ui->forcesTree, ui->chkForceSharedVision, settings->forc.flags, Chk::ForceFlags::SharedVision);
  chkCustomName = std::make_shared<ChkForge::GroupBoxDataMapper<bool>>(ui->forcesTree, ui->grpForceCustomName, &settings->useCustomForceNames[0], true);
  txtForceName = std::make_shared<ChkForge::TextDataMapper>(ui->forcesTree, ui->txtForceName, &settings->forceNames[0]);
}

ForcesTab::~ForcesTab() {}

void ForcesTab::setTabFocus() {
  ui->forcesTree->setFocus();
}

void ForcesTab::setPlayerSlotEnabled(int slot, bool enabled) {
  for (int i = 0; i < playersUnderForces[slot]->columnCount(); ++i) {
    playersUnderForces[slot]->setForeground(i, enabled ? QColorConstants::Black : QColorConstants::Gray);
  }
}

void ForcesTab::setForceSlotEnabled(int slot, bool enabled) {
  ui->forcesTree->topLevelItem(slot)->setForeground(0, enabled ? QColorConstants::Black : QColorConstants::Gray);
}

void ForcesTab::updatePlayerSlotEnabled(int slot) {
  setPlayerSlotEnabled(slot, isPlayerSlotEnabled(slot));
}

bool ForcesTab::isPlayerSlotEnabled(int slot) {
  return settings->forc.playerForce[slot] < 4 &&
    (settings->ownr.slotType[slot] == Sc::Player::SlotType::Computer || settings->ownr.slotType[slot] == Sc::Player::SlotType::Human);
}

int ForcesTab::playerIdFrom(QTreeWidgetItem* itm) {
  return itm->data(0, Qt::UserRole).toInt();
}

void ForcesTab::updateForceSlotEnabled(int slot) {
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

bool ForcesTab::anyRemasteredColor() {
  for (Chk::PlayerColor color : settings->colr.playerColor) {
    if (color >= 16) return true;
  }
  return false;
}

// TODO: Change force name in the QTreeView
void ForcesTab::updateForcesTree() {
  bool useRemasteredColorStr = anyRemasteredColor();

  for (int playerSlot = 0; playerSlot < playersUnderForces.size(); ++playerSlot) {
    QTreeWidgetItem* itm = playersUnderForces[playerSlot];

    itm->setText(0, ChkForge::getSlotOwnerName(settings->ownr.slotType[playerSlot]));
    itm->setText(1, ChkForge::getSlotRaceName(settings->side.playerRaces[playerSlot]));
    //: gluAll:lobbyScreen_MapSpecified
    itm->setText(2, useRemasteredColorStr ? ChkForge::getSlotColorName(3, settings->colr.playerColor[playerSlot]) : tr("Map specified"));

    int index = itm->parent()->indexOfChild(itm);
    itm->parent()->takeChild(index);

    int force = std::clamp(settings->forc.playerForce[playerSlot], Chk::Force(0), Chk::Force(4));

    ui->forcesTree->topLevelItem(force)->addChild(itm);
    updatePlayerSlotEnabled(playerSlot);
  }

  for (int forceSlot = 0; forceSlot < 5; ++forceSlot) {
    updateForceSlotEnabled(forceSlot);
  }
  ui->forcesTree->expandAll();

  if (ui->forcesTree->selectedItems().empty())
    ui->forcesTree->topLevelItem(0)->setSelected(true);
}
