#pragma once
#ifndef CHKFORGE_MAPCONTEXT_H
#define CHKFORGE_MAPCONTEXT_H

#include <unordered_set>
#include <random>

#include <MappingCoreLib/MapFile.h>

#include "../openbw/openbw/ui/ui.h"

#include <QRect>

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

    static std::shared_ptr<MapContext> create();

    void reset();
    void update();

    void new_map(int tileWidth, int tileHeight, Sc::Terrain::Tileset tileset, int brush, int clutter);
    bool load_map(const std::string& map_file_str);

    void add_view(MapView* view);
    void remove_view(MapView* view);
    bool has_one_view() const;

    QRect map_dimensions();
    int tile_width();
    int tile_height();

    void place_unit(Sc::Unit::Type unitType, int owner, int x, int y);
    void apply_brush(const QRect& rect, int tileGroup, int clutter);

    void chkdraft_to_openbw(bool is_editor_mode = true);

    void select_all();

    bool is_unsaved();
    std::string filename();

    std::string mapname();

  public:
    // TODO: Move viewport and screen position stuff out of openbw, to allow for multiple viewports in the same map
    bwgame::ui_functions openbw_ui;

    // TODO: Undo/redo buffer and actions
  private:
    MapFile chk{ Sc::Terrain::Tileset::Badlands, 64, 64 };

    // Other stuff/info (i.e. list of views that are holding the map)
    std::unordered_set<MapView*> views;

    std::random_device rnd_d;
    std::mt19937 random{ rnd_d() };

    bool has_unsaved_changes = false;
  };
}

#endif
