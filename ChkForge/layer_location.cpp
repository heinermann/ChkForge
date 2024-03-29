#include "layers.h"

#include "mapview.h"
#include "MapContext.h"

#include "Utils.h"

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <MappingCoreLib/Chk.h>

#include <QPainter>
#include <ranges>

using namespace ChkForge;
namespace bgi = boost::geometry::index;


/*
1. Properties dialog.
2. Mouse drag to move location position.
3. Mouse resize edge of location (shift + ctrl modifiers too).
4. ItemTree cross-interaction.
5. Arrow keys (+modifiers) to move/resize selected locations.

Mouse resize modifiers:
  None: Align to grid
  Shift: Align to pixel
  Ctrl: Align to unit boundary
  Ctrl+Shift: Align to terrain

Mouse move modifiers:
  None: Align to grid
  Shift: Align to pixel
  Ctrl: Align to center of unit
  Ctrl+Shift: Align location edge to terrain?

Arrow key modifiers:
  None: Align to grid
  Shift: Align to pixel
  Ctrl: Resize mode (grid)
  Ctrl+Shift: Resize mode (pixel)
*/

bool LocationLayer::mouseEvent(MapView* view, QMouseEvent* e)
{
  QMouseEvent* mouseEvent = reinterpret_cast<QMouseEvent*>(e);

  bool double_clicked = mouseEvent->flags() & Qt::MouseEventCreatedDoubleClick;
  bool shift_pressed = mouseEvent->modifiers() & Qt::ShiftModifier;
  bool ctrl_pressed = mouseEvent->modifiers() & Qt::ControlModifier;
  QPoint map_pos = view->pointToMap(mouseEvent->pos());

  switch (e->type())
  {
  case QEvent::MouseButtonPress:
    if (e->button() == Qt::LeftButton && !double_clicked) {
      location_drag = view->extendToRect(e->pos());
      return true;
    }
    break;
  case QEvent::MouseButtonDblClick:
    if (e->button() == Qt::LeftButton) {
      location_drag = std::nullopt;
      view->setCursor(Qt::ArrowCursor);
      return true;
    }
    break;
  case QEvent::MouseButtonRelease:
    if (mouseEvent->button() == Qt::LeftButton && location_drag) {
      if (rect{ *location_drag }.distance() < 4) {

        // Clear location cycle candidate if we click somewhere else
        if (pt{ map_pos - last_location_pt }.distance() > 4) {
          last_location_candidate = 0;
        }
        last_location_pt = map_pos;

        // Query the rtree to get all the locations, sort by smallest first
        std::vector<LocMap> results;
        rtree.query(bgi::intersects(BoostPt{ pt{ map_pos } }), std::back_inserter(results));
        std::ranges::sort(results, [](auto& a, auto& b) { return boost::geometry::area(a.first) < boost::geometry::area(b.first); });
        
        // Get only the location IDs from the results
        auto tmp_select = results | std::views::values;
        std::vector<int> tmp_out{ tmp_select.begin(), tmp_select.end() };

        // Select the result
        // FIXME some jank to avoid destroying intellisense
        if (tmp_out.empty()) {
          selectLocations({});
          last_location_candidate = 0;
        }
        else {
          int location_idx = tmp_out.at(last_location_candidate % tmp_out.size());
          if (this->selected_locations.contains(location_idx)) {
            last_location_candidate++;
          }
          selectLocations({ tmp_out.at(last_location_candidate % tmp_out.size()) });
          last_location_candidate++;
        }
      }
      else {
        // TODO create location from unused here
      }

      location_drag = std::nullopt;
      view->setCursor(Qt::ArrowCursor);
      return true;
    }
    break;
  case QEvent::MouseMove:
    if (e->buttons() & Qt::LeftButton) {
      if (!location_drag) {
        location_drag = view->extendToRect(e->pos());
      }
      location_drag->setBottomRight(e->pos());
      view->setCursor(Qt::CrossCursor);
      return true;
    }
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
void LocationLayer::paintLocation(MapView* view, QPainter& painter, int locationId) {
  if (locationId < 0 || locationId >= map->num_locations())
    return;

  auto location = map->get_location(locationId);
  QRect locationRect = view->mapToViewRect(rect{ location });
  QRect viewRect = { { 0, 0 }, view->getViewSize() };
  if (!viewRect.intersects(locationRect)) return;

  paintLocationRect(painter, locationRect, this->selected_locations.contains(locationId));
  paintLocationName(painter, locationRect.topLeft(), map->get_location_name(locationId));
}

void LocationLayer::paintOverlay(MapView* view, QWidget* obj, QPainter& painter)
{
  QFont font = QFont();
  font.setPixelSize(16 * view->getViewScale());
  painter.setFont(font);

  // Get locations from largest to smallest to paint them in that order
  std::vector<LocMap> locations{ rtree.begin(), rtree.end() };
  std::ranges::sort(locations, [](auto& a, auto& b) { return boost::geometry::area(a.first) > boost::geometry::area(b.first); });

  for (auto [_, idx] : locations) {
    if (!this->selected_locations.contains(idx)) {
      paintLocation(view, painter, idx);
    }
  }

  // Draw selected location on top of unselected ones
  for (int idx : this->selected_locations) {
    paintLocation(view, painter, idx);
  }

  if (location_drag && rect{ *location_drag }.distance() >= 4) {
    painter.setPen(QPen(Qt::red, 3));
    painter.drawRect(*location_drag);
  }
}
void LocationLayer::paintGame(MapView* map, uint8_t* data, size_t data_pitch, bwgame::rect screen_rect)
{
}
void LocationLayer::reset()
{
  this->selected_locations.clear();
  this->rtree.clear();
  this->location_data.clear();
}
void LocationLayer::logicUpdate()
{
}
void LocationLayer::layerChanged(bool isEntering)
{
  reset();
  if (isEntering) {
    for (int i = 0; i < map->num_locations(); i++) {
      auto loc_data = getLocationData(i);
      if (loc_data) {
        this->location_data[i] = *loc_data;
        this->rtree.insert(*loc_data);
      }
    }
  }
}

void LocationLayer::locationUpdated(int id) {
  if (this->location_data.contains(id)) {
    this->rtree.remove(this->location_data.at(id));
  }

  auto loc_data = getLocationData(id);
  if (loc_data) {
    this->location_data[id] = *loc_data;
    this->rtree.insert(*loc_data);
  }
}

std::optional<LocMap> LocationLayer::getLocationData(int id) const {
  if(id < 0 || id >= map->num_locations())
    return std::nullopt;
  return LocMap(BoostRect(rect{ map->get_location(id) }), id);
}

void LocationLayer::selectLocations(const std::vector<int>& locations) {
  this->selected_locations.clear();
  this->selected_locations.insert(std::begin(locations), std::end(locations));
}
