#include "upgradestab.h"
#include "ui_upgradestab.h"

#include "tree.h"
#include "icons.h"

UpgradesTab::UpgradesTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(std::make_unique<Ui::UpgradesTab>())
  , upgradesTreeModel(this)
{
  ui->setupUi(this);
  populatePlayerList(ui->upgradePlyrList);
  init();
}

UpgradesTab::~UpgradesTab() {}

void UpgradesTab::setTabFocus() {
  ui->upgradeTree->setFocus();
}

void UpgradesTab::init() {
  ChkForge::Tree::createTreeFromFile(upgradesTreeModel.invisibleRootItem(), "upgrades.txt", 0, [](QStandardItem* itm) {
    itm->setIcon(ChkForge::Icons::getUpgradeIcon(itm->data(ChkForge::Tree::ROLE_ID).toInt()));
  });

  ui->upgradeTree->setModel(&upgradesTreeModel);
  ui->upgradeTree->expandAll();
}
