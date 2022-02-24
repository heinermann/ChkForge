#include "MapContext.h"

using namespace ChkForge;

void MapContext::chkdraft_to_openbw()
{
  openbw_ui.reset();

  bwgame::game_load_functions game_load_funcs(openbw_ui.st);
  game_load_funcs.use_map_settings = true;

  openbw_ui.is_editor = editor_state == MapContext::TestState::Editing;
  game_load_funcs.st.is_editor_paused = editor_state == MapContext::TestState::Editing;

  placed_units.clear();
  unit_finder.clear();
  unit_sprite_finder.clear();
  
  auto unit_created_cb = [&](int index, bwgame::unit_t* unit, bool isThg2) {
    if (unit == nullptr) {
      bwgame::warn("Created unit was null @ idx %d", index);
      return;
    }

    if (isThg2) {
      unit_sprite_finder.add(unit);
    }
    else {
      placed_units.insert(unit);
      unit_finder.add(unit);
    }
  };

  std::stringstream map_data;
  chk->write(map_data);
  std::string data = map_data.str();

  game_load_funcs.load_map_data(reinterpret_cast<unsigned char*>(data.data()), data.size(), {}, !openbw_ui.is_editor, unit_created_cb);
}

int MapContext::placeOpenBwUnit(Chk::UnitPtr unit) {

  int type = unit->type;
  int owner = unit->owner;

  if (type >= Sc::Unit::TotalTypes) return -1;
  if (owner >= Sc::Player::Total) owner = 0;

  const bwgame::unit_type_t* obw_unit_type = openbw_ui.get_unit_type(bwgame::UnitTypes(type));

  bwgame::unit_t* new_unit = openbw_ui.create_editor_unit(obw_unit_type, { unit->xc, unit->yc }, owner);
  if (new_unit == nullptr) {
    bwgame::warn("Failed to create unit type %d at (%d, %d)", type, unit->xc, unit->yc);
    return -1;
  }

  placed_units.insert(new_unit);
  unit_finder.add(new_unit);

  return new_unit->index;
}

void MapContext::removeOpenBwUnit(int index)
{
  auto to_remove = openbw_ui.get_unit(index);
  if (to_remove == nullptr) return;

  unit_finder.remove(to_remove);
  placed_units.erase(to_remove);
  placed_unit_sprites.erase(to_remove);
  openbw_ui.remove_unit(to_remove);
}
