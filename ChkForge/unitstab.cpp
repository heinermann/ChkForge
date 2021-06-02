#include "unitstab.h"
#include "ui_unitstab.h"

UnitsTab::UnitsTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(new Ui::UnitsTab)
{
  ui->setupUi(this);
}

UnitsTab::~UnitsTab() {
  delete ui;
}

void UnitsTab::setTabFocus() {
  ui->unitTree->setFocus();
}
