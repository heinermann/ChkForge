#include "terrainbrush.h"
#include "ui_terrainbrush.h"

TerrainBrush::TerrainBrush(QWidget *parent)
  : DockWidgetWrapper(tr("Terrain Brush"), parent)
  , ui(std::make_unique<Ui::TerrainBrush>())
{
  ui->setupUi(&frame);
  setupDockWidget();
}

TerrainBrush::~TerrainBrush() {}
