#pragma once
#ifndef CHKFORGE_MAPCONTEXT_H
#define CHKFORGE_MAPCONTEXT_H

#include <unordered_set>

#include <MappingCoreLib/MpqFile.h>
#include <MappingCoreLib/Scenario.h>

#include "../openbw/openbw/ui/ui.h"

class MapView;

namespace ChkForge
{
  /**

  The purpose of the map context is to
  1. Connect ChkDraft and OpenBW with each other.
  2. Hold any additional metadata that ChkForge will use.

  */
  class MapContext
  {
  public:
    MapContext();

    void reset();
    void update();

    bool load_map(const std::string& map_file_str);

    // TODO: Move viewport and screen position stuff out of openbw, to allow for multiple viewports in the same map
    bwgame::ui_functions openbw_ui;

    void add_view(MapView* view);
    void remove_view(MapView* view);

    bool has_one_view();

    void place_unit(Sc::Unit::Type unitType, int owner, int x, int y);

    // TODO: Undo/redo buffer and actions
  private:
    MpqFile mpq;
    Scenario chk;

    // Other stuff/info (i.e. list of views that are holding the map)
    std::unordered_set<MapView*> views;
  };
}

#endif
