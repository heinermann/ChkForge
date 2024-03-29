#include "minimap.h"
#include "ui_minimap.h"

#include "mapview.h"

#include <QMessageBox>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QVector>
#include <QSize>
#include <QRect>
#include <QColor>

Minimap::Minimap(QWidget *parent) :
  //: EditLocal:10:148
  DockWidgetWrapper(tr("Minimap"), parent)
  , ui(std::make_unique<Ui::Minimap>())
{
  ui->setupUi(&frame);
  setupDockWidget();

  timer = std::make_unique<QTimer>(this);
  connect(timer.get(), &QTimer::timeout, this, &Minimap::updateLogic);

  ui->surface->installEventFilter(this);

  timer->start(33);
}

Minimap::~Minimap() {}

void Minimap::updateLogic()
{
  if (!activeMapView) return;

  activeMapView->draw_minimap(minimap_buffer.bits(), minimap_buffer.bytesPerLine(), minimap_buffer.width(), minimap_buffer.height());

  resetPalette();
  ui->surface->update();
}

void Minimap::setActiveMapView(MapView* view)
{
  this->activeMapView = view;
  resetMapBuffer();
  updateLogic();
}

void Minimap::onCloseMapView(MapView* map)
{
  if (this->activeMapView == map) setActiveMapView(nullptr);
}

void Minimap::resetMapBuffer()
{
  if (!this->activeMapView) return;

  this->minimap_buffer = QImage{
    this->activeMapView->map_tile_width(), this->activeMapView->map_tile_height(),
    QImage::Format::Format_Indexed8
  };
  resetPalette();
}

void Minimap::resetPalette()
{
  if (!this->activeMapView) return;
  this->minimap_buffer.setColorTable(this->activeMapView->get_palette());
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
      activeMapView->move_minimap(mouseEvent->pos() / scale);
    }
    return true;
  }
  case QEvent::MouseMove:
  {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(e);
    if (mouseEvent->buttons() & Qt::MouseButton::LeftButton) {
      activeMapView->move_minimap(mouseEvent->pos() / scale);
    }
    return true;
  }
  case QEvent::Paint:
    paint_surface(static_cast<QWidget*>(obj), static_cast<QPaintEvent*>(e));
    return true;
  }
  return false;
}

void Minimap::paint_surface(QWidget* obj, QPaintEvent* paintEvent)
{
  QPainter painter;
  painter.begin(obj);
  painter.fillRect(obj->rect(), QColorConstants::Black);
  
  QSize target_size = minimap_buffer.size();
  target_size.scale(obj->size(), Qt::AspectRatioMode::KeepAspectRatio);
  painter.drawImage(QRect(QPoint{ 0, 0 }, target_size), minimap_buffer);
  
  if (activeMapView) {
    double scale = 1.0 * target_size.width() / minimap_buffer.size().width();

    QRect screen = activeMapView->getScreenRect();
    QRect minimap_box = QRect{ screen.topLeft() / 32 * scale, screen.bottomRight() / 32 * scale };
    
    
    painter.setPen(QColor(255, 255, 255));
    painter.drawRect(minimap_box);
  }

  painter.end();
}
