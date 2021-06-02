#pragma once
#include <QObject>
#include <QWidget>
#include <QString>
#include <QButtonGroup>
#include <QTreeWidget>

#include "scenariosettingstab.h"

namespace Ui {
  class PlayersTab;
}

class PlayersTab : public ScenarioSettingsTab {
  Q_OBJECT
public:
  explicit PlayersTab(QWidget* parent = nullptr);
  ~PlayersTab() override;

  void setTabFocus() override;
  void updatePlayerTree();

private:
  void init();
  void setSelectedButtonGroup(QButtonGroup* btnGroup, int id);
  void clearSelectedButtonGroup(QButtonGroup* btnGroup);
  int playerIdFrom(QTreeWidgetItem* itm);
  QString getForceName(unsigned force) const;

private:
  Ui::PlayersTab* ui;

private slots:
  void on_plyrList_itemSelectionChanged();
  void on_btnGroupController_idClicked(int id);
  void on_btnGroupRace_idClicked(int id);
  void on_btnGroupPlayerForce_idClicked(int id);

signals:
  void updateData();
};
