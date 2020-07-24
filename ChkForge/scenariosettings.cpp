#include "scenariosettings.h"
#include "ui_scenariosettings.h"

ScenarioSettings::ScenarioSettings(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ScenarioSettings)
{
  ui->setupUi(this);
}

ScenarioSettings::~ScenarioSettings()
{
  delete ui;
}
