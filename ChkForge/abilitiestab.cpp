#include "abilitiestab.h"
#include "ui_abilitiestab.h"

AbilitiesTab::AbilitiesTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(new Ui::AbilitiesTab)
{
  ui->setupUi(this);
}

AbilitiesTab::~AbilitiesTab() {
  delete ui;
}

void AbilitiesTab::setTabFocus() {
  ui->techTree->setFocus();
}
