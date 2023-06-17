#include "layers.h"

#include "mapview.h"
#include "MapContext.h"

#include <QPainter>

using namespace ChkForge;

bool LocationLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  return false;
}
void LocationLayer::showContextMenu(QWidget* owner, const QPoint& position)
{
}
void LocationLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
  auto mrgn = map->getMap()->chk->layers.mrgn;
  for (size_t i = 0; i < mrgn->numLocations(); i++) {
    auto location = mrgn->getLocation(i);
    if (location) {
      QRect rct = {
        map->mapToViewPoint({int(location->left), int(location->top)}),
        map->mapToViewPoint({int(location->right), int(location->bottom)}),
      };
      painter.setPen(Qt::white);
      painter.fillRect(rct, QColor{ 0, 0, 128, 64 });
    }
  }
}
void LocationLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
}
void LocationLayer::reset()
{
}
void LocationLayer::logicUpdate()
{
}
void LocationLayer::layerChanged(bool isEntering)
{
}
