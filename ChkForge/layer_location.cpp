#include "layers.h"

#include "mapview.h"
#include "MapContext.h"

#include "Utils.h"

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <MappingCoreLib/Chk.h>

#include <QPainter>

using namespace ChkForge;
namespace bgi = boost::geometry::index;

using LocMap = std::pair<BoostRect, int>; // mapping to location id int


/*
1. Move rtree to class level.
  - Track previous location state as location index to LocMap.
  - Replace rtree entries by removing the old location and adding the new location.
2. Click in the same spot to switch to a different overlapping location.
3. Properties dialog.
4. Mouse drag to move location position.
5. Mouse resize edge of location (shift + ctrl modifiers too).
6. Draw order based on which locations overlap others, prioritize not overlapping the location name area.
7. ItemTree cross-interaction.
*/

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
        selectLocations({ idx });
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

void LocationLayer::paintLocationRect(QPainter& painter, QRect rct, bool selected) {
  painter.fillRect(rct, QColor{ 0, 0, 128, 64 });
  if (selected) {
    painter.setPen(QPen(Qt::white, 2));
    rct -= {1, 1, 1, 1};  // margins inward
  }
  else {
    painter.setPen(QPen(Qt::black, 1));
  }
  painter.drawRect(rct);
}

void LocationLayer::paintLocationName(QPainter& painter, const QPoint& pos, const QString& name) {
  QPoint textPos = pos + pt{ 4, 20 };
  painter.setPen(QColor{ 16, 252, 24 });
  painter.drawText(textPos, name);
}

// Note: 64 = Anywhere
void LocationLayer::paintLocation(MapView* map, QPainter& painter, int locationId) {
  Chk::LocationPtr location = map->getMap()->get_location(locationId);
  if (!location) return;

  QRect locationRect = map->mapToViewRect(rect{ location });
  QRect viewRect = { { 0, 0 }, map->getViewSize() };
  if (!viewRect.intersects(locationRect)) return;

  paintLocationRect(painter, locationRect, this->selected_locations.contains(locationId));
  paintLocationName(painter, locationRect.topLeft(), map->getMap()->get_location_name(locationId));
}

void LocationLayer::paintOverlay(MapView* map, QWidget* obj, QPainter& painter)
{
  QFont font = QFont();
  font.setPixelSize(16 * map->getViewScale());
  painter.setFont(font);

  for (size_t i = 0; i < map->getMap()->num_locations(); i++) {
    if (!this->selected_locations.contains(i)) {
      paintLocation(map, painter, i);
    }
  }

  // Draw selected location on top of unselected ones
  for (int idx : this->selected_locations) {
    paintLocation(map, painter, idx);
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

void LocationLayer::selectLocations(const std::vector<int>& locations) {
  this->selected_locations.clear();
  this->selected_locations.insert(locations.begin(), locations.end());
}
