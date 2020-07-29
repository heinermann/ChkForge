#include "minimap.h"
#include "ui_minimap.h"

#include "mapview.h"
#include "../openbw/openbw/ui/native_window.h"
#include "../openbw/openbw/ui/native_window_drawing.h"

#include <QMessageBox>
#include <QEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QVector>
#include <QSize>

Minimap* Minimap::g_minimap = nullptr;

Minimap::Minimap(QWidget *parent) :
  DockWidgetWrapper<Ui::Minimap>("Minimap", parent)
{
  // Setup some states to allow direct rendering into the widget
  //ui->sdl_widget->setAttribute(Qt::WA_PaintOnScreen);
  //ui->sdl_widget->setAttribute(Qt::WA_OpaquePaintEvent);
  //ui->sdl_widget->setAttribute(Qt::WA_NoSystemBackground);

  // Set strong focus to enable keyboard events to be received
  //ui->sdl_widget->setFocusPolicy(Qt::StrongFocus);

  timer = std::make_unique<QTimer>(this);
  connect(timer.get(), SIGNAL(timeout()), this, SLOT(updateLogic()));

  ui->surface->installEventFilter(this);
}

Minimap::~Minimap()
{
  g_minimap = nullptr;
}

void Minimap::SDLInit()
{
  g_minimap = this;

  timer->start(33);
}

void Minimap::updateLogic()
{
  if (!activeMapView) return;

  activeMapView->draw_minimap(this->minimap_buffer.bits(), this->minimap_buffer.bytesPerLine(), this->minimap_buffer.width(), this->minimap_buffer.height());

  resetPalette();
  ui->surface->update();
}

void Minimap::setActiveMapView(MapView* view)
{
  this->activeMapView = view;
  resetMapBuffer();
  updateLogic();
}

void Minimap::removeMyMapView(MapView* view)
{
  if (this->activeMapView == view) setActiveMapView(nullptr);
}

void Minimap::resetMapBuffer()
{
  if (!this->activeMapView) return;

  this->minimap_buffer = QImage{
    this->activeMapView->map_width(), this->activeMapView->map_height(),
    QImage::Format::Format_Indexed8
  };
  resetPalette();
}

void Minimap::resetPalette()
{
  if (!this->activeMapView) return;

  native_window_drawing::color* raw_colorTable = reinterpret_cast<native_window_drawing::color*>(this->activeMapView->get_minimap_palette());
  QVector<QRgb> colors(256);
  for (int i = 0; i < 256; ++i) {
    native_window_drawing::color src_color = raw_colorTable[i];
    colors[i] = qRgb(src_color.r, src_color.g, src_color.b);
  }
  this->minimap_buffer.setColorTable(colors);
}

bool Minimap::eventFilter(QObject* obj, QEvent* e)
{
  if (obj != ui->surface || !activeMapView ) return false;
  
  double scale = std::min(1.0 * this->width() / minimap_buffer.width(), 1.0 * this->height() / minimap_buffer.height());

  switch (e->type()) {
  case QEvent::MouseButtonPress:
  {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(e);
    if (mouseEvent->button() == Qt::MouseButton::LeftButton) {
      activeMapView->move_minimap(mouseEvent->x() / scale, mouseEvent->y() / scale);
    }
    return true;
  }
  case QEvent::MouseMove:
  {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(e);
    if (mouseEvent->buttons() & Qt::MouseButton::LeftButton) {
      activeMapView->move_minimap(mouseEvent->x() / scale, mouseEvent->y() / scale);
    }
    return true;
  }
  case QEvent::Paint:
  {
    paint_minimap(static_cast<QWidget*>(obj), static_cast<QPaintEvent*>(e));
    return true;
  }
  }
  return false;
}

void Minimap::paint_minimap(QWidget* obj, QPaintEvent* paintEvent)
{
  QPainter painter;
  painter.begin(obj);
  painter.fillRect(obj->rect(), QColorConstants::Black);
  QImage toDraw = this->minimap_buffer.scaled(size(), Qt::KeepAspectRatio);
  painter.drawImage(0, 0, toDraw);
  painter.end();
}
