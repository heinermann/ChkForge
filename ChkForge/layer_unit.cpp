#include "layers.h"
#include "mapview.h"
#include "MapContext.h"

#include "Utils.h"

using namespace ChkForge;

bool UnitLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  if (isPlacingThingy()) {
    bool shift_pressed = e->modifiers() & Qt::ShiftModifier;
    bool ctrl_pressed = e->modifiers() & Qt::ControlModifier;

    place_pos = e->pos();
    place_pos_bw = pt{ map->pointToMap(place_pos) };

    placement_sprite->position = place_pos_bw;

    switch (e->type())
    {
    case QEvent::MouseButtonPress:
      if (e->button() == Qt::LeftButton) {
        map->getMap()->placeUnit(place_pos_bw.x, place_pos_bw.y, this->placement_type, map->getMap()->get_player());
        return true;
      }
      if (e->button() == Qt::RightButton) {
        reset();
        map->setItemTreeSelection(ItemTree::CAT_NONE, -1);
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
    return false;
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
void UnitLayer::logicUpdate()
{
  if (placement_sprite) {
    placement_sprite->owner = map->get_player();
    map->openbw_ui.iscript_execute_sprite(&*placement_sprite);
  }
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
  const bwgame::unit_type_t* new_unit_type = nullptr;
  if (type != Sc::Unit::Type::NoUnit) {
    new_unit_type = map->openbw_ui.get_unit_type(bwgame::UnitTypes(type));
    new_sprite_type = &*new_unit_type->flingy->sprite;
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
      map->openbw_ui.initialize_sprite(&*placement_sprite, new_sprite_type, place_pos_bw, map->get_player());

      if (map->openbw_ui.ut_building(new_unit_type)) {
        map->openbw_ui.sprite_run_anim(&*placement_sprite, bwgame::iscript_anims::Built);
      }
      else if (new_unit_type->turret_unit_type) {
        map->openbw_ui.sprite_run_anim(&*placement_sprite, bwgame::iscript_anims::StarEditInit);
      }
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
