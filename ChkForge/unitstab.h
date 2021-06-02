#pragma once
#include <QObject>
#include <QWidget>

#include "scenariosettingstab.h"

namespace Ui {
  class UnitsTab;
}

class UnitsTab : public ScenarioSettingsTab {
  Q_OBJECT
public:
  explicit UnitsTab(QWidget* parent = nullptr);
  ~UnitsTab() override;

  void setTabFocus() override;

private:
  Ui::UnitsTab* ui;

};
