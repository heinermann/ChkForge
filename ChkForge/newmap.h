#ifndef NEWMAP_H
#define NEWMAP_H

#include <QDialog>
#include <QStandardItemModel>

#include <memory>

#include "terrain.h"

namespace Ui {
  class NewMap;
}

class NewMap : public QDialog
{
  Q_OBJECT

public:
  explicit NewMap(QWidget *parent = nullptr);
  ~NewMap();

  ChkForge::Tileset* tileset = &ChkForge::Tileset::Badlands;
  int brush = 0;
  int generator = 0;
  int clutter = 5;
  int seed = 0;

  int tile_width = 128;
  int tile_height = 128;

private:
  Ui::NewMap *ui;

  QStandardItemModel brush_list_model;


private slots:
  void on_cmb_tileset_currentIndexChanged(int index);
};

#endif // NEWMAP_H
