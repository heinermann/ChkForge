#include "MapContext.h"

#include <filesystem>
#include <memory>
#include <sstream>
#include <fstream>

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
      // Log Failed state
      return false;
    }
  }
  else {
    // TODO Check for chk vs replay...

    auto file_size = std::filesystem::file_size(std::filesystem::path(map_file_str));
    raw_chk.resize(file_size);

    std::ifstream chkFile(map_file_str, std::ios_base::binary | std::ios_base::in);
    if (!chkFile.read(reinterpret_cast<char*>(raw_chk.data()), raw_chk.size())) {
      // read failed
    }
    // otherwise fail
  }

  std::stringstream chk_stream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  std::copy(raw_chk.begin(), raw_chk.end(), std::ostream_iterator<uint8_t>(chk_stream));

  if (!chk.read(chk_stream)) {
    // fail
    return false;
  }

  bwgame::game_load_functions game_load_funcs(openbw_ui.st);
  game_load_funcs.use_map_settings = true;
  game_load_funcs.load_map_data(raw_chk.data(), raw_chk.size(), nullptr, false);

  openbw_ui.set_image_data();
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
