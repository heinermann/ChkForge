#include "newmap.h"
#include "ui_newmap.h"

#include <random>
#include <limits>

#include <string>
#include <MappingCoreLib/Sc.h>

#include "terrain.h"

NewMap::NewMap(QWidget *parent) :
  QDialog(parent),
  ui(std::make_unique<Ui::NewMap>())
{
  ui->setupUi(this);

  ui->lst_terrain->setModel(&brush_list_model);

  for (ChkForge::Tileset* tileset : ChkForge::Tileset::getAllTilesets()) {
    ui->cmb_tileset->addItem(tileset->getName(), tileset->getTilesetId());
  }

  ui->cmb_tileset->setCurrentIndex(0);
}

NewMap::~NewMap() {}

void NewMap::on_cmb_tileset_currentIndexChanged(int index)
{
  tileset = ChkForge::Tileset::fromId(index);

  brush_list_model.clear();

  auto brushes = tileset->getBrushes();

  auto root = brush_list_model.invisibleRootItem();
  for (auto& brush : brushes) {
    QStandardItem* item = new QStandardItem(brush.getName());
    item->setData(brush.getGroupId());
    item->setIcon(brush.getIcon());
    
    root->appendRow(item);
  }

  QModelIndex firstBrush = brush_list_model.index(tileset->getDefaultBrushIndex(), 0);
  ui->lst_terrain->setCurrentIndex(firstBrush);
}

void NewMap::accept()
{
  this->tile_width = ui->spn_mapWidth->value();
  this->tile_height = ui->spn_mapHeight->value();
  this->clutter = ui->spn_clutter->value();
  this->brush = brush_list_model.itemFromIndex(ui->lst_terrain->currentIndex())->data().toInt();

  QDialog::accept();
}
