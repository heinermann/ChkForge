#pragma once

#include <QObject>

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

  protected:
    int layer_id;
  };
}
