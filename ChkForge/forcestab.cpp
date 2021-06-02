#include "forcestab.h"
#include "ui_forcestab.h"
#include "strings.h"

#include <MappingCoreLib/Sc.h>
#include <set>

ForcesTab::ForcesTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(new Ui::ForcesTab)
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
    QTreeWidgetItem* playerItem = new QTreeWidgetItem(QStringList{ getGenericPlayerName(i), "a", "b", QString::number(i) });
    playerItem->setData(0, Qt::UserRole, i);
    playerItem->setFlags(Qt::ItemNeverHasChildren);
    playersUnderForces.append(playerItem);
  }
  ui->forcesTree->topLevelItem(4)->addChildren(playersUnderForces);
  ui->forcesTree->hideColumn(3);
  ui->forcesTree->expandAll();
}

ForcesTab::~ForcesTab() {
  delete ui;
}

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

void ForcesTab::updateForcesTree() {
  bool useRemasteredColorStr = anyRemasteredColor();

  for (int playerSlot = 0; playerSlot < playersUnderForces.size(); ++playerSlot) {
    QTreeWidgetItem* itm = playersUnderForces[playerSlot];

    itm->setText(0, getSlotOwnerName(settings->ownr.slotType[playerSlot]));
    itm->setText(1, getSlotRaceName(settings->side.playerRaces[playerSlot]));
    itm->setText(2, useRemasteredColorStr ? getSlotColorName(3, settings->colr.playerColor[playerSlot]) : tr("Map specified"));

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

void ForcesTab::on_forcesTree_itemSelectionChanged() {
  if (settings == nullptr) return;

  std::set<bool> uniqueHasCustomName;
  std::set<bool> uniqueAllies;
  std::set<bool> uniqueRandomStartLoc;
  std::set<bool> uniqueAlliedVictory;
  std::set<bool> uniqueSharedVision;
  std::set<std::string> uniqueNames;
  for (QTreeWidgetItem* itm : ui->forcesTree->selectedItems()) {
    int force = ui->forcesTree->indexOfTopLevelItem(itm);

    uniqueHasCustomName.insert(!settings->useDefaultForceNames[force]);
    uniqueNames.insert(settings->forceNames[force]);
    uniqueAllies.insert(settings->forc.flags[force] & Chk::ForceFlags::RandomAllies);
    uniqueRandomStartLoc.insert(settings->forc.flags[force] & Chk::ForceFlags::RandomizeStartLocation);
    uniqueAlliedVictory.insert(settings->forc.flags[force] & Chk::ForceFlags::AlliedVictory);
    uniqueSharedVision.insert(settings->forc.flags[force] & Chk::ForceFlags::SharedVision);
  }

  // TODO: normal tri-state groupbox
  //if (uniqueHasCustomName.size() > 1)

  if (uniqueNames.size() > 1)
    ui->txtForceName->clear();
  else
    ui->txtForceName->setText(QString::fromStdString(*uniqueNames.begin()));

  if (uniqueAllies.size() > 1)
    ui->chkForceAllies->setCheckState(Qt::PartiallyChecked);
  else
    ui->chkForceAllies->setChecked(*uniqueAllies.begin());

  if (uniqueRandomStartLoc.size() > 1)
    ui->chkForceRandomStartLocation->setCheckState(Qt::PartiallyChecked);
  else
    ui->chkForceRandomStartLocation->setChecked(*uniqueRandomStartLoc.begin());

  if (uniqueAlliedVictory.size() > 1)
    ui->chkForceAlliedVictory->setCheckState(Qt::PartiallyChecked);
  else
    ui->chkForceAlliedVictory->setChecked(*uniqueAlliedVictory.begin());

  if (uniqueSharedVision.size() > 1)
    ui->chkForceSharedVision->setCheckState(Qt::PartiallyChecked);
  else
    ui->chkForceSharedVision->setChecked(*uniqueSharedVision.begin());
}

void ForcesTab::on_txtForceName_textEdited(const QString& text) {
  for (QTreeWidgetItem* itm : ui->forcesTree->selectedItems()) {
    int force = ui->forcesTree->indexOfTopLevelItem(itm);
    QByteArray utf8Name = text.toUtf8();
    settings->forceNames[force] = std::string(utf8Name.data(), utf8Name.size());
    settings->useDefaultForceNames[force] = false;
    ui->grpForceCustomName->setChecked(true);
  }
}

void ForcesTab::on_chkForceAllies_stateChanged(int state) {
  for (QTreeWidgetItem* itm : ui->forcesTree->selectedItems()) {
    int force = ui->forcesTree->indexOfTopLevelItem(itm);

    if (state == Qt::Checked)
      settings->forc.flags[force] |= Chk::ForceFlags::RandomAllies;
    else if (state == Qt::Unchecked)
      settings->forc.flags[force] &= ~Chk::ForceFlags::RandomAllies;
  }
}

void ForcesTab::on_chkForceAlliedVictory_stateChanged(int state) {
  for (QTreeWidgetItem* itm : ui->forcesTree->selectedItems()) {
    int force = ui->forcesTree->indexOfTopLevelItem(itm);

    if (state == Qt::Checked)
      settings->forc.flags[force] |= Chk::ForceFlags::AlliedVictory;
    else if (state == Qt::Unchecked)
      settings->forc.flags[force] &= ~Chk::ForceFlags::AlliedVictory;
  }
}

void ForcesTab::on_chkForceRandomStartLocation_stateChanged(int state) {
  for (QTreeWidgetItem* itm : ui->forcesTree->selectedItems()) {
    int force = ui->forcesTree->indexOfTopLevelItem(itm);

    if (state == Qt::Checked)
      settings->forc.flags[force] |= Chk::ForceFlags::RandomizeStartLocation;
    else if (state == Qt::Unchecked)
      settings->forc.flags[force] &= ~Chk::ForceFlags::RandomizeStartLocation;
  }
}

void ForcesTab::on_chkForceSharedVision_stateChanged(int state) {
  for (QTreeWidgetItem* itm : ui->forcesTree->selectedItems()) {
    int force = ui->forcesTree->indexOfTopLevelItem(itm);

    if (state == Qt::Checked)
      settings->forc.flags[force] |= Chk::ForceFlags::SharedVision;
    else if (state == Qt::Unchecked)
      settings->forc.flags[force] &= ~Chk::ForceFlags::SharedVision;
  }
}
