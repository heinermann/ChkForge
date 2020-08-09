#include "layers.h"

#include <QMouseEvent>
#include <QPainter>
#include <QColor>
#include <QWidget>

#include "mapview.h"

using namespace ChkForge;


bool ThingyPlacer::thingyMouseEvent(MapView* map, QMouseEvent* e)
{
  this->current_view = map;

  bool double_clicked = e->flags() & Qt::MouseEventCreatedDoubleClick;
  bool shift_pressed = e->modifiers() & Qt::ShiftModifier;
  bool ctrl_pressed = e->modifiers() & Qt::ControlModifier;

  if (isPlacingThingy()) {
    drag_select = std::nullopt;
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
        drag_select = std::nullopt;
        return true;
      }
      break;
    case QEvent::MouseButtonRelease:
      if (e->button() == Qt::LeftButton && this->drag_select) {
        if (drag_select->width() < 2 && drag_select->height() < 2) {
          map->select_unit_at(false, shift_pressed, ctrl_pressed, e->pos());
        }
        else {
          map->select_units(false, shift_pressed, ctrl_pressed, *drag_select);
        }
        drag_select = std::nullopt;
        return true;
      }
      break;
    case QEvent::MouseMove:
      if (e->buttons() & Qt::LeftButton) {
        if (!drag_select) {
          drag_select = map->extendToRect(e->pos());
        }
        drag_select->setBottomRight(e->pos());
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
void SelectLayer::paintOverlay(QWidget* obj, QPainter& painter)
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
void TerrainLayer::paintOverlay(QWidget* obj, QPainter& painter)
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
void DoodadLayer::paintOverlay(QWidget* obj, QPainter& painter)
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
void SpriteLayer::paintOverlay(QWidget* obj, QPainter& painter)
{
  this->paintThingySelectOverlay(obj, painter);
}
void SpriteLayer::reset()
{
}
void SpriteLayer::layerChanged(bool isEntering)
{
}
bool SpriteLayer::isPlacingThingy() {
  return placement_type != NoSprite;
}
void SpriteLayer::setPlacementSpriteType(Sc::Sprite::Type type)
{
  placement_type = type;
}
void SpriteLayer::select_thingy_at(bool double_clicked, bool shift, bool ctrl, const QPoint& position)
{
}
void SpriteLayer::select_thingies(bool double_clicked, bool shift, bool ctrl, const QRect& rect)
{
}


bool UnitLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  return this->thingyMouseEvent(map, e);
}
void UnitLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
}
void UnitLayer::paintOverlay(QWidget* obj, QPainter& painter)
{
  this->paintThingySelectOverlay(obj, painter);
}
void UnitLayer::reset()
{
}
void UnitLayer::layerChanged(bool isEntering)
{
}
bool UnitLayer::isPlacingThingy() {
  return placement_type != Sc::Unit::Type::NoUnit;
}
void UnitLayer::setPlacementUnitType(Sc::Unit::Type type)
{
  placement_type = type;
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
void LocationLayer::paintOverlay(QWidget* obj, QPainter& painter)
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
void FogLayer::paintOverlay(QWidget* obj, QPainter& painter)
{
}
void FogLayer::reset()
{
}
void FogLayer::layerChanged(bool isEntering)
{
}

