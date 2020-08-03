#include "MapContext.h"

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

void MapContext::load_map(const std::string& map_file_str) {
  openbw_ui.load_map_file(map_file_str);

  openbw_ui.set_image_data();
}
