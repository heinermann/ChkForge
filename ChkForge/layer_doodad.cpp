#include "layers.h"

using namespace ChkForge;

bool DoodadLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  return false;
}
void DoodadLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
}
void DoodadLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
}
void DoodadLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
}
void DoodadLayer::reset()
{
}
void DoodadLayer::logicUpdate()
{
}
void DoodadLayer::layerChanged(bool isEntering)
{
}
