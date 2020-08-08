#include "itemtree.h"
#include "ui_itemtree.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QTextStream>

#include "icons.h"

ItemTree::ItemTree(QWidget *parent)
  : DockWidgetWrapper<Ui::ItemTree>("Item Tree", parent)
  , treeModel(this)
{
  treeModel.invisibleRootItem()->appendRow(createTilesetTree());
  treeModel.invisibleRootItem()->appendRow(createDoodadsTree());
  treeModel.invisibleRootItem()->appendRow(createUnitsTree());
  treeModel.invisibleRootItem()->appendRow(createSpritesTree());
  treeModel.invisibleRootItem()->appendRow(createUnitSpritesTree());
  treeModel.invisibleRootItem()->appendRow(createLocationsTree());
  treeModel.invisibleRootItem()->appendRow(createBrushesTree());

  ui->treeView->setModel(&treeModel);
}


QStandardItem* ItemTree::createTilesetTree()
{
  QStandardItem* top = createTreeItem("Tileset");
  createTreeFromFile(top, "terrain.txt");
  return top;
}

QStandardItem* ItemTree::createDoodadsTree()
{
  QStandardItem* top = createTreeItem(tr("Doodads"));

  return top;
}

QStandardItem* ItemTree::createUnitsTree()
{
  QStandardItem* top = createTreeItem(tr("Units"));
  createTreeFromFile(top, "units.txt", [](QStandardItem* itm) {
    itm->setIcon(ChkForge::Icons::getUnitIcon(itm->data().toInt()));
  });
  return top;
}

QStandardItem* ItemTree::createSpritesTree()
{
  QStandardItem* top = createTreeItem(tr("Sprites"));
  createTreeFromFile(top, "sprites.txt", [](QStandardItem* itm) {
    itm->setIcon(QIcon(":/icons/sprite.png"));
    });
  return top;
}

QStandardItem* ItemTree::createUnitSpritesTree()
{
  QStandardItem* top = createTreeItem(tr("Unit-Sprites"));
  createTreeFromFile(top, "units.txt", [](QStandardItem* itm) {
    itm->setIcon(ChkForge::Icons::getUnitIcon(itm->data().toInt()));
    });
  return top;
}

QStandardItem* ItemTree::createLocationsTree()
{
  QStandardItem* top = createTreeItem(tr("Locations"));

  return top;
}

QStandardItem* ItemTree::createBrushesTree()
{
  QStandardItem* top = createTreeItem(tr("Custom Brushes"));
  QStandardItem* clipboard = createTreeItem(tr("Clipboard"));
  clipboard->setIcon(QIcon(":/themes/oxygen-icons-png/oxygen/16x16/actions/edit-paste.png"));
  top->appendRow(clipboard);
  return top;
}

void ItemTree::createTreeFromFile(QStandardItem* parent, const QString& resourceName, const std::function<void(QStandardItem*)>& item_cb)
{
  QFile treeFile(":/Trees/" + resourceName);
  if (!treeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::critical(this, "Error", "Unable to load Trees/" + resourceName + ": " + treeFile.errorString());
  }
  QTextStream contents(treeFile.readAll());
  txtToTree(parent, contents, item_cb);
}

void ItemTree::txtToTree(QStandardItem *parent, QTextStream &txt, const std::function<void(QStandardItem*)>& item_cb)
{
  static QRegularExpression VALUE_REGEX("\\s*(?<id>\\d+)\\s+(?<name>.*)\\s*");
  static QRegularExpression START_CATEGORY_REGEX("\\s*#(?<group>.*)\\s*");
  static QRegularExpression END_CATEGORY_REGEX("\\s*##\\s*");

  while(!txt.atEnd()) {
    QString line = txt.readLine();

    auto endCategoryMatch = END_CATEGORY_REGEX.match(line);
    if (endCategoryMatch.hasMatch()) return;

    auto startCategoryMatch = START_CATEGORY_REGEX.match(line);
    if (startCategoryMatch.hasMatch()) {
      QStandardItem *newGroup = createTreeItem(startCategoryMatch.captured("group"));
      txtToTree(newGroup, txt, item_cb);
      parent->appendRow(newGroup);
    }
    else
    {
      auto valueMatch = VALUE_REGEX.match(line);
      if (valueMatch.hasMatch())
      {
        QStandardItem* item = createTreeItem(valueMatch.captured("name"));
        item->setData(valueMatch.captured("id").toInt());
        if (item_cb) item_cb(item);
        parent->appendRow(item);
      }
    }
  }
}

QStandardItem* ItemTree::createTreeItem(const QString& text)
{
  QStandardItem* item = new QStandardItem(text);
  item->setEditable(false);
  return item;
}
