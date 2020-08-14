#include "layers.h"

#include <QMouseEvent>
#include <QPainter>
#include <QColor>
#include <QWidget>

#include "mapview.h"
#include "MapContext.h"
#include "../openbw/openbw/bwgame.h"

using namespace ChkForge;


int Layer::getLayerId() {
  return layer_id;
}

void ThingyPlacer::clearDragSelect(MapView* map)
{
  drag_select = std::nullopt;
  map->setCursor(Qt::ArrowCursor);
}


bool ThingyPlacer::thingyMouseEvent(MapView* map, QMouseEvent* e)
{
  this->current_view = map;

  bool double_clicked = e->flags() & Qt::MouseEventCreatedDoubleClick;
  bool shift_pressed = e->modifiers() & Qt::ShiftModifier;
  bool ctrl_pressed = e->modifiers() & Qt::ControlModifier;

  if (isPlacingThingy()) {
    clearDragSelect(map);
  }
  else {
    switch (e->type())
    {
    case QEvent::MouseButtonPress:
      if (e->button() == Qt::LeftButton && !double_clicked) {
        drag_select = map->extendToRect(e->pos());
        return true;
      }
      break;
    case QEvent::MouseButtonDblClick:
      if (e->button() == Qt::LeftButton) {
        map->select_unit_at(true, shift_pressed, ctrl_pressed, e->pos());
        clearDragSelect(map);
        return true;
      }
      break;
    case QEvent::MouseButtonRelease:
      if (e->button() == Qt::LeftButton && this->drag_select) {
        if (std::abs(drag_select->width()) < 2 && std::abs(drag_select->height()) < 2) {
          map->select_unit_at(false, shift_pressed, ctrl_pressed, e->pos());
        }
        else {
          map->select_units(false, shift_pressed, ctrl_pressed, *drag_select);
        }
        clearDragSelect(map);
        return true;
      }
      break;
    case QEvent::MouseMove:
      if (e->buttons() & Qt::LeftButton) {
        if (!drag_select) {
          drag_select = map->extendToRect(e->pos());
        }
        drag_select->setBottomRight(e->pos());
        map->setCursor(Qt::CrossCursor);
        return true;
      }
    }
  }
  return false;
}

void ThingyPlacer::paintThingySelectOverlay(QWidget* obj, QPainter& painter)
{
  // Selection box
  if (drag_select) {
    painter.setPen(QColor(16, 252, 24));
    painter.drawRect(*drag_select);
  }
}



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
void SelectLayer::layerChanged(bool isEntering)
{
}


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
void TerrainLayer::layerChanged(bool isEntering)
{
}


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
void DoodadLayer::layerChanged(bool isEntering)
{
}


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


bool UnitLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  if (isPlacingThingy()) {
    bool shift_pressed = e->modifiers() & Qt::ShiftModifier;
    bool ctrl_pressed = e->modifiers() & Qt::ControlModifier;

    place_pos = e->pos();
    place_pos_bw = map->getMap()->toBw(map->pointToMap(place_pos));

    placement_sprite->position = place_pos_bw;

    switch (e->type())
    {
    case QEvent::MouseButtonPress:
      if (e->button() == Qt::LeftButton) {
        return true;
      }
      if (e->button() == Qt::RightButton) {
        setPlacementUnitType(Sc::Unit::Type::NoUnit);
        return true;
      }
      break;
    case QEvent::MouseButtonDblClick:
      break;
    case QEvent::MouseButtonRelease:
      if (e->button() == Qt::LeftButton) {
        return true;
      }
      break;
    case QEvent::MouseMove:
      if (e->buttons() & Qt::LeftButton) {
        return true;
      }
    }
  }
  else {
    return this->thingyMouseEvent(map, e);
  }
}
void UnitLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
}
void UnitLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
  if (isPlacingThingy()) {

  }
  else {
    this->paintThingySelectOverlay(obj, painter);
  }
}
void UnitLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
  if (isPlacingThingy() && placement_sprite) {
    map->getMap()->openbw_ui.draw_sprite(&*placement_sprite, data, data_pitch, screen_rect);
  }
}
void UnitLayer::reset()
{
  setPlacementUnitType(Sc::Unit::Type::NoUnit);
}
void UnitLayer::layerChanged(bool isEntering)
{
  reset();
}
bool UnitLayer::isPlacingThingy() {
  return placement_type != Sc::Unit::Type::NoUnit;
}
void UnitLayer::setPlacementUnitType(Sc::Unit::Type type)
{
  placement_type = type;
  const bwgame::sprite_type_t* new_sprite_type = nullptr;
  if (type != Sc::Unit::Type::NoUnit) {
    new_sprite_type = &*map->openbw_ui.get_unit_type(bwgame::UnitTypes(type))->flingy->sprite;
  }

  if (placement_sprite && placement_sprite->sprite_type != new_sprite_type) {
    for (auto i = placement_sprite->images.begin(); i != placement_sprite->images.end();) {
      bwgame::image_t* image = &*i++;
      map->openbw_ui.destroy_image(image);
    }
    placement_sprite = std::nullopt;
  }
  
  if (type != Sc::Unit::Type::NoUnit) {
    if (!placement_sprite) {
      placement_sprite = bwgame::sprite_t();
      placement_sprite->sprite_type = nullptr;
    }

    if (placement_sprite->sprite_type != new_sprite_type) {
      map->openbw_ui.initialize_sprite(&*placement_sprite, new_sprite_type, place_pos_bw, owner);
    }
  }
}
void UnitLayer::select_thingy_at(bool double_clicked, bool shift, bool ctrl, const QPoint& position)
{
  current_view->select_unit_at(double_clicked, shift, ctrl, position);
}
void UnitLayer::select_thingies(bool double_clicked, bool shift, bool ctrl, const QRect& rect)
{
  current_view->select_units(double_clicked, shift, ctrl, rect);
}


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
void LocationLayer::layerChanged(bool isEntering)
{
}


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
void FogLayer::layerChanged(bool isEntering)
{
}

