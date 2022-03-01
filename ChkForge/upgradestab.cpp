#include "upgradestab.h"
#include "ui_upgradestab.h"

UpgradesTab::UpgradesTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(new Ui::UpgradesTab)
{
  ui->setupUi(this);
  populatePlayerList(ui->upgradePlyrList);
}

UpgradesTab::~UpgradesTab() {
  delete ui;
}

void UpgradesTab::setTabFocus() {
  ui->upgradeTree->setFocus();
}
