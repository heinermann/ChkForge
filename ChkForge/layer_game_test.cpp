#include "layers.h"
#include "mapview.h"
#include "MapContext.h"

using namespace ChkForge;

bool GameTestLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  bool shift_pressed = e->modifiers() & Qt::ShiftModifier;

  switch (e->type())
  {
  case QEvent::MouseButtonPress:
    if (e->button() == Qt::RightButton) {
      QPoint target = map->pointToMap(e->pos());
      // TODO right click action
      return true;
    }
    break;
  }
  return this->thingyMouseEvent(map, e);
}

void GameTestLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
}

void GameTestLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
  this->paintThingySelectOverlay(obj, painter);
}

void GameTestLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
  // TODO cursor?
}

void GameTestLayer::reset()
{
}

void GameTestLayer::logicUpdate()
{
}

void GameTestLayer::layerChanged(bool isEntering)
{
  reset();
}

bool GameTestLayer::isPlacingThingy() {
  return false;
}

void GameTestLayer::select_thingy_at(bool double_clicked, bool shift, bool ctrl, const QPoint& position)
{
  current_view->select_unit_at(double_clicked, shift, ctrl, position);
}

void GameTestLayer::select_thingies(bool double_clicked, bool shift, bool ctrl, const QRect& rect)
{
  current_view->select_units(double_clicked, shift, ctrl, rect);
}
