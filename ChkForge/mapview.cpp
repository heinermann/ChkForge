#include "mapview.h"
#include "ui_mapview.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSize>
#include <QWindow>

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

struct saved_state {
  bwgame::state st;
  bwgame::action_state action_st;
};

struct main_t {
  bwgame::ui_functions ui;

  main_t(bwgame::game_player player) : ui(std::move(player)) {}

  std::chrono::high_resolution_clock clock;
  std::chrono::high_resolution_clock::time_point last_tick;

  std::chrono::high_resolution_clock::time_point last_fps;
  int fps_counter = 0;

  bwgame::a_map<int, std::unique_ptr<saved_state>> saved_states;

  void reset();
  void update();
};

void main_t::reset() {
  saved_states.clear();
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
	int save_interval = 10 * 1000 / 42;
	if (ui.st.current_frame == 0 || ui.st.current_frame % save_interval == 0) {
	  auto i = saved_states.find(ui.st.current_frame);
	  if (i == saved_states.end()) {
		auto v = std::make_unique<saved_state>();
		v->st = copy_state(ui.st);
		v->action_st = copy_state(ui.action_st, ui.st, v->st);

		bwgame::a_map<int, std::unique_ptr<saved_state>> new_saved_states;
		new_saved_states[ui.st.current_frame] = std::move(v);
		while (!saved_states.empty()) {
		  auto i = saved_states.begin();
		  auto v = std::move(*i);
		  saved_states.erase(i);
		  new_saved_states[v.first] = std::move(v.second);
		}
		std::swap(saved_states, new_saved_states);
	  }
	}
	ui.replay_functions::next_frame();
  };

  if (!ui.is_done() || ui.st.current_frame != ui.replay_frame) {
	if (ui.st.current_frame != ui.replay_frame) {
	  if (ui.st.current_frame != ui.replay_frame) {
		auto i = saved_states.lower_bound(ui.replay_frame);
		if (i != saved_states.begin()) --i;
		auto& v = i->second;
		if (ui.st.current_frame > ui.replay_frame || v->st.current_frame > ui.st.current_frame) {
		  ui.st = copy_state(v->st);
		  ui.action_st = copy_state(v->action_st, v->st, ui.st);
		}
	  }
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

  ui.update();
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
}

MapView::~MapView()
{
  Minimap::g_minimap->removeMyMapView(this);

  SDL_DestroyRenderer(this->RendererRef);
  SDL_DestroyWindow(this->WindowRef);
  delete bw;
}

void MapView::onCloseRequested()
{
  auto widget = qobject_cast<ads::CDockWidget*>(sender());
  auto result = QMessageBox::question(nullptr, "Close?", "Are you sure?");
  if (result == QMessageBox::Yes)
    widget->closeDockWidget();
}

void MapView::SDLInit()
{
  ui->sdl_view->setAttribute(Qt::WA_PaintOnScreen);
  ui->sdl_view->setAttribute(Qt::WA_OpaquePaintEvent);
  ui->sdl_view->setAttribute(Qt::WA_NoSystemBackground);

  // Set strong focus to enable keyboard events to be received
  ui->sdl_view->setFocusPolicy(Qt::StrongFocus);

  WindowRef = SDL_CreateWindowFrom((void*)ui->sdl_view->winId());
  RendererRef = SDL_CreateRenderer(WindowRef, -1, SDL_RENDERER_ACCELERATED);

  QSize gfxSize = ui->sdl_view->size();

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

  bw->ui.wnd.set(WindowRef);
  bw->ui.resize(gfxSize.width(), gfxSize.height());
  bw->ui.screen_pos = { (int)bw->ui.game_st.map_width / 2 - gfxSize.width() / 2, (int)bw->ui.game_st.map_height / 2 - gfxSize.height() / 2 };
  bw->ui.set_image_data();
  bw->ui.global_volume = 0;

  timer->start(33);
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
}

void MapView::draw_minimap(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height)
{
  bw->ui.draw_minimap(data, data_pitch, surface_width, surface_height);
}

void MapView::move_minimap(int x, int y)
{
  bw->ui.move_minimap(x, y);
}

uint32_t* MapView::get_minimap_palette() {
  return bw->ui.get_minimap_palette();
}

void MapView::changeEvent(QEvent* event)
{
  switch (event->type()) {
  case QEvent::FocusIn:
    Minimap::g_minimap->setActiveMapView(this);
    break;
  }
}
