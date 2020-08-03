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

#include "minimap.h"

#include "../openbw/openbw/ui/ui.h"
#include "../openbw/openbw/ui/native_window_drawing.h"
#include "../openbw/openbw/ui/common.h"
#include "../openbw/openbw/bwgame.h"
#include "../openbw/openbw/replay.h"

namespace bwgame {

  namespace ui {
    void log_str(a_string str) {
      static FILE* log_file = nullptr;

      fwrite(str.data(), str.size(), 1, stdout);
      fflush(stdout);
      if (!log_file) log_file = fopen("log.txt", "wb");
      if (log_file) {
        fwrite(str.data(), str.size(), 1, log_file);
        fflush(log_file);
      }
    }

    void fatal_error_str(a_string str) {
      log("fatal error: %s\n", str);
      //std::terminate();
    }
  }
}

struct main_t {
  bwgame::ui_functions ui;

  main_t(bwgame::game_player player) : ui(std::move(player)) {}

  void reset();
  void update();
};

void main_t::reset() {
  ui.reset();
}

void main_t::update() {
  ui.player.next_frame();
  /*
  auto next = [&]() {
	//ui.replay_functions::next_frame();
    ui.player.next_frame();
  };

  if (!ui.is_done() || ui.st.current_frame != ui.replay_frame) {
	if (ui.st.current_frame != ui.replay_frame) {
	  if (ui.st.current_frame < ui.replay_frame) {
		for (size_t i = 0; i != 32 && ui.st.current_frame != ui.replay_frame; ++i) {
		  for (size_t i2 = 0; i2 != 4 && ui.st.current_frame != ui.replay_frame; ++i2) {
			next();
		  }
		  if (clock.now() - now >= std::chrono::milliseconds(50)) break;
		}
	  }
	  last_tick = now;
	}
	else {
	  if (ui.is_paused) {
		last_tick = now;
	  }
	  else {
		auto tick_t = now - last_tick;
		if (tick_t >= tick_speed * 16) {
		  last_tick = now - tick_speed * 16;
		  tick_t = tick_speed * 16;
		}
		auto tick_n = tick_speed.count() == 0 ? 128 : tick_t / tick_speed;
		for (auto i = tick_n; i;) {
		  --i;
		  ++fps_counter;
		  last_tick += tick_speed;

		  if (!ui.is_done()) next();
		  else break;
		  if (i % 4 == 3 && clock.now() - now >= std::chrono::milliseconds(50)) break;
		}
		ui.replay_frame = ui.st.current_frame;
	  }
	}
  }*/
}

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
  delete bw;
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
  auto load_data_file = bwgame::data_loading::data_files_directory("C:/Program Files (x86)/StarCraft 1.16.1");
  bwgame::game_player player(load_data_file);

  bw = new main_t(std::move(player));

  bw->ui.load_all_image_data(load_data_file);

  bw->ui.load_data_file = [&](bwgame::a_vector<uint8_t>& data, bwgame::a_string filename) {
    load_data_file(data, std::move(filename));
  };

  bw->ui.init();
  bw->ui.load_map_file("C:/Program Files (x86)/StarCraft/Maps/(2)Bottleneck.scm");

  //bw->ui.load_replay_file("C:/Users/Adam Heinermann/Downloads/394928-Locu_kras-PvT.rep");

  bw->ui.screen_pos = { 0, 0 };
  bw->ui.set_image_data();
  bw->ui.global_volume = 0;

  timer->start(42);
  resizeSurface(ui->surface->size());
}

int MapView::map_tile_width()
{
  return bw->ui.game_st.map_tile_width;
}

int MapView::map_tile_height()
{
  return bw->ui.game_st.map_tile_height;
}

void MapView::updateLogic()
{
  bw->update();
  updateSurface();
}

void MapView::updateSurface()
{
  this->ui->surface->update();
}

void MapView::draw_minimap(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height)
{
  bw->ui.draw_minimap(data, data_pitch, surface_width, surface_height);
}

void MapView::move_minimap(int x, int y)
{
  bw->ui.move_minimap(x, y);
  updateScrollbarPositions();
  updateSurface();
}

QVector<QRgb> MapView::get_palette() {
  native_window_drawing::color* raw_colorTable = bw->ui.get_palette();
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
      this->is_drag_selecting = true;
      this->drag_select_from = this->drag_select_to = mouseEvent->pos();
    }
    else if (mouseEvent->button() == Qt::MiddleButton) {
      this->is_dragging_screen = true;
      this->last_drag_position = mouseEvent->pos();
      this->drag_screen_pos = getScreenPos() + mouseEvent->pos() / view_scale;
    }
    return true;
  case QEvent::MouseButtonDblClick:
    if (mouseEvent->button() == Qt::LeftButton) {
      bw->ui.select_units(true, shift_pressed, ctrl_pressed,
        mouseEvent->pos().x(), mouseEvent->pos().y(),
        mouseEvent->pos().x(), mouseEvent->pos().y());
      this->is_drag_selecting = false;
    }
    return true;
  case QEvent::MouseButtonRelease:
    if (mouseEvent->button() == Qt::LeftButton && this->is_drag_selecting) {
      bw->ui.select_units(false, shift_pressed, ctrl_pressed,
        this->drag_select_from.x(), this->drag_select_from.y(),
        mouseEvent->pos().x(), mouseEvent->pos().y());
      this->is_drag_selecting = false;
    }
    else if (mouseEvent->button() == Qt::MiddleButton) {
      this->is_dragging_screen = false;
    }
    return true;
  case QEvent::MouseMove:
    if (mouseEvent->buttons() & Qt::LeftButton) {
      if (!this->is_drag_selecting) {
        this->is_drag_selecting = true;
        this->drag_select_from = mouseEvent->pos();
      }
      this->drag_select_to = mouseEvent->pos();
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
  bw->ui.screen_pos.x = ui->hScroll->value();
  updateSurface();
}

void MapView::vScrollMoved()
{
  bw->ui.screen_pos.y = ui->vScroll->value();
  updateSurface();
}

void MapView::paint_surface(QWidget* obj, QPaintEvent* paintEvent)
{
  QPainter painter;
  painter.begin(obj);
  painter.fillRect(obj->rect(), QColorConstants::Black);
  
  bw->ui.draw_game(this->buffer.bits(), this->buffer.bytesPerLine(), this->buffer.width(), this->buffer.height());
  
  pix_buffer.convertFromImage(this->buffer);
  painter.drawPixmap(0, 0, pix_buffer);
  
  // Selection box
  if (is_drag_selecting) {
    painter.setPen(QColor(16, 252, 24));
    painter.drawLine(drag_select_from.x(), drag_select_from.y(), drag_select_from.x(), drag_select_to.y());
    painter.drawLine(drag_select_from.x(), drag_select_from.y(), drag_select_to.x(), drag_select_from.y());
    painter.drawLine(drag_select_to.x(), drag_select_from.y(), drag_select_to.x(), drag_select_to.y());
    painter.drawLine(drag_select_from.x(), drag_select_to.y(), drag_select_to.x(), drag_select_to.y());
  }

  painter.end();
}

QPoint MapView::getScreenPos()
{
  return QPoint{ bw->ui.screen_pos.x, bw->ui.screen_pos.y };
}

QSize MapView::getViewSize()
{
  return ui->surface->size();
}

void MapView::setScreenPos(const QPoint& pos)
{
  bw->ui.set_screen_pos(pos.x(), pos.y());
  updateScrollbarPositions();
}

void MapView::updateScrollbarPositions()
{
  this->ui->hScroll->setValue(bw->ui.screen_pos.x);
  this->ui->vScroll->setValue(bw->ui.screen_pos.y);
}

void MapView::resizeSurface(const QSize& newSize)
{
  this->buffer = QImage(newSize, QImage::Format::Format_Indexed8);
  this->buffer.setColorTable(this->get_palette());

  this->pix_buffer = QPixmap(newSize);

  bw->ui.resize(newSize.width(), newSize.height());
  this->ui->surface->update();

  int hPageStep = newSize.width();
  this->ui->hScroll->setPageStep(hPageStep);
  this->ui->hScroll->setMaximum(bw->ui.game_st.map_width - hPageStep);
  
  int vPageStep = newSize.height();
  this->ui->vScroll->setPageStep(vPageStep);
  this->ui->vScroll->setMaximum(bw->ui.game_st.map_height - vPageStep);
  updateScrollbarPositions();
  updateSurface();
}
