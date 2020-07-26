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

class MapView : public DockWidgetWrapper<Ui::MapView>
{
  Q_OBJECT

public:
  explicit MapView(QWidget *parent = nullptr);
  virtual ~MapView();

  void SDLInit();

private:
  std::unique_ptr<QImage> buffer;
  main_t* bw;
  std::unique_ptr<QTimer> timer;

  SDL_Window* WindowRef;
  SDL_Renderer* RendererRef;

private slots:
  void onCloseRequested();
  void update();

};

#endif // MAPVIEW_H
