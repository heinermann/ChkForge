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
  explicit ScenarioSettings(QWidget *parent = nullptr);
  ~ScenarioSettings();

private:
  Ui::ScenarioSettings *ui;
};

#endif // SCENARIOSETTINGS_H
