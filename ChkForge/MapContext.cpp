#include "MapContext.h"

#include <filesystem>
#include <memory>
#include <sstream>
#include <fstream>
#include <QMessageBox>

#include "terrain.h"

using namespace ChkForge;

MapContext::MapContext()
  : openbw_ui(bwgame::game_player())
{
}

std::shared_ptr<MapContext> MapContext::create() {
  return std::make_shared<MapContext>();
}

void MapContext::reset() {
  openbw_ui.reset();
}

void MapContext::update() {
  openbw_ui.player.next_frame();
}

void MapContext::new_map(int tileWidth, int tileHeight, Sc::Terrain::Tileset tileset, int brush, int clutter) {
  chk = Scenario(tileset, tileWidth, tileHeight);
  
  apply_brush(map_dimensions(), brush, clutter);

  std::stringstream chk_stream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  chk.write(chk_stream);
  size_t data_size = chk_stream.tellp();

  std::vector<uint8_t> raw_chk(data_size);
  chk_stream.read(reinterpret_cast<char*>(raw_chk.data()), data_size);

  bwgame::game_load_functions game_load_funcs(openbw_ui.st);
  game_load_funcs.use_map_settings = true;
  game_load_funcs.load_map_data(raw_chk.data(), raw_chk.size(), nullptr, false);

  openbw_ui.set_image_data();
}

bool MapContext::load_map(const std::string& map_file_str) {
  std::vector<uint8_t> raw_chk;

  if (mpq.open(map_file_str)) {
    if (!mpq.getFile("staredit\\scenario.chk", raw_chk))
    {
      QMessageBox::critical(nullptr, QString(), "Unable to find scenario.chk in MPQ archive.");
      return false;
    }
  }
  else {
    // TODO Check for chk vs replay...

    auto file_size = std::filesystem::file_size(std::filesystem::path(map_file_str));
    raw_chk.resize(file_size);

    std::ifstream chkFile(map_file_str, std::ios_base::binary | std::ios_base::in);
    if (!chkFile.read(reinterpret_cast<char*>(raw_chk.data()), raw_chk.size())) {
      QMessageBox::critical(nullptr, QString(), "Failed to read the Chk file (fstream)");
      return false;
    }
    // otherwise fail
  }

  std::stringstream chk_stream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  std::copy(raw_chk.begin(), raw_chk.end(), std::ostream_iterator<uint8_t>(chk_stream));

  if (!chk.read(chk_stream)) {
    QMessageBox::critical(nullptr, QString(), "Failed to read the Chk file (chk), not a valid map.");
    return false;
  }

  bwgame::game_load_functions game_load_funcs(openbw_ui.st);
  game_load_funcs.use_map_settings = true;
  try {
    game_load_funcs.load_map_data(raw_chk.data(), raw_chk.size(), nullptr, false);
  }
  catch (bwgame::exception& e)
  {
    QMessageBox::critical(nullptr, QString(), QString("OpenBW failure: ") + e.what());
    return false;
  }

  openbw_ui.set_image_data();
  return true;
}

void MapContext::chkdraft_to_openbw(bool is_editor_mode)
{
  openbw_ui.reset();

  bwgame::game_load_functions game_load_funcs(openbw_ui.st);
  game_load_funcs.use_map_settings = true;

  // Sync dimensions
  size_t tile_width = chk.layers.getTileWidth(), tile_height = chk.layers.getTileHeight();
  openbw_ui.st.game->map_tile_width = tile_width;
  openbw_ui.st.game->map_tile_height = tile_height;
  openbw_ui.st.game->map_width = tile_width * 32;
  openbw_ui.st.game->map_height = tile_height * 32;
  openbw_ui.st.game->map_walk_width = tile_width * 4;
  openbw_ui.st.game->map_walk_height = tile_height * 4;

  // Sync tileset
  openbw_ui.st.game->tileset_index = chk.layers.getTileset();

  // Sync player data
  for (size_t i = 0; i != 12; ++i) {
    openbw_ui.st.players[i].controller = chk.players.getSlotType(i);

    Chk::Race srcRace = chk.players.getPlayerRace(i);
    if (srcRace == Chk::Race::UserSelectable || srcRace == Chk::Race::Random) {
      srcRace = static_cast<Chk::Race>(random() % 3);
    }
    switch (srcRace) {
    case Chk::Race::Zerg:
      openbw_ui.st.players[i].race = bwgame::race_t::zerg;
      break;
    case Chk::Race::Protoss:
      openbw_ui.st.players[i].race = bwgame::race_t::protoss;
      break;
    default:
      openbw_ui.st.players[i].race = bwgame::race_t::terran;
      break;
    }
    
    openbw_ui.st.players[i].force = chk.players.getPlayerForce(i);
    openbw_ui.st.players[i].color = chk.players.getPlayerColor(i);
  }

  // Sync string data
  /*
  openbw_ui.st.game->map_strings.clear();
  size_t num_strings = chk.strings.getCapacity(Chk::Scope::Game);
  openbw_ui.st.game->map_strings.reserve(num_strings);
  for (size_t i = 0; i < num_strings; ++i) {
    openbw_ui.st.game->map_strings.emplace_back(*chk.strings.getString<std::string>(i, Chk::Scope::Game));
  }

  openbw_ui.st.game->scenario_name = *chk.strings.getScenarioName<std::string>(Chk::Scope::GameOverEditor);
  openbw_ui.st.game->scenario_description = *chk.strings.getScenarioDescription<std::string>(Chk::Scope::GameOverEditor);
  */

  //--
  game_load_funcs.reset();
  //--

  // Sync MTXM (Tiles) and MASK (Fog)
  openbw_ui.st.game->gfx_tiles.resize(tile_width * tile_height);
  for (size_t i = 0; i < tile_width * tile_height; ++i) {
    openbw_ui.st.game->gfx_tiles.push_back(bwgame::tile_id(chk.layers.mtxm->getTile(i)));
    openbw_ui.st.tiles[i].visible = openbw_ui.st.tiles[i].explored = chk.layers.mask->getFog(i);
  }

  for (size_t i = 0; i != openbw_ui.st.game->gfx_tiles.size(); ++i) {
    bwgame::tile_id tile_id = openbw_ui.st.game->gfx_tiles[i];
    if (tile_id.group_index() >= openbw_ui.cv5().size()) tile_id = {};
    size_t megatile_index = openbw_ui.cv5().at(tile_id.group_index()).mega_tile_index[tile_id.subtile_index()];
    
    int cv5_flags = openbw_ui.cv5().at(tile_id.group_index()).flags & ~(
      bwgame::tile_t::flag_walkable | bwgame::tile_t::flag_unwalkable | bwgame::tile_t::flag_very_high | 
      bwgame::tile_t::flag_middle | bwgame::tile_t::flag_high | bwgame::tile_t::flag_partially_walkable
      );

    openbw_ui.st.tiles_mega_tile_index[i] = (uint16_t)megatile_index;
    openbw_ui.st.tiles[i].flags = openbw_ui.mega_tile_flags().at(megatile_index) | cv5_flags;
    if (tile_id.has_creep()) {
      openbw_ui.st.tiles_mega_tile_index[i] |= 0x8000;
      openbw_ui.st.tiles[i].flags |= bwgame::tile_t::flag_has_creep;
    }
  }

  openbw_ui.tiles_flags_and(0, openbw_ui.st.game->map_tile_height - 2, 5, 1, ~(bwgame::tile_t::flag_walkable | bwgame::tile_t::flag_has_creep | bwgame::tile_t::flag_partially_walkable));
  openbw_ui.tiles_flags_or(0, openbw_ui.st.game->map_tile_height - 2, 5, 1, bwgame::tile_t::flag_unbuildable);
  openbw_ui.tiles_flags_and(openbw_ui.st.game->map_tile_width - 5, openbw_ui.st.game->map_tile_height - 2, 5, 1, ~(bwgame::tile_t::flag_walkable | bwgame::tile_t::flag_has_creep | bwgame::tile_t::flag_partially_walkable));
  openbw_ui.tiles_flags_or(openbw_ui.st.game->map_tile_width - 5, openbw_ui.st.game->map_tile_height - 2, 5, 1, bwgame::tile_t::flag_unbuildable);

  openbw_ui.tiles_flags_and(0, openbw_ui.st.game->map_tile_height - 1, openbw_ui.st.game->map_tile_width, 1, ~(bwgame::tile_t::flag_walkable | bwgame::tile_t::flag_has_creep | bwgame::tile_t::flag_partially_walkable));
  openbw_ui.tiles_flags_or(0, openbw_ui.st.game->map_tile_height - 1, openbw_ui.st.game->map_tile_width, 1, bwgame::tile_t::flag_unbuildable);

  game_load_funcs.regions_create();

  // Sync THG2 (Sprites)
  for (size_t i = 0; i < chk.layers.thg2->numSprites(); ++i) {
    Chk::SpritePtr sprite = chk.layers.thg2->getSprite(i);
    bwgame::xy position{ sprite->xc, sprite->yc };
    if (sprite->isDrawnAsSprite()) {
      auto* obw_sprite_type = openbw_ui.get_sprite_type(bwgame::SpriteTypes(sprite->type));
      openbw_ui.create_thingy(obw_sprite_type, position, sprite->owner);
    }
    else {
      int owner = sprite->owner;
      auto* obw_unit_type = openbw_ui.get_unit_type(bwgame::UnitTypes(sprite->type));
      if (openbw_ui.unit_is_door(obw_unit_type)) owner = 11;
      bwgame::unit_t* unit = openbw_ui.create_initial_unit(obw_unit_type, position, owner);
      if (sprite->flags & 0x80) game_load_funcs.disable_thg2_unit(unit);
    }
  }

  // Sync Unit and Weapon settings
  for (size_t i = 0; i < 228; ++i) {
    auto chkd_type = Sc::Unit::Type(i);
    bwgame::unit_type_t* unit_type = game_load_funcs.get_unit_type(bwgame::UnitTypes(i));

    unit_type->hitpoints = bwgame::fp8::from_raw(chk.properties.getUnitHitpoints(chkd_type));
    unit_type->shield_points = chk.properties.getUnitShieldPoints(chkd_type);
    unit_type->armor = chk.properties.getUnitArmorLevel(chkd_type);
    unit_type->build_time = chk.properties.getUnitBuildTime(chkd_type);
    unit_type->mineral_cost = chk.properties.getUnitMineralCost(chkd_type);
    unit_type->gas_cost = chk.properties.getUnitGasCost(chkd_type);
    unit_type->unit_map_string_index = chk.properties.getUnitNameStringId(chkd_type);

    bwgame::unit_type_t* attacking_type = unit_type->turret_unit_type ? (bwgame::unit_type_t*)unit_type->turret_unit_type : unit_type;
    bwgame::weapon_type_t* grnd_wpn = (bwgame::weapon_type_t*)(const bwgame::weapon_type_t*)attacking_type->ground_weapon;
    bwgame::weapon_type_t* air_wpn = (bwgame::weapon_type_t*)(const bwgame::weapon_type_t*)attacking_type->air_weapon;

    if (grnd_wpn) {
      grnd_wpn->damage_amount = chk.properties.getWeaponBaseDamage(Sc::Weapon::Type(grnd_wpn->id));
      grnd_wpn->damage_bonus = chk.properties.getWeaponUpgradeDamage(Sc::Weapon::Type(grnd_wpn->id));
    }
    if (air_wpn) {
      air_wpn->damage_amount = chk.properties.getWeaponBaseDamage(Sc::Weapon::Type(air_wpn->id));
      air_wpn->damage_bonus = chk.properties.getWeaponUpgradeDamage(Sc::Weapon::Type(air_wpn->id));
    }
  }

  // Sync Upgrade settings
  // TODO

  // Sync Tech settings
  // TODO

  // Sync Upgrade restrictions
  // TODO

  // Sync Tech restrictions
  // TODO

  // Sync Unit restrictions
  // TODO

  // Sync Unit placement
  // TODO

  // Sync unit properties (for triggers)
  // TODO

  // Sync locations (for triggers)
  // TODO

  // Sync Triggers
  // TODO
}

void MapContext::add_view(MapView* view)
{
  views.insert(view);
}

void MapContext::remove_view(MapView* view)
{
  views.erase(view);
}

bool MapContext::has_one_view() const
{
  return views.size() == 1;
}

QRect MapContext::map_dimensions()
{
  return QRect{ 0, 0, tile_width(), tile_height() };
}
int MapContext::tile_width()
{
  return chk.layers.getTileWidth();
}
int MapContext::tile_height()
{
  return chk.layers.getTileHeight();
}

void MapContext::place_unit(Sc::Unit::Type unitType, int owner, int x, int y)
{
  auto unit = std::make_shared<Chk::Unit>();

  unit->classId = 0;
  unit->relationClassId = 0;
  unit->relationFlags = 0;

  unit->type = unitType;
  unit->owner = owner;
  unit->xc = x;
  unit->yc = y;

  unit->hitpointPercent = 100;
  unit->shieldPercent = 100;
  unit->energyPercent = 100;
  unit->hangerAmount = 0;
  unit->resourceAmount = 0;
  unit->validFieldFlags = 0xFFFF;

  unit->stateFlags = 0;
  unit->validStateFlags = 0xFFFF;

  unit->unused = 0;

  // TODO: Undo

  chk.layers.addUnit(unit);

  // see also create_initial_unit
  openbw_ui.create_completed_unit(openbw_ui.get_unit_type(static_cast<bwgame::UnitTypes>(unitType)), bwgame::xy{ x, y }, owner);
}

void MapContext::apply_brush(const QRect& rect, int tileGroup, int clutter)
{
  // Source: Modified from Starforge: Ultimate
  QRect clip = rect.intersected(map_dimensions());

  Tileset* tileset = Tileset::fromId(chk.layers.getTileset());

  // TODO: undo
  for (int y = clip.top(); y <= clip.bottom(); ++y) {
    for (int x = clip.left(); x <= clip.right(); ++x) {
      if (x % 2 == 0 || tileGroup < 2) {

        if (x == clip.right() - 1 && x < tile_width() - 1 && tileGroup > 1)
        {
          int next_tile = chk.layers.getTile(x + 1, y);
          if (next_tile / 16 == tileGroup + 1) {
            chk.layers.setTile(x, y, next_tile - 16);
            continue;
          }
        }

        chk.layers.setTile(x, y, tileset->randomTile(tileGroup, clutter));
      }
      else {
        if (x == clip.x() && x > 0) {
          int prev_tile = chk.layers.getTile(x - 1, y);
          if (prev_tile / 16 != tileGroup) {
            chk.layers.setTile(x, y, tileset->randomTile(tileGroup + 1, clutter));
            continue;
          }
        }

        chk.layers.setTile(x, y, chk.layers.getTile(x - 1, y) + 16);
      }

    }
  }
}

void MapContext::select_all() {
  openbw_ui.current_selection_clear();
  for (auto u = openbw_ui.st.visible_units.begin(); u != openbw_ui.st.visible_units.end(); ++u) {
    // TODO: Performance improvement on underlying functions
    openbw_ui.current_selection_add(&*u);
  }
}
