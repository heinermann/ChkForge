#include "abilitiestab.h"
#include "ui_abilitiestab.h"

AbilitiesTab::AbilitiesTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(std::make_unique<Ui::AbilitiesTab>())
{
  ui->setupUi(this);
  populatePlayerList(ui->techPlyrList);
}

AbilitiesTab::~AbilitiesTab() {}

void AbilitiesTab::setTabFocus() {
  ui->techTree->setFocus();
}
