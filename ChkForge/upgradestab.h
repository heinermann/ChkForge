#pragma once
#include <QObject>
#include <QWidget>
#include <QStandardItemModel>
#include <memory>

#include "scenariosettingstab.h"

namespace Ui {
  class UpgradesTab;
}

class UpgradesTab : public ScenarioSettingsTab {
  Q_OBJECT
public:
  explicit UpgradesTab(QWidget* parent = nullptr);
  ~UpgradesTab() override;

  void setTabFocus() override;

private:
  void init();

private:
  std::unique_ptr<Ui::UpgradesTab> ui;
  QStandardItemModel upgradesTreeModel;
};
