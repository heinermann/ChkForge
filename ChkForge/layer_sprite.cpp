#include "layers.h"

using namespace ChkForge;

bool SpriteLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  return this->thingyMouseEvent(map, e);
}
void SpriteLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
}
void SpriteLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
  this->paintThingySelectOverlay(obj, painter);
}
void SpriteLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
}
void SpriteLayer::reset()
{
}
void SpriteLayer::logicUpdate()
{
}
void SpriteLayer::layerChanged(bool isEntering)
{
}
bool SpriteLayer::isPlacingThingy() {
  return placement_type != NoSprite && placement_unit_type != Sc::Unit::Type::NoUnit;
}
void SpriteLayer::setPlacementSpriteType(Sc::Sprite::Type type)
{
  placement_type = type;
}
void SpriteLayer::setPlacementUnitType(Sc::Unit::Type type)
{
  placement_unit_type = type;
}
void SpriteLayer::select_thingy_at(bool double_clicked, bool shift, bool ctrl, const QPoint& position)
{
}
void SpriteLayer::select_thingies(bool double_clicked, bool shift, bool ctrl, const QRect& rect)
{
}

