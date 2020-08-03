#include "MapContext.h"

#include <filesystem>
#include <memory>
#include <sstream>
#include <fstream>
//#include "mapview.h"


using namespace ChkForge;

MapContext::MapContext()
  : openbw_ui(bwgame::game_player())
{
}

void MapContext::reset() {
  openbw_ui.reset();
}

void MapContext::update() {
  openbw_ui.player.next_frame();
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

  chk.layers.addUnit(unit);
  
  // see also create_initial_unit
  openbw_ui.create_completed_unit(openbw_ui.get_unit_type(static_cast<bwgame::UnitTypes>(unitType)), bwgame::xy{ x, y }, owner);
}

void MapContext::add_view(MapView* view)
{
  views.insert(view);
}

void MapContext::remove_view(MapView* view)
{
  views.erase(view);
}

bool MapContext::has_one_view()
{
  return views.size() == 1;
}
