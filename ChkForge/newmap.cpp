#include "newmap.h"
#include "ui_newmap.h"

#include <random>
#include <limits>

#include <string>
#include <MappingCoreLib/Sc.h>

#include "terrain.h"

NewMap::NewMap(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::NewMap)
{
  ui->setupUi(this);

  ui->spn_seed->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

  auto r_engine = std::default_random_engine();
  auto random = std::uniform_int_distribution<int>();
  ui->spn_seed->setValue(random(r_engine));

  ui->lst_terrain->setModel(&brush_list_model);

  for (ChkForge::Tileset* tileset : ChkForge::Tileset::getAllTilesets()) {
    ui->cmb_tileset->addItem(tileset->getName(), tileset->getId());
  }

  ui->cmb_tileset->setCurrentIndex(0);
}

NewMap::~NewMap()
{
  delete ui;
}

void NewMap::on_cmb_tileset_currentIndexChanged(int index)
{
  tileset = ChkForge::Tileset::fromId(index);

  brush_list_model.clear();

  auto brushes = tileset->getBrushes();
  auto brushValues = tileset->getBrushValues();

  auto root = brush_list_model.invisibleRootItem();
  for (int i = 0; i < brushes.size(); ++i) {
    QStandardItem* item = new QStandardItem(brushes[i]);
    item->setData(brushValues[i]);
    
    root->appendRow(item);
  }

  QModelIndex firstBrush = brush_list_model.index(tileset->getDefaultBrushIndex(), 0);
  ui->lst_terrain->setCurrentIndex(firstBrush);
}
