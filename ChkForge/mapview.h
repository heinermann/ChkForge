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

  void move_minimap(int x, int y);
  void draw_minimap(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height);
  uint32_t* get_minimap_palette();

  int map_width();
  int map_height();

private:
  std::unique_ptr<QImage> buffer;
  main_t* bw = nullptr;
  std::unique_ptr<QTimer> timer;

  SDL_Window* WindowRef = nullptr;
  SDL_Renderer* RendererRef = nullptr;

  virtual void changeEvent(QEvent* event) override;

private slots:
  void onCloseRequested();
  void updateLogic();

};

#endif // MAPVIEW_H
