#include "layers.h"
#include "mapview.h"
#include "MapContext.h"

using namespace ChkForge;

bool GameTestLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  // TODO right click orders
  return this->thingyMouseEvent(map, e);
}

void GameTestLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
  // TODO Maybe instead of context menu, do right click orders here
}

void GameTestLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
  this->paintThingySelectOverlay(obj, painter);
}

void GameTestLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
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
