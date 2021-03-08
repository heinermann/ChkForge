#ifndef SCENARIOSETTINGS_H
#define SCENARIOSETTINGS_H

#include <QDialog>

namespace Ui {
  class ScenarioSettings;
}

class ScenarioSettings : public QDialog
{
  Q_OBJECT

public:
  enum Tab {
    TAB_PLAYERS,
    TAB_FORCES,
    TAB_UNITS,
    TAB_UPGRADES,
    TAB_ABILITIES
  };

  explicit ScenarioSettings(QWidget* parent = nullptr);
  explicit ScenarioSettings(QWidget *parent, int startTab);
  ~ScenarioSettings();
private:
  Ui::ScenarioSettings *ui;
};

#endif // SCENARIOSETTINGS_H
