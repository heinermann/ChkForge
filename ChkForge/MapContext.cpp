#include "MapContext.h"
#include "mapview.h"

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
  connect(&update_timer, &QTimer::timeout, this, &MapContext::update);
  update_timer.start(42);
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
  chk = MapFile(tileset, tileWidth, tileHeight);
  
  apply_brush(map_dimensions(), brush, clutter);
  chkdraft_to_openbw(true);
  openbw_ui.set_image_data();
}

bool MapContext::load_map(const std::string& map_file_str) {
  std::string dumb_path_limitation_string = map_file_str;
  std::replace(std::begin(dumb_path_limitation_string), std::end(dumb_path_limitation_string), '/', '\\');
  if (!chk.load(dumb_path_limitation_string)) {
    QMessageBox::critical(nullptr, QString(), "Failed to read the Chk file (chk), not a valid map.");
    return false;
  }

  chkdraft_to_openbw(true);
  openbw_ui.set_image_data();
  return true;
}

void MapContext::add_view(MapView* view)
{
  views.insert(view);
  connect(&update_timer, &QTimer::timeout, view, &MapView::updateSurface);
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

bool MapContext::is_unsaved()
{
  return this->has_unsaved_changes;
}

std::string MapContext::filename()
{
  return chk.getFileName();
}

std::string MapContext::mapname()
{
  return *chk.strings.getScenarioName<RawString>().get();
}
