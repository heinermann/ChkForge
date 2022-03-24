#pragma once
#include <QObject>
#include <QWidget>
#include <QStandardItemModel>
#include <memory>

#include "scenariosettingstab.h"

namespace Ui {
  class AbilitiesTab;
}

class AbilitiesTab : public ScenarioSettingsTab {
  Q_OBJECT
public:
  explicit AbilitiesTab(QWidget* parent = nullptr);
  ~AbilitiesTab() override;

  void setTabFocus() override;

private:
  void init();

private:
  std::unique_ptr<Ui::AbilitiesTab> ui;
  QStandardItemModel techTreeModel;
};
