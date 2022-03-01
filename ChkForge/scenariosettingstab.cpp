#include "scenariosettingstab.h"

#include <QTreeWidget>

#include "strings.h"

ScenarioSettingsTab::ScenarioSettingsTab(QWidget* parent)
  : QWidget(parent)
{}

ScenarioSettingsTab::~ScenarioSettingsTab()
{}

void ScenarioSettingsTab::populatePlayerList(QTreeWidget* treeWidget, unsigned count) {
  for (unsigned i = 0; i < count; ++i) {
    QTreeWidgetItem* item = new QTreeWidgetItem({ ChkForge::getGenericPlayerName(i) });
    treeWidget->addTopLevelItem(item);
  }
}
