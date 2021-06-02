#pragma once
#include <QObject>
#include <QWidget>
#include <QList>
#include <QTreeWidget>

#include "scenariosettingstab.h"

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
  Ui::ForcesTab* ui;

  QList<QTreeWidgetItem*> playersUnderForces;

private slots:
  void on_forcesTree_itemSelectionChanged();
  void on_txtForceName_textEdited(const QString& text);
  void on_chkForceAllies_stateChanged(int state);
  void on_chkForceAlliedVictory_stateChanged(int state);
  void on_chkForceRandomStartLocation_stateChanged(int state);
  void on_chkForceSharedVision_stateChanged(int state);
};
