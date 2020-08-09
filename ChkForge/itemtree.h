#ifndef ITEMTREE_H
#define ITEMTREE_H

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include <functional>

#include "DockWidgetWrapper.h"
#include "ui_itemtree.h"

class ItemTree : public DockWidgetWrapper<Ui::ItemTree>
{
  Q_OBJECT

public:
  explicit ItemTree(QWidget *parent = nullptr);
  virtual ~ItemTree() {};

  enum Category {
    CAT_NONE,
    CAT_TERRAIN,
    CAT_DOODAD,
    CAT_UNIT,
    CAT_SPRITE,
    CAT_UNITSPRITE,
    CAT_LOCATION,
    CAT_BRUSH
  };

private:
  static const int ROLE_CATEGORY = Qt::UserRole + 1;
  static const int ROLE_ID = Qt::UserRole + 2;
  static const int ROLE_SEARCHKEY = Qt::UserRole + 3;

  QStandardItemModel treeModel;
  QSortFilterProxyModel proxyModel;

  QStandardItem* createTilesetTree();
  QStandardItem* createDoodadsTree();
  QStandardItem* createUnitsTree();
  QStandardItem* createSpritesTree();
  QStandardItem* createUnitSpritesTree();
  QStandardItem* createLocationsTree();
  QStandardItem* createBrushesTree();

  QStandardItem* createTreeItem(const QString& text);

  void txtToTree(QStandardItem* parent, QTextStream &txt, int category, const std::function<void(QStandardItem*)>& item_cb = nullptr);
  void createTreeFromFile(QStandardItem* parent, const QString& resourceName, int category, const std::function<void(QStandardItem*)>& item_cb = nullptr);

  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  void set_item(Category category, int id);

  void onSearchTextChanged(const QString& text);

signals:
  void itemTreeChanged(Category category, int id);

};

#endif // ITEMTREE_H
