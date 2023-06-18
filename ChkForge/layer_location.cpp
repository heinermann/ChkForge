#include "layers.h"

#include "mapview.h"
#include "MapContext.h"

#include "Utils.h"

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <QPainter>

using namespace ChkForge;
namespace bgi = boost::geometry::index;

using LocMap = std::pair<BoostRect, int>; // mapping to location id int

bool LocationLayer::mouseEvent(MapView* map, QMouseEvent* e)
{
  QMouseEvent* mouseEvent = reinterpret_cast<QMouseEvent*>(e);

  bool double_clicked = mouseEvent->flags() & Qt::MouseEventCreatedDoubleClick;
  bool shift_pressed = mouseEvent->modifiers() & Qt::ShiftModifier;
  bool ctrl_pressed = mouseEvent->modifiers() & Qt::ControlModifier;
  QPoint map_pos = map->pointToMap(mouseEvent->pos());

  switch (e->type())
  {
  case QEvent::MouseButtonPress:
    if (mouseEvent->button() == Qt::LeftButton) {
      std::vector<LocMap> location_data;

      auto chk = map->getMap()->chk;
      auto mrgn = chk->layers.mrgn;
      for (size_t i = 0; i < mrgn->numLocations(); i++) {
        auto location = mrgn->getLocation(i);
        if (!location) continue;

        location_data.emplace_back(rect{ location }, i);
      }

      bgi::rtree<LocMap, bgi::linear<256>> rtree{ location_data };

      std::vector<LocMap> results;
      rtree.query(bgi::intersects(BoostPt{ pt{ map_pos } }), std::back_inserter(results));

      if (!results.empty()) {
        auto [_, idx] = results.front();
        this->selected_locations.clear();
        this->selected_locations.insert(idx);
      }
    }
    break;
  case QEvent::MouseButtonDblClick:
    break;
  case QEvent::MouseButtonRelease:
    break;
  case QEvent::MouseMove:
    break;
  }
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
    if (!location) continue;

    QRect rct = map->mapToViewRect(rect{ location });

    // Skip if none of the location is within the view
    if (!QRect({ 0, 0 }, map->getViewSize()).intersects(rct)) continue;

    painter.fillRect(rct, QColor{ 0, 0, 128, 64 });

    if (this->selected_locations.contains(i)) {
      painter.setPen(QPen(Qt::white, 2));
      rct -= {1, 1, 1, 1};  // margins
    }
    else {
      painter.setPen(QPen(Qt::black, 1));
    }
    painter.drawRect(rct);

    QPoint textPos = map->mapToViewPoint(rect{ location }.topLeft() + pt{ 4, 20 });
    QString name = map->getMap()->get_location_name(i);

    painter.setPen(QColor{ 16, 252, 24 });
    painter.drawText(textPos, name);
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
