#pragma once
#include <QObject>
#include <QWidget>
#include <memory>

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
  std::unique_ptr<Ui::UnitsTab> ui;
};
