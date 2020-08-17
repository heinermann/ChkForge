#include "layers.h"

using namespace ChkForge;

bool SelectLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  return false;
}
void SelectLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
}
void SelectLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
}
void SelectLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
}
void SelectLayer::reset()
{
}
void SelectLayer::logicUpdate()
{
}
void SelectLayer::layerChanged(bool isEntering)
{
}
