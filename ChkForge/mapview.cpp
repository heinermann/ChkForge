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

QSize MapView::map_tile_size() const
{
  return QSize{ map_tile_width(), map_tile_height() };
}

int MapView::map_tile_width() const
{
  return map->openbw_ui.game_st.map_tile_width;
}

int MapView::map_tile_height() const
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

QSize MapView::minimap_size() const
{
  int tile_width = map_tile_width();
  int tile_height = map_tile_height();

  int minimap_width = std::max(tile_width, tile_height);
  int minimap_height = minimap_width;

  if (tile_width < tile_height) {
    minimap_width = minimap_width * minimap_width * tile_width / (minimap_height * tile_height);
  }
  else if (tile_height < tile_width) {
    minimap_height = minimap_height * minimap_height * tile_height / (minimap_width * tile_width);
  }

  return QSize{ minimap_width, minimap_height };
}

void MapView::draw_minimap(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height)
{
  map->openbw_ui.draw_minimap(data, data_pitch, surface_width, surface_height);
}

void MapView::move_minimap(const QPoint& pos)
{
  auto mm_size = minimap_size();

  auto clamped_pos = QPoint{ std::clamp(pos.x(), 0, mm_size.width() - 1), std::clamp(pos.y(), 0, mm_size.height() - 1) };

  int x = clamped_pos.x() * map_tile_size().width() / mm_size.width();
  int y = clamped_pos.y() * map_tile_size().height() / mm_size.height();

  QPoint{ screen_position.width(), screen_position.height() };

  setScreenPos(QPoint{ x, y } * 32 - QPoint{ screen_position.width(), screen_position.height() } / 2);

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
    else if (mouseEvent->button() == Qt::RightButton) {
      auto place_pos = screen_position.topLeft() + mouseEvent->pos();
      map->place_unit(Sc::Unit::Type::TerranMarine, 0, place_pos.x(), place_pos.y());
    }
    return true;
  case QEvent::MouseButtonDblClick:
    if (mouseEvent->button() == Qt::LeftButton) {
      select_units(true, shift_pressed, ctrl_pressed, QRect{ mouseEvent->pos(), mouseEvent->pos() });
      drag_select = std::nullopt;
    }
    return true;
  case QEvent::MouseButtonRelease:
    if (mouseEvent->button() == Qt::LeftButton && this->drag_select) {
      select_units(false, shift_pressed, ctrl_pressed, *drag_select);
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
    QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(e);
    this->resizeSurface(resizeEvent->size());
    return false;
  }
  case QEvent::Paint:
    paint_surface(static_cast<QWidget*>(obj), static_cast<QPaintEvent*>(e));
    return true;
  case QEvent::KeyPress:
    keyPressEvent(static_cast<QKeyEvent*>(e));
    return true;
  }

  return false;
}

void MapView::keyPressEvent(QKeyEvent* event)
{
  //if (event->modifiers() == Qt::KeyboardModifier::NoModifier) {
    switch (event->key()) {
    case Qt::Key::Key_Left:
      setScreenPos(screen_position.topLeft() - QPoint{4, 0});
      break;
    case Qt::Key::Key_Right:
      setScreenPos(screen_position.topLeft() + QPoint{ 4, 0 });
      break;
    case Qt::Key::Key_Up:
      setScreenPos(screen_position.topLeft() - QPoint{ 0, 4 });
      break;
    case Qt::Key::Key_Down:
      setScreenPos(screen_position.topLeft() + QPoint{ 0, 4 });
      break;
    }
  //}
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
  setScreenPos(QPoint{ ui->hScroll->value(), screen_position.y() });
  updateSurface();
}

void MapView::vScrollMoved()
{
  setScreenPos(QPoint{ screen_position.x(), ui->vScroll->value() });
  updateSurface();
}

void MapView::paint_surface(QWidget* obj, QPaintEvent* paintEvent)
{
  QPainter painter;
  painter.begin(obj);
  painter.fillRect(obj->rect(), QColorConstants::Black);
  
  map->openbw_ui.draw_game(this->buffer.bits(), this->buffer.bytesPerLine(),
    bwgame::rect{ {screen_position.left(), screen_position.top()}, {screen_position.right(), screen_position.bottom()} });
  
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
  return screen_position.topLeft();
}

QSize MapView::getViewSize()
{
  return ui->surface->size();
}

void MapView::setScreenPos(const QPoint& pos)
{
  screen_position.moveTopLeft(pos);
  if (screen_position.right() > map->openbw_ui.game_st.map_width) screen_position.moveRight(map->openbw_ui.game_st.map_width);
  if (screen_position.left() < 0) screen_position.moveLeft(0);
  if (screen_position.bottom() > map->openbw_ui.game_st.map_height) screen_position.moveBottom(map->openbw_ui.game_st.map_height);
  if (screen_position.top() < 0) screen_position.moveTop(0);
  updateScrollbarPositions();
}

void MapView::updateScrollbarPositions()
{
  this->ui->hScroll->setValue(screen_position.x());
  this->ui->vScroll->setValue(screen_position.y());
}

void MapView::resizeSurface(const QSize& newSize)
{
  this->buffer = QImage(newSize, QImage::Format::Format_Indexed8);
  this->buffer.setColorTable(this->get_palette());

  this->pix_buffer = QPixmap(newSize);

  screen_position.setSize(newSize);
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

void MapView::select_units(bool double_clicked, bool shift, bool ctrl, const QRect& selection)
{
  map->openbw_ui.select_units(double_clicked, shift, ctrl,
    selection.left(), selection.top(), selection.right(), selection.bottom(),
    bwgame::rect{ {screen_position.left(), screen_position.top()}, {screen_position.right(), screen_position.bottom()} });
}
