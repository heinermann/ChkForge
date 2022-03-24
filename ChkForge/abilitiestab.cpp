#include "abilitiestab.h"
#include "ui_abilitiestab.h"

#include "tree.h"
#include "icons.h"

AbilitiesTab::AbilitiesTab(QWidget* parent)
  : ScenarioSettingsTab(parent)
  , ui(std::make_unique<Ui::AbilitiesTab>())
  , techTreeModel(this)
{
  ui->setupUi(this);
  populatePlayerList(ui->techPlyrList);
  init();
}

AbilitiesTab::~AbilitiesTab() {}

void AbilitiesTab::setTabFocus() {
  ui->techTree->setFocus();
}

void AbilitiesTab::init() {
  ChkForge::Tree::createTreeFromFile(techTreeModel.invisibleRootItem(), "techs.txt", 0, [](QStandardItem* itm) {
    itm->setIcon(ChkForge::Icons::getTechIcon(itm->data(ChkForge::Tree::ROLE_ID).toInt()));
  });

  ui->techTree->setModel(&techTreeModel);
  ui->techTree->expandAll();
}