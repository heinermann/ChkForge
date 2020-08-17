#include "layers.h"

using namespace ChkForge;

bool FogLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  return false;
}
void FogLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
}
void FogLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
}
void FogLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
}
void FogLayer::reset()
{
}
void FogLayer::logicUpdate()
{
}
void FogLayer::layerChanged(bool isEntering)
{
}

