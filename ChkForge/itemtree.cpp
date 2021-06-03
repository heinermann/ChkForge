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

#include "icons.h"


ItemTree::ItemTree(QWidget *parent)
  : DockWidgetWrapper<Ui::ItemTree>(tr("Item Tree"), parent)
  , treeModel(this)
{
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


QStandardItem* ItemTree::createTilesetTree()
{
  QStandardItem* top = createTreeItem("Tileset");
  createTreeFromFile(top, "terrain.txt", CAT_TERRAIN);
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
  createTreeFromFile(top, "units.txt", CAT_UNIT, [](QStandardItem* itm) {
    itm->setIcon(ChkForge::Icons::getUnitIcon(itm->data(ROLE_ID).toInt()));
  });
  return top;
}

QStandardItem* ItemTree::createSpritesTree()
{
  QStandardItem* top = createTreeItem(tr("Sprites"));
  createTreeFromFile(top, "sprites.txt", CAT_SPRITE, [](QStandardItem* itm) {
    itm->setIcon(QIcon(":/icons/sprite.png"));
    });
  return top;
}

QStandardItem* ItemTree::createUnitSpritesTree()
{
  QStandardItem* top = createTreeItem(tr("Unit-Sprites"));
  createTreeFromFile(top, "units.txt", CAT_UNITSPRITE, [](QStandardItem* itm) {
    itm->setIcon(ChkForge::Icons::getUnitIcon(itm->data(ROLE_ID).toInt()));
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

void ItemTree::createTreeFromFile(QStandardItem* parent, const QString& resourceName, int category, const std::function<void(QStandardItem*)>& item_cb)
{
  QFile treeFile(":/Trees/" + resourceName);
  if (!treeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::critical(this, tr("Error"), tr("Unable to load Trees/%1: %2").arg(resourceName).arg(treeFile.errorString()));
  }
  QTextStream contents(treeFile.readAll());
  txtToTree(parent, contents, category, item_cb);
}

void ItemTree::txtToTree(QStandardItem *parent, QTextStream &txt, int category, const std::function<void(QStandardItem*)>& item_cb)
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
      txtToTree(newGroup, txt, category, item_cb);
      parent->appendRow(newGroup);
    }
    else
    {
      auto valueMatch = VALUE_REGEX.match(line);
      if (valueMatch.hasMatch())
      {
        QStandardItem* item = createTreeItem(valueMatch.captured("name"));
        
        int id = valueMatch.captured("id").toInt();
        item->setData(id, ROLE_ID);
        item->setData(category, ROLE_CATEGORY);
        item->setData(category << 16 | id, ROLE_SEARCHKEY);

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

// TODO: find an item to select using `treeModel.match(...)` w/ ROLE_SEARCHKEY

void ItemTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  if (selected.isEmpty()) {
    emit itemTreeChanged(CAT_NONE, 0);
    return;
  }
  
  auto* selected_item = treeModel.itemFromIndex(proxyModel.mapToSource(selected.front().topLeft()));
  auto category = selected_item->data(ROLE_CATEGORY);
  if (category.isValid()) {
    emit itemTreeChanged(Category(category.toInt()), selected_item->data(ROLE_ID).toInt());
  }
  // If it's not valid, then assume it's just a folder item that can be expanded or w/e
}

void ItemTree::set_item(Category category, int id)
{
  auto startIndex = treeModel.indexFromItem(treeModel.invisibleRootItem());
  auto result = treeModel.match(startIndex, ROLE_SEARCHKEY, category << 16 | id, 1, Qt::MatchExactly | Qt::MatchRecursive);
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
