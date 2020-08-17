#include "layers.h"

#include <QMouseEvent>
#include <QPainter>
#include <QColor>
#include <QWidget>

#include "mapview.h"
#include "MapContext.h"
#include "../openbw/openbw/bwgame.h"

using namespace ChkForge;


int Layer::getLayerId() {
  return layer_id;
}

void ThingyPlacer::clearDragSelect(MapView* map)
{
  drag_select = std::nullopt;
  map->setCursor(Qt::ArrowCursor);
}


bool ThingyPlacer::thingyMouseEvent(MapView* map, QMouseEvent* e)
{
  this->current_view = map;

  bool double_clicked = e->flags() & Qt::MouseEventCreatedDoubleClick;
  bool shift_pressed = e->modifiers() & Qt::ShiftModifier;
  bool ctrl_pressed = e->modifiers() & Qt::ControlModifier;

  if (isPlacingThingy()) {
    clearDragSelect(map);
  }
  else {
    switch (e->type())
    {
    case QEvent::MouseButtonPress:
      if (e->button() == Qt::LeftButton && !double_clicked) {
        drag_select = map->extendToRect(e->pos());
        return true;
      }
      break;
    case QEvent::MouseButtonDblClick:
      if (e->button() == Qt::LeftButton) {
        map->select_unit_at(true, shift_pressed, ctrl_pressed, e->pos());
        clearDragSelect(map);
        return true;
      }
      break;
    case QEvent::MouseButtonRelease:
      if (e->button() == Qt::LeftButton && this->drag_select) {
        if (std::abs(drag_select->width()) < 2 && std::abs(drag_select->height()) < 2) {
          map->select_unit_at(false, shift_pressed, ctrl_pressed, e->pos());
        }
        else {
          map->select_units(false, shift_pressed, ctrl_pressed, *drag_select);
        }
        clearDragSelect(map);
        return true;
      }
      break;
    case QEvent::MouseMove:
      if (e->buttons() & Qt::LeftButton) {
        if (!drag_select) {
          drag_select = map->extendToRect(e->pos());
        }
        drag_select->setBottomRight(e->pos());
        map->setCursor(Qt::CrossCursor);
        return true;
      }
    }
  }
  return false;
}

void ThingyPlacer::paintThingySelectOverlay(QWidget* obj, QPainter& painter)
{
  // Selection box
  if (drag_select) {
    painter.setPen(QColor(16, 252, 24));
    painter.drawRect(*drag_select);
  }
}

