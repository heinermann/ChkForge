#pragma once
#include <QObject>
#include <QWidget>

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
  Ui::AbilitiesTab* ui;
};
