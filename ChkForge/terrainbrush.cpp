#include "terrainbrush.h"
#include "ui_terrainbrush.h"

TerrainBrush::TerrainBrush(QWidget *parent) :
  DockWidgetWrapper<Ui::TerrainBrush>(tr("Terrain Brush"), parent)
{
}
