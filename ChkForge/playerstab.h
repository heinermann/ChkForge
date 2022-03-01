#pragma once
#include <QObject>
#include <QWidget>
#include <QString>
#include <QButtonGroup>
#include <QTreeWidget>

#include <memory>

#include "scenariosettingstab.h"
#include "RadioDataMapper.h"

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
  void setupDataMappers();

private:
  void init();
  void setSelectedButtonGroup(QButtonGroup* btnGroup, int id);
  void clearSelectedButtonGroup(QButtonGroup* btnGroup);
  int playerIdFrom(QTreeWidgetItem* itm);
  QString getForceName(unsigned force) const;

private:
  std::unique_ptr<Ui::PlayersTab> ui;

  std::shared_ptr<ChkForge::RadioDataMapper<Sc::Player::SlotType>> rdoSlot;
  std::shared_ptr<ChkForge::RadioDataMapper<Chk::Race>> rdoRace;
  std::shared_ptr<ChkForge::RadioDataMapper<Chk::Force>> rdoForce;

private slots:
  void on_plyrList_itemSelectionChanged();

signals:
  void updateData();
};
