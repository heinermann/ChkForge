#include "mapview.h"
#include "ui_mapview.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSize>
#include <QWindow>
#include <QPainter>
#include <QImage>
#include <QColor>
#include <QResizeEvent>
#include <QPoint>
#include <QRect>

#include "minimap.h"

#include "../openbw/openbw/ui/ui.h"
#include "../openbw/openbw/bwgame.h"

#include "MapContext.h"

MapView::MapView(QWidget *parent) :
  DockWidgetWrapper<Ui::MapView>("Map", parent)
{
  this->setFeature(DockWidgetDeleteOnClose, true);
  this->setFeature(DockWidgetFloatable, true);

  this->setFeature(CustomCloseHandling, true);
  connect(qobject_cast<ads::CDockWidget*>(this), SIGNAL(closeRequested()), SLOT(onCloseRequested()));

  timer = std::make_unique<QTimer>(this);
  connect(timer.get(), SIGNAL(timeout()), this, SLOT(updateLogic()));

  connect(ui->hScroll, SIGNAL(valueChanged(int)), this, SLOT(hScrollMoved()));
  connect(ui->vScroll, SIGNAL(valueChanged(int)), this, SLOT(vScrollMoved()));

  ui->surface->installEventFilter(this);
}

MapView::~MapView()
{
  if (Minimap::g_minimap) {
    Minimap::g_minimap->removeMyMapView(this);
  }
  delete map;
}

void MapView::onCloseRequested()
{
  auto widget = qobject_cast<ads::CDockWidget*>(sender());
  auto result = QMessageBox::question(nullptr, "Close?", "Are you sure?");
  if (result == QMessageBox::Yes)
    widget->closeDockWidget();
}

void MapView::init()
{
  map = new ChkForge::MapContext();
  map->load_map("C:/Program Files (x86)/StarCraft/Maps/(2)Bottleneck.scm");

  timer->start(42);
  resizeSurface(ui->surface->size());
}

int MapView::map_tile_width()
{
  return map->openbw_ui.game_st.map_tile_width;
}

int MapView::map_tile_height()
{
  return map->openbw_ui.game_st.map_tile_height;
}

void MapView::updateLogic()
{
  map->update();
  updateSurface();
}

void MapView::updateSurface()
{
  this->ui->surface->update();
}

void MapView::draw_minimap(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height)
{
  map->openbw_ui.draw_minimap(data, data_pitch, surface_width, surface_height);
}

void MapView::move_minimap(int x, int y)
{
  map->openbw_ui.move_minimap(x, y);
  updateScrollbarPositions();
  updateSurface();
}

QVector<QRgb> MapView::get_palette() {
  native_window_drawing::color* raw_colorTable = map->openbw_ui.get_palette();
  QVector<QRgb> colors(256);
  for (int i = 0; i < 256; ++i) {
    native_window_drawing::color src_color = raw_colorTable[i];
    colors[i] = qRgb(src_color.r, src_color.g, src_color.b);
  }
  return colors;
}

void MapView::changeEvent(QEvent* event)
{
  switch (event->type()) {
  case QEvent::FocusIn:
    Minimap::g_minimap->setActiveMapView(this);
    break;
  }
}

bool MapView::mouseEventFilter(QObject* obj, QEvent* e)
{
  QMouseEvent* mouseEvent = reinterpret_cast<QMouseEvent*>(e);
  bool double_clicked = mouseEvent->flags() & Qt::MouseEventCreatedDoubleClick;
  bool shift_pressed = mouseEvent->modifiers() & Qt::ShiftModifier;
  bool ctrl_pressed = mouseEvent->modifiers() & Qt::ControlModifier;

  switch (e->type())
  {
  case QEvent::MouseButtonPress:
    if (mouseEvent->button() == Qt::LeftButton && !double_clicked) {
      drag_select = QRect(mouseEvent->pos(), mouseEvent->pos());
    }
    else if (mouseEvent->button() == Qt::MiddleButton) {
      this->is_dragging_screen = true;
      this->last_drag_position = mouseEvent->pos();
      this->drag_screen_pos = getScreenPos() + mouseEvent->pos() / view_scale;
    }
    return true;
  case QEvent::MouseButtonDblClick:
    if (mouseEvent->button() == Qt::LeftButton) {
      map->openbw_ui.select_units(true, shift_pressed, ctrl_pressed,
        mouseEvent->pos().x(), mouseEvent->pos().y(),
        mouseEvent->pos().x(), mouseEvent->pos().y());
      drag_select = std::nullopt;
    }
    return true;
  case QEvent::MouseButtonRelease:
    if (mouseEvent->button() == Qt::LeftButton && this->drag_select) {
      map->openbw_ui.select_units(false, shift_pressed, ctrl_pressed,
        drag_select->left(), drag_select->top(),
        drag_select->right(), drag_select->bottom());
      drag_select = std::nullopt;
    }
    else if (mouseEvent->button() == Qt::MiddleButton) {
      this->is_dragging_screen = false;
    }
    return true;
  case QEvent::MouseMove:
    if (mouseEvent->buttons() & Qt::LeftButton) {
      if (!drag_select) {
        drag_select = QRect{ mouseEvent->pos(), mouseEvent->pos() };
      }
      drag_select->setBottomRight(mouseEvent->pos());
    }

    if (mouseEvent->buttons() & Qt::MiddleButton) {
      if (is_dragging_screen) {
        QPoint diff = mouseEvent->pos() - this->last_drag_position;
        this->last_drag_position = mouseEvent->pos();

        // TODO: Investigate how to wrap without this line (without bugs)
        if (diff.manhattanLength() > ui->surface->width() / 4) return true;

        this->setScreenPos(getScreenPos() - diff / this->view_scale);
        updateSurface();

        // Do mouse wrapping
        auto x_view_offset = QPoint{ ui->surface->width() - 8, 0 };
        auto y_view_offset = QPoint{ 0, ui->surface->height() - 8 };

        if (mouseEvent->pos().x() < 0) {
          this->last_drag_position += x_view_offset;
          QCursor::setPos(mouseEvent->globalPos() + x_view_offset);
        }
        else if (mouseEvent->pos().x() >= ui->surface->width()) {
          this->last_drag_position -= x_view_offset;
          QCursor::setPos(mouseEvent->globalPos() - x_view_offset);
        }

        if (mouseEvent->pos().y() < 0) {
          this->last_drag_position += y_view_offset;
          QCursor::setPos(mouseEvent->globalPos() + y_view_offset);
        }
        else if (mouseEvent->pos().y() >= ui->surface->height()) {
          this->last_drag_position -= y_view_offset;
          QCursor::setPos(mouseEvent->globalPos() - y_view_offset);
        }

      }
    }
    return true;
  }
}

bool MapView::surfaceEventFilter(QObject* obj, QEvent* e)
{
  switch (e->type())
  {
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonDblClick:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseMove:
    return mouseEventFilter(obj, e);
  case QEvent::Resize:
  {
    QResizeEvent* resizeEvent = reinterpret_cast<QResizeEvent*>(e);
    this->resizeSurface(resizeEvent->size());
    return false;
  }
  case QEvent::Paint:
    paint_surface(static_cast<QWidget*>(obj), static_cast<QPaintEvent*>(e));
    return true;
  }
  return false;
}

bool MapView::eventFilter(QObject* obj, QEvent* e)
{
  if (obj == ui->surface) {
    return surfaceEventFilter(obj, e);
  }
  return false;
}

void MapView::hScrollMoved()
{
  map->openbw_ui.screen_pos.x = ui->hScroll->value();
  updateSurface();
}

void MapView::vScrollMoved()
{
  map->openbw_ui.screen_pos.y = ui->vScroll->value();
  updateSurface();
}

void MapView::paint_surface(QWidget* obj, QPaintEvent* paintEvent)
{
  QPainter painter;
  painter.begin(obj);
  painter.fillRect(obj->rect(), QColorConstants::Black);
  
  map->openbw_ui.draw_game(this->buffer.bits(), this->buffer.bytesPerLine(), this->buffer.width(), this->buffer.height());
  
  pix_buffer.convertFromImage(this->buffer);
  painter.drawPixmap(0, 0, pix_buffer);
  
  // Selection box
  if (drag_select) {
    painter.setPen(QColor(16, 252, 24));
    painter.drawRect(*drag_select);
  }

  painter.end();
}

QPoint MapView::getScreenPos()
{
  return QPoint{ map->openbw_ui.screen_pos.x, map->openbw_ui.screen_pos.y };
}

QSize MapView::getViewSize()
{
  return ui->surface->size();
}

void MapView::setScreenPos(const QPoint& pos)
{
  map->openbw_ui.set_screen_pos(pos.x(), pos.y());
  updateScrollbarPositions();
}

void MapView::updateScrollbarPositions()
{
  this->ui->hScroll->setValue(map->openbw_ui.screen_pos.x);
  this->ui->vScroll->setValue(map->openbw_ui.screen_pos.y);
}

void MapView::resizeSurface(const QSize& newSize)
{
  this->buffer = QImage(newSize, QImage::Format::Format_Indexed8);
  this->buffer.setColorTable(this->get_palette());

  this->pix_buffer = QPixmap(newSize);

  map->openbw_ui.resize(newSize.width(), newSize.height());
  this->ui->surface->update();

  int hPageStep = newSize.width();
  this->ui->hScroll->setPageStep(hPageStep);
  this->ui->hScroll->setMaximum(map->openbw_ui.game_st.map_width - hPageStep);
  
  int vPageStep = newSize.height();
  this->ui->vScroll->setPageStep(vPageStep);
  this->ui->vScroll->setMaximum(map->openbw_ui.game_st.map_height - vPageStep);
  updateScrollbarPositions();
  updateSurface();
}
