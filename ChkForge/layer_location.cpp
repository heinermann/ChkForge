#include "layers.h"

using namespace ChkForge;

bool LocationLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  return false;
}
void LocationLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
}
void LocationLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
}
void LocationLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
}
void LocationLayer::reset()
{
}
void LocationLayer::logicUpdate()
{
}
void LocationLayer::layerChanged(bool isEntering)
{
}
