#include "minimap.h"
#include "ui_minimap.h"

#include "mapview.h"
#include "../openbw/openbw/ui/native_window.h"
#include "../openbw/openbw/ui/native_window_drawing.h"

#include <QMessageBox>

Minimap* Minimap::g_minimap = nullptr;

Minimap::Minimap(QWidget *parent) :
  DockWidgetWrapper<Ui::Minimap>("Minimap", parent),
  sdl_minimap_wnd(new native_window::window)
{
  timer = std::make_unique<QTimer>(this);
  connect(timer.get(), SIGNAL(timeout()), this, SLOT(update()));
}

Minimap::~Minimap()
{
  SDL_DestroyRenderer(this->RendererRef);
  SDL_DestroyWindow(this->WindowRef);
}

void Minimap::SDLInit()
{
  g_minimap = this;

  WindowRef = SDL_CreateWindowFrom((void*)ui->sdl_widget->winId());
  RendererRef = SDL_CreateRenderer(WindowRef, -1, SDL_RENDERER_ACCELERATED);

  this->sdl_minimap_wnd->set(WindowRef);

  timer->start(33);
}

void Minimap::update()
{
  if (!activeMapView) return;

  native_window::event_t e;
  while (this->sdl_minimap_wnd->peek_message(e)) {
    activeMapView->process_minimap_event(e);
  }

  activeMapView->minimap_update();

  std::unique_ptr<native_window_drawing::surface> wnd_surface = native_window_drawing::get_window_surface(this->sdl_minimap_wnd.get());
  wnd_surface->set_blend_mode(native_window_drawing::blend_mode::none);
  wnd_surface->set_alpha(0);

  activeMapView->blit_minimap_to_surface(&*wnd_surface);
  sdl_minimap_wnd->update_surface();
}

void Minimap::setActiveMapView(MapView* view)
{
  this->activeMapView = view;
  update();
}