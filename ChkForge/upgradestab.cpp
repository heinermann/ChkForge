#include "upgradestab.h"
#include "ui_upgradestab.h"

UpgradesTab::UpgradesTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(std::make_unique<Ui::UpgradesTab>())
{
  ui->setupUi(this);
  populatePlayerList(ui->upgradePlyrList);
}

UpgradesTab::~UpgradesTab() {}

void UpgradesTab::setTabFocus() {
  ui->upgradeTree->setFocus();
}
