#ifndef MINIMAP_H
#define MINIMAP_H

#include "DockWidgetWrapper.h"
#include "ui_minimap.h"

#include <QTimer>

#include <SDL.h>

class MapView;

namespace native_window {
  struct window;
};

class Minimap : public DockWidgetWrapper<Ui::Minimap>
{
  Q_OBJECT

public:
  explicit Minimap(QWidget *parent = nullptr);
  virtual ~Minimap();

  void SDLInit();

  void setActiveMapView(MapView* view);

  static Minimap* g_minimap;
private:
  std::unique_ptr<QTimer> timer;

  SDL_Window* WindowRef = nullptr;
  SDL_Renderer* RendererRef = nullptr;

  MapView* activeMapView = nullptr;

  std::unique_ptr<native_window::window> sdl_minimap_wnd;

private slots:
  void update();
};

#endif // MINIMAP_H
