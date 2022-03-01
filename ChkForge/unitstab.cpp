#include "unitstab.h"
#include "ui_unitstab.h"

UnitsTab::UnitsTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(std::make_unique<Ui::UnitsTab>())
{
  ui->setupUi(this);
  populatePlayerList(ui->unitPlyrList);
}

UnitsTab::~UnitsTab() {}

void UnitsTab::setTabFocus() {
  ui->unitTree->setFocus();
}
