#pragma once

#include <QObject>
#include <QWidget>

#include "scenariosettingsdata.h"

class ScenarioSettingsTab : public QWidget {
  Q_OBJECT
public:
  explicit ScenarioSettingsTab(QWidget* parent = nullptr);

  virtual ~ScenarioSettingsTab();
  virtual void setTabFocus() = 0;

  ScenarioSettingsData* settings = nullptr;
};
