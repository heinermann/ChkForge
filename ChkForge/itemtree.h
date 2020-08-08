#ifndef ITEMTREE_H
#define ITEMTREE_H

#include <QStandardItemModel>

#include <functional>

#include "DockWidgetWrapper.h"
#include "ui_itemtree.h"

class ItemTree : public DockWidgetWrapper<Ui::ItemTree>
{
  Q_OBJECT

public:
  explicit ItemTree(QWidget *parent = nullptr);
  virtual ~ItemTree() {};

private:
  QStandardItemModel treeModel;

  QStandardItem* createTilesetTree();
  QStandardItem* createDoodadsTree();
  QStandardItem* createUnitsTree();
  QStandardItem* createSpritesTree();
  QStandardItem* createUnitSpritesTree();
  QStandardItem* createLocationsTree();
  QStandardItem* createBrushesTree();

  QStandardItem* createTreeItem(const QString& text);

  void txtToTree(QStandardItem* parent, QTextStream &txt, const std::function<void(QStandardItem*)>& item_cb = nullptr);
  void createTreeFromFile(QStandardItem* parent, const QString& resourceName, const std::function<void(QStandardItem*)>& item_cb = nullptr);
};

#endif // ITEMTREE_H
