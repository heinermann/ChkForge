#pragma once
#ifndef CHKFORGE_MAPCONTEXT_H
#define CHKFORGE_MAPCONTEXT_H

#include <unordered_set>
#include <random>
#include <memory>
#include <vector>
#include <filesystem>

#include <MappingCoreLib/MapFile.h>
#include <MappingCoreLib/Sc.h>

#include "../openbw/openbw/ui/ui.h"
#include "UnitFinder.h"

#include <QObject>
#include <QRect>
#include <QTimer>
#include <QRgb>

#include "layers.h"
#include "UndoManager.h"

class MapView;

namespace ChkForge
{
  /**

  The purpose of the map context is to
  1. Connect ChkDraft and OpenBW with each other.
  2. Hold any additional metadata that ChkForge will use.

  */
  class MapContext : public QObject
  {
    Q_OBJECT

  public:
    enum class TestState {
      Editing,
      Testing
    };

    MapContext();

    static std::shared_ptr<MapContext> create();

    void reset();
    void update();

    void new_map(int tileWidth, int tileHeight, Sc::Terrain::Tileset tileset, int brush, int clutter);
    bool load_map(std::filesystem::path map_file);

    void add_view(MapView* view);
    void remove_view(MapView* view);
    bool has_one_view() const;

    QRect map_dimensions() const;
    int tile_width() const;
    int tile_height() const;

    Sc::Terrain::Tileset tileset() const;
    std::vector<std::pair<QString, int>> locations() const;
    QString get_location_name(int id) const;

    void place_unit(Sc::Unit::Type unitType, int owner, int x, int y);
    void apply_brush(const QRect& rect, int tileGroup, int clutter);

    void chkdraft_to_openbw();

    std::unordered_set<bwgame::unit_t*> find_units(bwgame::rect rect);
    void select_all();

    bool is_unsaved();
    void set_unsaved(bool needs_saving = true);
    bool save();
    bool saveAs(std::filesystem::path filename);
    std::string filename();
    std::string filepath();

    std::string mapname();

    QRgb player_color(int player_num);

    void set_layer(Layer_t layer_index);
    std::shared_ptr<Layer> get_layer();

    void set_player(int player_id);
    int get_player();
    void set_layer_unit_type(Sc::Unit::Type type);
    void set_layer_sprite_type(Sc::Sprite::Type type);
    void set_layer_sprite_unit_type(Sc::Unit::Type type);

    void placeUnit(int x, int y, Sc::Unit::Type type, int player);

    int placeOpenBwUnit(Chk::UnitPtr unit);
    void removeOpenBwUnit(int index);

    void start_playback();
    void stop_playback();
    bool is_paused();
    bool toggle_pause();
    void frame_advance(int num_frames = 1);

    TestState get_editor_state();
    bool is_testing();

  public:
    std::shared_ptr<MapFile> chk = std::make_shared<MapFile>(Sc::Terrain::Tileset::Badlands, 64, 64);
    bwgame::ui_functions openbw_ui;

    UndoManager actions{ this };
  private:
    // Other stuff/info (i.e. list of views that are holding the map)
    std::unordered_set<MapView*> views;

    std::random_device rnd_d;
    std::mt19937 random{ rnd_d() };

    bool has_unsaved_changes = false;
    bool game_paused = false;
    TestState editor_state = TestState::Editing;
    Layer_t last_edit_layer = Layer_t::LAYER_SELECT;

    QTimer update_timer{};

    std::shared_ptr<SelectLayer> layer_select = std::make_shared<SelectLayer>(this);
    std::shared_ptr<TerrainLayer> layer_terrain = std::make_shared<TerrainLayer>(this);
    std::shared_ptr<DoodadLayer> layer_doodad = std::make_shared<DoodadLayer>(this);
    std::shared_ptr<SpriteLayer> layer_sprite = std::make_shared<SpriteLayer>(this);
    std::shared_ptr<UnitLayer> layer_unit = std::make_shared<UnitLayer>(this);
    std::shared_ptr<LocationLayer> layer_location = std::make_shared<LocationLayer>(this);
    std::shared_ptr<FogLayer> layer_fog = std::make_shared<FogLayer>(this);
    std::shared_ptr<GameTestLayer> layer_game_test = std::make_shared<GameTestLayer>(this);

    std::shared_ptr<Layer> current_layer = layer_select;

    std::map<Layer_t, std::shared_ptr<Layer>> layer_map = {
      {Layer_t::LAYER_SELECT, layer_select},
      {Layer_t::LAYER_TERRAIN, layer_terrain},
      {Layer_t::LAYER_DOODAD, layer_doodad},
      {Layer_t::LAYER_SPRITE, layer_sprite},
      {Layer_t::LAYER_UNIT, layer_unit},
      {Layer_t::LAYER_LOCATION, layer_location},
      {Layer_t::LAYER_FOG, layer_fog},
      {Layer_t::LAYER_GAME_TEST, layer_game_test},
    };

    int current_player = 0;

    std::unordered_set<bwgame::unit_t*> placed_units;
    std::unordered_set<bwgame::unit_t*> placed_unit_sprites;
    UnitFinder unit_finder;
    UnitFinder unit_sprite_finder;

  signals:
    void triggerUndoRedoChanged();
  };
}

#endif
