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
        std::vector<LocMap> results;
        rtree.query(bgi::intersects(BoostPt{ pt{ map_pos } }), std::back_inserter(results));
        
        auto tmp_select = results | std::views::values;
        std::vector<int> tmp_out{ tmp_select.begin(), tmp_select.end() };

        // FIXME some jank to avoid destroying intellisense
        if (tmp_out.empty()) {
          last_location_candidate = 0;
          selectLocations({});
        }
        else {
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
  Chk::LocationPtr location = map->get_location(locationId);
  if (!location) return;

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

  for (size_t i = 0; i < map->num_locations(); i++) {
    if (!this->selected_locations.contains(i)) {
      paintLocation(view, painter, i);
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
  auto location = map->get_location(id);
  if (!location) return std::nullopt;
  return LocMap(BoostRect(rect{ location }), id);
}

void LocationLayer::selectLocations(const std::vector<int>& locations) {
  this->selected_locations.clear();
  this->selected_locations.insert(std::begin(locations), std::end(locations));
}
