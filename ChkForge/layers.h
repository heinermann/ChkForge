#pragma once

#include <QMouseEvent>
#include <QObject>
#include <QRect>

#include <vector>
#include <optional>

#include <MappingCoreLib/Sc.h>

class MapView;

namespace ChkForge {
  
  enum Layer_t {
    LAYER_SELECT,
    LAYER_TERRAIN,
    LAYER_DOODAD,
    LAYER_SPRITE,
    LAYER_UNIT,
    LAYER_LOCATION,
    LAYER_FOG
  };

  class Layer : public QObject {
  public:
    Layer(int id) : layer_id(id) {}
    virtual ~Layer() {}

    virtual bool mouseEvent(MapView* map, QMouseEvent* e) = 0;
    virtual void showContextMenu(QWidget* owner, const QPoint& position) = 0;

    virtual void paintOverlay(QWidget* obj, QPainter& painter) = 0;
    virtual void reset() = 0;

    virtual void layerChanged(bool isEntering) = 0;

    int getLayerId();
  protected:
    int layer_id;
  };

  class ThingyPlacer {
  public:
    bool thingyMouseEvent(MapView* map, QMouseEvent* e);
    void paintThingySelectOverlay(QWidget* obj, QPainter& painter);
    
    virtual bool isPlacingThingy() = 0;
    virtual void select_thingy_at(bool double_clicked, bool shift, bool ctrl, const QPoint& position) = 0;
    virtual void select_thingies(bool double_clicked, bool shift, bool ctrl, const QRect& rect) = 0;

  protected:
    MapView* current_view = nullptr;

  private:
    std::optional<QRect> drag_select = std::nullopt;

    void clearDragSelect(MapView* map);
  };


  class SelectLayer : public Layer {
  public:
    SelectLayer() : Layer(Layer_t::LAYER_SELECT) {}
    virtual ~SelectLayer() {}

    virtual bool mouseEvent(MapView* map, QMouseEvent* e) override;
    virtual void showContextMenu(QWidget* owner, const QPoint& position) override;

    virtual void paintOverlay(QWidget* obj, QPainter& painter) override;
    virtual void reset() override;

    virtual void layerChanged(bool isEntering) override;
  private:
  };

  class TerrainLayer : public Layer {
  public:
    TerrainLayer() : Layer(Layer_t::LAYER_TERRAIN) {}
    virtual ~TerrainLayer() {}

    virtual bool mouseEvent(MapView* map, QMouseEvent* e) override;
    virtual void showContextMenu(QWidget* owner, const QPoint& position) override;

    virtual void paintOverlay(QWidget* obj, QPainter& painter) override;
    virtual void reset() override;

    virtual void layerChanged(bool isEntering) override;

    enum class PlacementType {
      BrushIsom,
      BrushRect,
      TilesetIndex,
      IsometricPairs
    };

  private:
    int width = 1, height = 1;
    int clutter = 5;
    PlacementType placement_type = PlacementType::BrushIsom;
    int brush = -1;

    std::vector<int> tile_ids{};
  };

  class DoodadLayer : public Layer {
  public:
    DoodadLayer() : Layer(Layer_t::LAYER_DOODAD) {}
    virtual ~DoodadLayer() {}

    virtual bool mouseEvent(MapView* map, QMouseEvent* e) override;
    virtual void showContextMenu(QWidget* owner, const QPoint& position) override;

    virtual void paintOverlay(QWidget* obj, QPainter& painter) override;
    virtual void reset() override;

    virtual void layerChanged(bool isEntering) override;
  private:
  };

  class SpriteLayer : public Layer, ThingyPlacer {
  public:
    SpriteLayer() : Layer(Layer_t::LAYER_SPRITE) {}
    virtual ~SpriteLayer() {}

    virtual bool mouseEvent(MapView* map, QMouseEvent* e) override;
    virtual void showContextMenu(QWidget* owner, const QPoint& position) override;

    virtual void paintOverlay(QWidget* obj, QPainter& painter) override;
    virtual void reset() override;

    virtual void layerChanged(bool isEntering) override;

    virtual bool isPlacingThingy() override;
    virtual void select_thingy_at(bool double_clicked, bool shift, bool ctrl, const QPoint& position) override;
    virtual void select_thingies(bool double_clicked, bool shift, bool ctrl, const QRect& rect) override;

    void setPlacementSpriteType(Sc::Sprite::Type type);
    void setPlacementUnitType(Sc::Unit::Type type);
  private:
    static const Sc::Sprite::Type NoSprite{ Sc::Sprite::TotalSprites };
    Sc::Sprite::Type placement_type = NoSprite;
    Sc::Unit::Type placement_unit_type = Sc::Unit::Type::NoUnit;
  };

  class UnitLayer : public Layer, ThingyPlacer {
  public:
    UnitLayer() : Layer(Layer_t::LAYER_UNIT) {}
    virtual ~UnitLayer() {}

    virtual bool mouseEvent(MapView* map, QMouseEvent* e) override;
    virtual void showContextMenu(QWidget* owner, const QPoint& position) override;

    virtual void paintOverlay(QWidget* obj, QPainter& painter) override;
    virtual void reset() override;

    virtual void layerChanged(bool isEntering) override;

    virtual bool isPlacingThingy() override;
    virtual void select_thingy_at(bool double_clicked, bool shift, bool ctrl, const QPoint& position) override;
    virtual void select_thingies(bool double_clicked, bool shift, bool ctrl, const QRect& rect) override;

    void setPlacementUnitType(Sc::Unit::Type type);
  private:
    Sc::Unit::Type placement_type = Sc::Unit::Type::NoUnit;
    QPoint place_pos;
    int owner = 0;
  };

  class LocationLayer : public Layer {
  public:
    LocationLayer() : Layer(Layer_t::LAYER_LOCATION) {}
    virtual ~LocationLayer() {}

    virtual bool mouseEvent(MapView* map, QMouseEvent* e) override;
    virtual void showContextMenu(QWidget* owner, const QPoint& position) override;

    virtual void paintOverlay(QWidget* obj, QPainter& painter) override;
    virtual void reset() override;

    virtual void layerChanged(bool isEntering) override;
  private:
  };

  class FogLayer : public Layer {
  public:
    FogLayer() : Layer(Layer_t::LAYER_FOG) {}
    virtual ~FogLayer() {}

    virtual bool mouseEvent(MapView* map, QMouseEvent* e) override;
    virtual void showContextMenu(QWidget* owner, const QPoint& position) override;

    virtual void paintOverlay(QWidget* obj, QPainter& painter) override;
    virtual void reset() override;

    virtual void layerChanged(bool isEntering) override;
  private:
  };

}
