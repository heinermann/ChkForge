#include "icons.h"

#include "../openbw/bwglobal_ui.h"

#include <vector>
#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QVector>
#include <QRgb>
#include <QResource>

namespace ChkForge::Icons {
  std::vector<QIcon> cmdicons32;

  const QIcon& getUnitIcon(int unit_type)
  {
    return cmdicons32.at(unit_type);
  }
  
  const std::vector<QIcon>& getAllIcons()
  {
    return cmdicons32;
  }

  void init()
  {
    QResource palette(":/pal/Icons.pal");
    auto* pal_data = palette.data();

    QVector<QRgb> colorTable(256);
    for (size_t i = 0; i != 256; ++i) {
      colorTable[i] = qRgb(pal_data[3 * i + 0], pal_data[3 * i + 1], pal_data[3 * i + 2]);
    }
    colorTable[0] = qRgba(0, 0, 0, 255); // background

    QImage img{ 32, 32, QImage::Format::Format_Indexed8 };
    img.setColorTable(colorTable);

    for (int i = 0; i < bwgame::global_ui_st.cmdicons.frames.size(); ++i) {
      img.fill(0);
      bwgame::global_ui_st.draw_icon(i, img.bits(), img.bytesPerLine(), img.width(), img.height());
      QPixmap pixmap = QPixmap::fromImage(img);
      cmdicons32.emplace_back(pixmap);
    }
  }

}
