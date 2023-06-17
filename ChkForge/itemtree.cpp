#include "itemtree.h"
#include "ui_itemtree.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QTextStream>
#include <QEvent>
#include <QKeyEvent>

#include <MappingCoreLib/Sc.h>
#include "terrain.h"

#include "icons.h"
#include "tree.h"

ItemTree::ItemTree(QWidget *parent)
  : DockWidgetWrapper(tr("Item Tree"), parent)
  , ui(std::make_unique<Ui::ItemTree>())
  , treeModel(this)
{
  ui->setupUi(&frame);
  setupDockWidget();

  treeModel.invisibleRootItem()->appendRow(createTilesetTree());
  treeModel.invisibleRootItem()->appendRow(createDoodadsTree());
  treeModel.invisibleRootItem()->appendRow(createUnitsTree());
  treeModel.invisibleRootItem()->appendRow(createSpritesTree());
  treeModel.invisibleRootItem()->appendRow(createUnitSpritesTree());
  treeModel.invisibleRootItem()->appendRow(createLocationsTree());
  treeModel.invisibleRootItem()->appendRow(createBrushesTree());

  proxyModel.setSourceModel(&treeModel);
  proxyModel.setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  proxyModel.setRecursiveFilteringEnabled(true);

  ui->treeView->setModel(&proxyModel);

  ui->treeView->installEventFilter(this);

  ui->treeView->setExpandsOnDoubleClick(false);

  connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ItemTree::selectionChanged);
  connect(ui->search, &QLineEdit::textChanged, this, &ItemTree::onSearchTextChanged);
  connect(ui->treeView, &QTreeView::clicked, this, &ItemTree::itemClicked);
}

ItemTree::~ItemTree() {}

void ItemTree::update_tileset(Sc::Terrain::Tileset tileset_id) {
  ChkForge::Tileset* tileset = ChkForge::Tileset::fromId(tileset_id);

  tilesetTreeItem->removeRows(0, tilesetTreeItem->rowCount());
  tilesetTreeItem->setText(tileset->getName());

  QList<QStandardItem*> rows;
  for (auto& brush : tileset->getBrushes()) {
    QStandardItem* item = ChkForge::Tree::createTreeItem(brush.getName());

    item->setData(brush.getGroupId(), ChkForge::Tree::ROLE_ID);
    item->setData(Category::CAT_TERRAIN, ChkForge::Tree::ROLE_CATEGORY);
    item->setData(Category::CAT_TERRAIN << 16 | brush.getGroupId(), ChkForge::Tree::ROLE_SEARCHKEY);
    item->setIcon(brush.getIcon());

    rows.emplaceBack(item);
  }
  tilesetTreeItem->appendRows(rows);
}

void ItemTree::update_locations(const std::vector<std::pair<QString, int>>& locations) {
  locationsTreeItem->removeRows(0, locationsTreeItem->rowCount());

  QList<QStandardItem*> rows;
  for (auto [name, id] : locations) {
    QStandardItem* item = ChkForge::Tree::createTreeItem(name);

    item->setEditable(true);

    item->setData(id, ChkForge::Tree::ROLE_ID);
    item->setData(Category::CAT_LOCATION, ChkForge::Tree::ROLE_CATEGORY);
    item->setData(Category::CAT_LOCATION << 16 | id, ChkForge::Tree::ROLE_SEARCHKEY);

    rows.emplaceBack(item);
  }
  locationsTreeItem->appendRows(rows);
}

QStandardItem* ItemTree::createTilesetTree()
{
  this->tilesetTreeItem = ChkForge::Tree::createTreeItem("Tileset");
  return this->tilesetTreeItem;
}

QStandardItem* ItemTree::createDoodadsTree()
{
  //: stat_txt:FIRST_UNIT_STRING-203
  QStandardItem* top = ChkForge::Tree::createTreeItem(tr("Doodads"));

  return top;
}

QStandardItem* ItemTree::createUnitsTree()
{
  //: gluAll:scoreUNITS
  QStandardItem* top = ChkForge::Tree::createTreeItem(tr("Units"));
  ChkForge::Tree::createTreeFromFile(top, "units.txt", CAT_UNIT, [](QStandardItem* itm) {
    itm->setIcon(ChkForge::Icons::getUnitIcon(itm->data(ChkForge::Tree::ROLE_ID).toInt()));
  });
  return top;
}

QStandardItem* ItemTree::createSpritesTree()
{
  QStandardItem* top = ChkForge::Tree::createTreeItem(tr("Sprites"));
  ChkForge::Tree::createTreeFromFile(top, "sprites.txt", CAT_SPRITE, [](QStandardItem* itm) {
    itm->setIcon(QIcon(":/icons/sprite.png"));
    });
  return top;
}

QStandardItem* ItemTree::createUnitSpritesTree()
{
  QStandardItem* top = ChkForge::Tree::createTreeItem(tr("Unit-Sprites"));
  ChkForge::Tree::createTreeFromFile(top, "units.txt", CAT_UNITSPRITE, [](QStandardItem* itm) {
    itm->setIcon(ChkForge::Icons::getUnitIcon(itm->data(ChkForge::Tree::ROLE_ID).toInt()));
    });
  return top;
}

QStandardItem* ItemTree::createLocationsTree()
{
  this->locationsTreeItem = ChkForge::Tree::createTreeItem(tr("Locations"));
  return this->locationsTreeItem;
}

QStandardItem* ItemTree::createBrushesTree()
{
  QStandardItem* top = ChkForge::Tree::createTreeItem(tr("Custom Brushes"));
  QStandardItem* clipboard = ChkForge::Tree::createTreeItem(tr("Clipboard"));
  clipboard->setIcon(QIcon(":/themes/oxygen-icons-png/oxygen/16x16/actions/edit-paste.png"));
  top->appendRow(clipboard);
  return top;
}

// TODO: find an item to select using `treeModel.match(...)` w/ ROLE_SEARCHKEY

void ItemTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  static bool is_changing = false;
  if (is_changing) return;

  if (selected.isEmpty()) {
    emit itemTreeChanged(CAT_NONE, 0);
    return;
  }
  
  for (auto& idx : selected.indexes()) {
    if (idx.data(ChkForge::Tree::ROLE_CATEGORY) != Category::CAT_LOCATION) {
      is_changing = true;
      ui->treeView->selectionModel()->select(ui->treeView->currentIndex(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
      is_changing = false;
      break;
    }
  }

  auto* selected_item = treeModel.itemFromIndex(proxyModel.mapToSource(ui->treeView->currentIndex()));
  auto category = selected_item->data(ChkForge::Tree::ROLE_CATEGORY);
  if (category.isValid()) {
    emit itemTreeChanged(Category(category.toInt()), selected_item->data(ChkForge::Tree::ROLE_ID).toInt());
  }
  // If it's not valid, then assume it's just a folder item that can be expanded or w/e
}

void ItemTree::set_item(Category category, int id)
{
  auto startIndex = treeModel.indexFromItem(treeModel.invisibleRootItem());
  auto result = treeModel.match(startIndex, ChkForge::Tree::ROLE_SEARCHKEY, category << 16 | id, 1, Qt::MatchExactly | Qt::MatchRecursive);
  if (!result.empty()) {
    ui->treeView->selectionModel()->select(proxyModel.mapFromSource(result.front()), QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
  }
  else {
    ui->treeView->selectionModel()->clearSelection();
  }
}

void ItemTree::onSearchTextChanged(const QString& text)
{
  proxyModel.setFilterWildcard(text);
  if (text.isEmpty()) {
    ui->treeView->resetIndentation();
  }
  else {
    ui->treeView->expandAll();
    ui->treeView->setIndentation(0);
  }
}

bool ItemTree::eventFilter(QObject* obj, QEvent* e) {
  if (obj == ui->treeView && e->type() == QEvent::Type::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
    
    switch (keyEvent->key()) {
    case Qt::Key::Key_1:
    case Qt::Key::Key_2:
    case Qt::Key::Key_3:
    case Qt::Key::Key_4:
    case Qt::Key::Key_5:
    case Qt::Key::Key_6:
    case Qt::Key::Key_7:
    case Qt::Key::Key_8:
    case Qt::Key::Key_9:
    case Qt::Key::Key_0:
    case Qt::Key::Key_Minus:
    case Qt::Key::Key_Equal:
      e->ignore();
      return true;
    }
  }
  return false;
}

void ItemTree::itemClicked(const QModelIndex& index) {
  bool isExpanded = ui->treeView->isExpanded(index);
  if (!isExpanded) {
    ui->treeView->expand(index);
  }
  ui->treeView->setExpandsOnDoubleClick(isExpanded);
}
