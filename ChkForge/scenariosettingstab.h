#pragma once

#include <QObject>
#include <QWidget>

#include "scenariosettingsdata.h"

class QTreeWidget;

class ScenarioSettingsTab : public QWidget {
  Q_OBJECT
public:
  explicit ScenarioSettingsTab(QWidget* parent = nullptr);

  virtual ~ScenarioSettingsTab();
  virtual void setTabFocus() = 0;

  void populatePlayerList(QTreeWidget* treeWidget, unsigned count = Sc::Player::Total);

  ScenarioSettingsData* settings = nullptr;
};
