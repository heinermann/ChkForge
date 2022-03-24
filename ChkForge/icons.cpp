#include "icons.h"

#include "../openbw/bwglobal_ui.h"

#include <MappingCoreLib/Sc.h>

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

  // TODO: Implement MappingCoreLib loading of DAT files from CASC
  // DAT loading and asset mapping already available
  std::array<int, Sc::Upgrade::TotalTypes> upgrade_icons = {
    292, 293, 291, 297, 298, 303, 304, 288, 289, 290, 299, 300, 301, 305, 306, 310, 238, 287, 239, 248, 249, 256, 284, 285, 261, 262,
    296, 263, 264, 266, 268, 294, 295, 281, 307, 314, 315, 316, 317, 276, 318, 319, 320, 321, 322, 0, 0, 383, 0, 385, 0, 384, 379, 378, 380,
    0, 0, 0, 0, 0, 0
  };
  std::array<int, Sc::Tech::TotalTypes> tech_icons = {
    237, 240, 241, 243, 250, 245, 247, 242, 251, 252, 252, 259, 313, 40, 270, 265, 302, 267, 271, 275, 277, 280, 278, 68, 366, 369, 0,
    371, 63, 372, 373, 381, 376, 0, 365, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  std::array<int, Sc::Weapon::Total> weapon_icons = {
    323, 323, 324, 324, 325, 325, 364, 326, 327, 326, 327, 328, 328, 329, 330, 331, 332, 331, 332, 333, 334, 333, 334, 333, 334, 335, 335,
    336, 336, 337, 251, 311, 240, 241, 241, 338, 338, 338, 339, 339, 340, 340, 341, 342, 342, 343, 344, 344, 345, 345, 346, 346, 347, 348,
    349, 350, 271, 40, 267, 270, 265, 265, 351, 352, 353, 353, 354, 354, 355, 355, 356, 356, 357, 358, 359, 358, 359, 360, 360, 361, 362,
    362, 85, 278, 278, 353, 353, 363, 0, 0, 0, 0, 0, 0 ,0, 0, 326, 327, 335, 327, 332, 359, 241, 331, 344, 332, 332, 332, 240, 382, 323,
    353, 324, 324, 358, 359, 324, 323, 323, 323, 323, 323, 323, 323, 323, 323, 323, 323, 323, 323
  };

  const QIcon& getUpgradeIcon(int upgrade_type) {
    int icon_id = upgrade_icons.at(upgrade_type);
    return cmdicons32.at(icon_id);
  }

  const QIcon& getTechIcon(int tech_type) {
    int icon_id = tech_icons.at(tech_type);
    return cmdicons32.at(icon_id);
  }

  const QIcon& getWeaponIcon(int weapon_type) {
    int icon_id = weapon_icons.at(weapon_type);
    return cmdicons32.at(icon_id);
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
