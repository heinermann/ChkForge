/// TODO: Access Qt directly instead of going through SDL

#include "../openbw/openbw/ui/native_window.h"
#include "../openbw/openbw/ui/native_window_drawing.h"
#include "../openbw/openbw/ui/native_sound.h"
#include "../openbw/openbw/ui/common.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QOpenGLWidget>
#include <QVector>
#include <QPainter>
#include <QRgb>
#include <QRect>
#include <QPixmap>

using bwgame::ui::log;
using bwgame::ui::fatal_error;


namespace native_window_drawing {

  struct palette_impl : palette
  {
    QVector<QRgb> pal{ 256 };

    palette_impl() {}

    virtual ~palette_impl() override {}

    virtual void set_colors(color colors[256]) override {
      for (size_t i = 0; i < 256; ++i) {
        pal[i] = qRgb(colors[i].r, colors[i].g, colors[i].b);
        //col[i].a = colors[i].a;
      }
    }
  };

  struct qt_surface : surface
  {
    QImage* surf = nullptr;
    QPainter::CompositionMode composition_mode = QPainter::CompositionMode_Source;
    int alpha = 0; // TODO?

    void set(QImage* newSurf) {
      if (this->surf) delete this->surf;
      this->surf = newSurf;
      w = newSurf->width();
      h = newSurf->height();
      pitch = newSurf->bytesPerLine();
    }

    virtual ~qt_surface() override
    {
      delete surf;
    }

    virtual void set_palette(palette* pal) override {
      palette_impl* pal_impl = static_cast<palette_impl*>(pal);
      surf->setColorTable(pal_impl->pal);
    }
    virtual void* lock() override {
      return surf->bits();
    }
    virtual void unlock() override {
    }
    virtual void blit(surface* dst, int x, int y) override {
      qt_surface* dst_surface = static_cast<qt_surface*>(dst);
      QPainter painter{ dst_surface->surf };

      painter.setCompositionMode(this->composition_mode);
      painter.drawImage(x, y, *this->surf);
    }
    virtual void blit_scaled(surface* dst, int x, int y, int w, int h) override {
      qt_surface* dst_surface = static_cast<qt_surface*>(dst);
      QPainter painter{ dst_surface->surf };

      painter.setCompositionMode(this->composition_mode);
      QRect r{ x, y, w, h };
      painter.drawImage(r, *this->surf);
    }
    virtual void fill(int r, int g, int b, int a) override {
      this->surf->fill(QColor(r, g, b, a));
    }
    virtual void set_alpha(int a) override {
      this->alpha = a;
    }
    virtual void set_blend_mode(blend_mode blend) override {
      switch (blend)
      {
      case blend_mode::none:
        this->composition_mode = QPainter::CompositionMode_Source;
        break;
      case blend_mode::alpha:
        this->composition_mode = QPainter::CompositionMode_SourceOver;
        break;
      case blend_mode::add:
        this->composition_mode = QPainter::CompositionMode_Plus;
        break;
      case blend_mode::mod:
        this->composition_mode = QPainter::CompositionMode_Multiply;
        break;
      }
    }
  };
}

namespace native_window {
  struct window_impl
  {
    QGraphicsView* view;
    native_window_drawing::qt_surface* surface;

    window_impl()
      : surface(new native_window_drawing::qt_surface())
    {
      QImage* img = new QImage{ 640, 480, QImage::Format::Format_Indexed8 };
      surface->set(img);
      
      //view->scene()->
    }

    ~window_impl()
    {
      delete surface;
    }

    void setView(QGraphicsView* view) {
      this->view = view;
      update();
    }

    void update(bool force = true) {
      if (surface->surf->size() != view->size()) {
        surface->set(new QImage{ surface->surf->scaled(view->size()) });
        force = true;
      }

      if (force) {
        QPixmap px = QPixmap::fromImage(*surface->surf);
      }
    }
  };
}

namespace native_window_drawing {

  std::unique_ptr<surface> create_rgba_surface(int width, int height) {
    QImage* qimage = new QImage{ width, height, QImage::Format::Format_ARGB32 };
    auto r = std::make_unique<qt_surface>();
    r->set(qimage);
    return std::unique_ptr<surface>(r.release());
  }

  std::unique_ptr<surface> get_window_surface(native_window::window* wnd) {
    auto r = std::make_unique<qt_surface>();
    r->set(wnd->impl->surface->surf);
    return std::unique_ptr<surface>(r.release());
  }

  std::unique_ptr<surface> convert_to_8_bit_indexed(surface* s) {
    qt_surface* fromSurface = static_cast<qt_surface*>(s);
    QImage* newSurface = new QImage{ fromSurface->surf->convertToFormat(QImage::Format::Format_Indexed8) };

    auto r = std::make_unique<qt_surface>();
    r->set(newSurface);
    return std::unique_ptr<surface>(r.release());
  }

  palette* new_palette() {
    return new palette_impl();
  }
  void delete_palette(palette* pal) {
    delete pal;
  }

  std::unique_ptr<surface> load_image(const void* data, size_t size) {
    // N/A if use_new_images = false
    auto r = std::make_unique<qt_surface>();
    return std::unique_ptr<surface>(r.release());
  }
}


namespace native_window {
  window::window() {
    impl = std::make_unique<window_impl>();
  }

  window::~window() {
  }

  window::window(window&& n) {
    impl = std::move(n.impl);
  }

  void window::destroy() {
  }

  bool window::create(const char* title, int x, int y, int width, int height) {
    return false;
  }

  void window::get_cursor_pos(int* x, int* y) {
    QCursor cursor = impl->view->cursor();

    *x = cursor.pos().x();
    *y = cursor.pos().y();
  }

  bool window::peek_message(event_t& e) {
    // TODO: Implement
    //return impl->peek_message(e);
    return false;
  }

  bool window::show_cursor(bool show) {
    return true;
  }

  bool window::get_key_state(int scancode) {
    // TODO: Implement
    //return impl->get_key_state(scancode);
    return false;
  }

  bool window::get_mouse_button_state(int button) {
    // TODO: Implement
    //return impl->get_mouse_button_state(button);
    return false;
  }

  void window::update_surface() {
    impl->update();
  }

  window::operator bool() const {
    return impl->view != nullptr;
  }

  void window::set(void* handle) {
    impl->setView(static_cast<QGraphicsView*>(handle));
  }
}
