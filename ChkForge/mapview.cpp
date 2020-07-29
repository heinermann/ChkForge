#include "mapview.h"
#include "ui_mapview.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSize>
#include <QWindow>
#include <QPainter>
#include <QImage>
#include <QResizeEvent>

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
      std::terminate();
    }
  }
}

struct main_t {
  bwgame::ui_functions ui;

  main_t(bwgame::game_player player) : ui(std::move(player)) {}

  std::chrono::high_resolution_clock clock;
  std::chrono::high_resolution_clock::time_point last_tick;

  std::chrono::high_resolution_clock::time_point last_fps;
  int fps_counter = 0;

  void reset();
  void update();
};

void main_t::reset() {
  ui.reset();
}

void main_t::update() {
  auto now = clock.now();

  auto tick_speed = std::chrono::milliseconds((bwgame::fp8::integer(42) / ui.game_speed).integer_part());

  if (now - last_fps >= std::chrono::seconds(1)) {
	last_fps = now;
	fps_counter = 0;
  }

  auto next = [&]() {
	ui.replay_functions::next_frame();
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
  }
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
  bw->ui.create_window = false;

  bw->ui.load_all_image_data(load_data_file);

  bw->ui.load_data_file = [&](bwgame::a_vector<uint8_t>& data, bwgame::a_string filename) {
    load_data_file(data, std::move(filename));
  };

  bw->ui.init();
  bw->ui.load_replay_file("C:/Users/Adam Heinermann/Downloads/394928-Locu_kras-PvT.rep");

  bw->ui.screen_pos = { 0, 0 };
  bw->ui.set_image_data();
  bw->ui.global_volume = 0;

  timer->start(33);
  resizeSurface(ui->surface->size());
}

int MapView::map_width()
{
  return bw->ui.game_st.map_tile_width;
}

int MapView::map_height()
{
  return bw->ui.game_st.map_tile_height;
}

void MapView::updateLogic()
{
  bw->update();
  bw->ui.draw_game(this->buffer.bits(), this->buffer.bytesPerLine(), this->buffer.width(), this->buffer.height());
  this->ui->surface->update();
}

void MapView::draw_minimap(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height)
{
  bw->ui.draw_minimap(data, data_pitch, surface_width, surface_height);
}

void MapView::move_minimap(int x, int y)
{
  bw->ui.move_minimap(x, y);
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

bool MapView::eventFilter(QObject* obj, QEvent* e)
{
  if (obj != ui->surface) return false;

  switch (e->type())
  {
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

void MapView::paint_surface(QWidget* obj, QPaintEvent* paintEvent)
{
  QPainter painter;
  painter.begin(obj);
  painter.fillRect(obj->rect(), QColorConstants::Black);
  painter.drawImage(0, 0, this->buffer);
  painter.end();
}

void MapView::resizeSurface(const QSize& newSize)
{
  this->buffer = QImage(newSize, QImage::Format::Format_Indexed8);
  this->buffer.setColorTable(this->get_palette());

  bw->ui.resize(newSize.width(), newSize.height());
  this->ui->surface->update();
}
