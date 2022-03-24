#include "unitstab.h"
#include "ui_unitstab.h"

#include "tree.h"
#include "icons.h"

UnitsTab::UnitsTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(std::make_unique<Ui::UnitsTab>())
  , unitTreeModel(this)
{
  ui->setupUi(this);
  populatePlayerList(ui->unitPlyrList);
  init();
}

UnitsTab::~UnitsTab() {}

void UnitsTab::setTabFocus() {
  ui->unitTree->setFocus();
}

void UnitsTab::init() {
  ChkForge::Tree::createTreeFromFile(unitTreeModel.invisibleRootItem(), "units.txt", 0, [](QStandardItem* itm) {
    itm->setIcon(ChkForge::Icons::getUnitIcon(itm->data(ChkForge::Tree::ROLE_ID).toInt()));
  });

  ui->unitTree->setModel(&unitTreeModel);
  ui->unitTree->expandAll();
}
