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
  QFont font = QFont();
  font.setPixelSize(16 * map->getViewScale());
  painter.setFont(font);

  auto chk = map->getMap()->chk;
  auto mrgn = chk->layers.mrgn;
  for (size_t i = 0; i < mrgn->numLocations(); i++) {
    // Note: 64 = Anywhere

    auto location = mrgn->getLocation(i);
    if (location) {
      QRect rct = {
        map->mapToViewPoint({int(location->left), int(location->top)}),
        map->mapToViewPoint({int(location->right), int(location->bottom)}),
      };

      // Skip if none of the location is within the view
      if (!QRect({ 0, 0 }, map->getViewSize()).intersects(rct)) continue;
 
      painter.setPen(Qt::black);
      painter.fillRect(rct, QColor{ 0, 0, 128, 64 });
      painter.drawRect(rct);

      auto name = chk->strings.getLocationName<RawString>(i);
      QPoint textPos = map->mapToViewPoint({ int(location->left) + 4, int(location->top) + 20 });
      
      painter.setPen(QColor{ 16, 252, 24 });
      if (name) {
        painter.drawText(textPos, QString::fromStdString(*name));
      }
      else {
        painter.drawText(textPos, QString("Location %1").arg(i + 1));
      }
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
