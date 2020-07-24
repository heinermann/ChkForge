#ifndef TERRAINBRUSH_H
#define TERRAINBRUSH_H

#include "DockWidgetWrapper.h"
#include "ui_terrainbrush.h"

class TerrainBrush : public DockWidgetWrapper<Ui::TerrainBrush>
{
  Q_OBJECT

public:
  explicit TerrainBrush(QWidget *parent = nullptr);
  virtual ~TerrainBrush() {}

private:
};

#endif // TERRAINBRUSH_H
