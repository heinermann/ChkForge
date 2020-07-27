#ifndef MAPVIEW_H
#define MAPVIEW_H

#include "DockWidgetWrapper.h"
#include "ui_mapview.h"
#include <QImage>
#include <QTimer>
#include <QGraphicsScene>
#include <memory>

#include <SDL.h>

struct main_t;

namespace native_window
{
  struct event_t;
}

namespace native_window_drawing
{
  struct surface;
}

class MapView : public DockWidgetWrapper<Ui::MapView>
{
  Q_OBJECT

public:
  explicit MapView(QWidget *parent = nullptr);
  virtual ~MapView();

  void SDLInit();

  void minimap_update();
  void process_minimap_event(const native_window::event_t& e);
  void blit_minimap_to_surface(native_window_drawing::surface* dst);

private:
  std::unique_ptr<QImage> buffer;
  main_t* bw = nullptr;
  std::unique_ptr<QTimer> timer;

  SDL_Window* WindowRef = nullptr;
  SDL_Renderer* RendererRef = nullptr;

  virtual void changeEvent(QEvent* event) override;

private slots:
  void onCloseRequested();
  void update();

};

#endif // MAPVIEW_H
