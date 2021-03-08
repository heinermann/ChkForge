#include "scenariosettings.h"
#include "ui_scenariosettings.h"

ScenarioSettings::ScenarioSettings(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::ScenarioSettings)
{
  ui->setupUi(this);
}

ScenarioSettings::ScenarioSettings(QWidget* parent, int startTab) :
  ScenarioSettings(parent)
{
  ui->tabs->setCurrentIndex(startTab);
}

ScenarioSettings::~ScenarioSettings()
{
  delete ui;
}
