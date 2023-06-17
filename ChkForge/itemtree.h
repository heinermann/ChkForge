#ifndef ITEMTREE_H
#define ITEMTREE_H

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QTimer>

#include <MappingCoreLib/Sc.h>

#include "DockWidgetWrapper.h"

namespace Ui {
  class ItemTree;
}

class ItemTree : public DockWidgetWrapper
{
  Q_OBJECT

public:
  explicit ItemTree(QWidget *parent = nullptr);
  virtual ~ItemTree() override;

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

  void set_item(Category category, int id);
  void update_tileset(Sc::Terrain::Tileset tileset);
  void update_locations(const std::vector<std::pair<QString, int>>& locations);

private:
  std::unique_ptr<Ui::ItemTree> ui;

  QStandardItemModel treeModel;
  QSortFilterProxyModel proxyModel;

  QStandardItem* tilesetTreeItem = nullptr;
  QStandardItem* locationsTreeItem = nullptr;

private:
  QStandardItem* createTilesetTree();
  QStandardItem* createDoodadsTree();
  QStandardItem* createUnitsTree();
  QStandardItem* createSpritesTree();
  QStandardItem* createUnitSpritesTree();
  QStandardItem* createLocationsTree();
  QStandardItem* createBrushesTree();

  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void onSearchTextChanged(const QString& text);
  virtual bool eventFilter(QObject* obj, QEvent* e) override;
  void itemClicked(const QModelIndex& index);

signals:
  void itemTreeChanged(Category category, int id);
};

#endif // ITEMTREE_H
