#include "layers.h"

using namespace ChkForge;

bool TerrainLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  return false;
}
void TerrainLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
}
void TerrainLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
}
void TerrainLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
}
void TerrainLayer::reset()
{
}
void TerrainLayer::logicUpdate()
{
}
void TerrainLayer::layerChanged(bool isEntering)
{
}
