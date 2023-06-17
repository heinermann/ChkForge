#include "MapContext.h"
#include "mapview.h"

#include <filesystem>
#include <memory>
#include <sstream>
#include <fstream>
#include <QMessageBox>

#include "terrain.h"
#include "layers.h"
#include "PlaceUnitAction.h"

#include "OpenSave.h"

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
  if (!game_paused) {
    openbw_ui.player.next_frame();
  }
  current_layer->logicUpdate();
}

void MapContext::new_map(int tileWidth, int tileHeight, Sc::Terrain::Tileset tileset, int brush, int clutter) {
  chk = std::make_shared<MapFile>(tileset, tileWidth, tileHeight);
  
  apply_brush(map_dimensions(), brush, clutter);
  chkdraft_to_openbw();
  openbw_ui.set_image_data();
  set_unsaved(true);
}

bool MapContext::load_map(std::filesystem::path map_file) {
  map_file.make_preferred();
  if (!chk->load(map_file.string())) {
    QMessageBox::critical(nullptr, QString(), tr("Failed to read the Chk file (chk), not a valid map."));
    return false;
  }

  chkdraft_to_openbw();
  openbw_ui.set_image_data();
  set_unsaved(false);
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

QRect MapContext::map_dimensions() const
{
  return QRect{ 0, 0, tile_width(), tile_height() };
}
int MapContext::tile_width() const
{
  return chk->layers.getTileWidth();
}
int MapContext::tile_height() const
{
  return chk->layers.getTileHeight();
}

Sc::Terrain::Tileset MapContext::tileset() const
{
  return Sc::Terrain::Tileset(openbw_ui.game_st.tileset_index);
}

std::vector<std::pair<QString, int>> MapContext::locations() const {
  std::vector<std::pair<QString, int>> result;

  auto mrgn = chk->layers.mrgn;
  for (size_t i = 0; i < mrgn->numLocations(); i++) {
    // Note: 64 = Anywhere

    auto location = mrgn->getLocation(i);
    if (!location) continue;

    result.emplace_back(get_location_name(i), i);
  }

  return result;
}

QString MapContext::get_location_name(int id) const {
  auto name = chk->strings.getLocationName<RawString>(id);
  if (name) {
    return QString::fromStdString(*name);
  }
  return tr("Location %1").arg(id + 1);
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

  chk->layers.addUnit(unit);

  placeOpenBwUnit(unit);
  // see also create_initial_unit
  //openbw_ui.create_completed_unit(openbw_ui.get_unit_type(static_cast<bwgame::UnitTypes>(unitType)), bwgame::xy{ x, y }, owner);
}

void MapContext::apply_brush(const QRect& rect, int tileGroup, int clutter)
{
  // Source: Modified from Starforge: Ultimate
  QRect clip = rect.intersected(map_dimensions());

  Tileset* tileset = Tileset::fromId(chk->layers.getTileset());

  // TODO: undo
  for (int y = clip.top(); y <= clip.bottom(); ++y) {
    for (int x = clip.left(); x <= clip.right(); ++x) {
      if (x % 2 == 0 || tileGroup < 2) {

        if (x == clip.right() - 1 && x < tile_width() - 1 && tileGroup > 1)
        {
          int next_tile = chk->layers.getTile(x + 1, y);
          if (next_tile / 16 == tileGroup + 1) {
            chk->layers.setTile(x, y, next_tile - 16);
            continue;
          }
        }

        chk->layers.setTile(x, y, tileset->randomTile(tileGroup, clutter));
      }
      else {
        if (x == clip.x() && x > 0) {
          int prev_tile = chk->layers.getTile(x - 1, y);
          if (prev_tile / 16 != tileGroup) {
            chk->layers.setTile(x, y, tileset->randomTile(tileGroup + 1, clutter));
            continue;
          }
        }

        chk->layers.setTile(x, y, chk->layers.getTile(x - 1, y) + 16);
      }

    }
  }
}

std::unordered_set<bwgame::unit_t*> MapContext::find_units(bwgame::rect rect) {
  return unit_finder.find(rect.from.x, rect.from.y, rect.to.x, rect.to.y);
}

void MapContext::select_all() {
  for (bwgame::unit_t* u : placed_units) {
    // TODO: Performance improvement on underlying functions
    openbw_ui.current_selection_add(u);
  }
}

bool MapContext::is_unsaved() {
  return this->has_unsaved_changes;
}

void MapContext::set_unsaved(bool needs_saving) {
  this->has_unsaved_changes = needs_saving;

  for (MapView* view : this->views) {
    view->updateTitle();
  }
}


bool MapContext::save() {
  std::filesystem::path path = chk->getFilePath();
  if (path.empty()) {
    path = OpenSave::getMapSaveFilename();
  }
  return saveAs(path);
}

bool MapContext::saveAs(std::filesystem::path filename) {
  filename.make_preferred();
  bool result = chk->save(filename.string());
  if (result) this->has_unsaved_changes = false;
  return result;
}

std::string MapContext::filename()
{
  return chk->getFileName();
}

std::string MapContext::filepath()
{
  return chk->getFilePath();
}

std::string MapContext::mapname()
{
  auto str = chk->strings.getScenarioName<RawString>();
  if (str) {
    return *str.get();
  }
  return "";
}

QRgb MapContext::player_color(int player_num)
{
  int color_index = openbw_ui.player_color(player_num);

  auto color = openbw_ui.palette_colors[color_index];
  return qRgb(color.r, color.g, color.b);
}

QRect MapContext::toQt(const bwgame::rect& rect) {
  return QRect{ toQt(rect.from), toQt(rect.to) };
}
QPoint MapContext::toQt(const bwgame::xy& pt) {
  return QPoint{ pt.x, pt.y };
}

bwgame::rect MapContext::toBw(const QRect& rect) {
  return bwgame::rect{ toBw(rect.topLeft()), toBw(rect.bottomRight()) };
}
bwgame::xy MapContext::toBw(const QPoint& pt) {
  return bwgame::xy{ pt.x(), pt.y() };
}

void MapContext::set_layer(Layer_t layer_index)
{
  if (current_layer->getLayerId() == layer_index) return;

  current_layer->layerChanged(false);
  current_layer = layer_map.at(layer_index);
  current_layer->layerChanged(true);
}

std::shared_ptr<Layer> MapContext::get_layer()
{
  return current_layer;
}

void MapContext::set_player(int player_id)
{
  current_player = player_id;
}
int MapContext::get_player()
{
  return current_player;
}
void MapContext::set_layer_unit_type(Sc::Unit::Type type)
{
  layer_unit->setPlacementUnitType(type);
}
void MapContext::set_layer_sprite_type(Sc::Sprite::Type type)
{
  layer_sprite->setPlacementSpriteType(type);
}
void MapContext::set_layer_sprite_unit_type(Sc::Unit::Type type)
{
  layer_sprite->setPlacementUnitType(type);
}

void MapContext::placeUnit(int x, int y, Sc::Unit::Type type, int player)
{
  actions.applyAction<PlaceUnitAction>(x, y, type, player);
  emit triggerUndoRedoChanged();
}

void MapContext::start_playback() {
  if (is_testing()) return;
  
  editor_state = TestState::Testing;
  game_paused = false;

  last_edit_layer = Layer_t(get_layer()->getLayerId());
  set_layer(Layer_t::LAYER_GAME_TEST);

  // Resync map to game
  chkdraft_to_openbw();
}

void MapContext::stop_playback() {
  if (!is_testing()) return;
  
  editor_state = TestState::Editing;
  game_paused = false;

  set_layer(last_edit_layer);

  // Resync map to game
  chkdraft_to_openbw();
}

bool MapContext::is_paused() {
  return game_paused;
}

bool MapContext::toggle_pause() {
  if (is_testing()) {
    game_paused = !game_paused;
  }
  return game_paused;
}

void MapContext::frame_advance(int num_frames) {
  if (!is_testing()) return;
  for (int i = 0; i < num_frames; ++i) {
    openbw_ui.player.next_frame();
  }
}

MapContext::TestState MapContext::get_editor_state() {
  return editor_state;
}

bool MapContext::is_testing() {
  return editor_state == MapContext::TestState::Testing;
}
