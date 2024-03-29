#pragma once
#include <QObject>
#include <QWidget>
#include <QList>
#include <QTreeWidget>
#include <memory>

#include "scenariosettingstab.h"
#include "CheckboxDataMapper.h"
#include "GroupBoxDataMapper.h"
#include "TextDataMapper.h"

namespace Ui {
  class ForcesTab;
}

class ForcesTab : public ScenarioSettingsTab {
  Q_OBJECT
public:
  explicit ForcesTab(QWidget* parent = nullptr);
  ~ForcesTab() override;

  void setTabFocus() override;
  void updateForcesTree();
  void setupDataMappers();

private:
  void init();

  void setPlayerSlotEnabled(int slot, bool enabled);
  void setForceSlotEnabled(int slot, bool enabled);

  void updatePlayerSlotEnabled(int slot);
  bool isPlayerSlotEnabled(int slot);
  void updateForceSlotEnabled(int slot);
  bool anyRemasteredColor();

  int playerIdFrom(QTreeWidgetItem* itm); // TODO: Duplicated from PlayersTab

private:
  std::unique_ptr<Ui::ForcesTab> ui;

  QList<QTreeWidgetItem*> playersUnderForces;

  std::shared_ptr<ChkForge::CheckboxDataMapper<u8>> chkAllies;
  std::shared_ptr<ChkForge::CheckboxDataMapper<u8>> chkAlliedVictory;
  std::shared_ptr<ChkForge::CheckboxDataMapper<u8>> chkRandomStartLocation;
  std::shared_ptr<ChkForge::CheckboxDataMapper<u8>> chkSharedVision;
  std::shared_ptr<ChkForge::GroupBoxDataMapper<bool>> chkCustomName;
  std::shared_ptr<ChkForge::TextDataMapper> txtForceName;
};
