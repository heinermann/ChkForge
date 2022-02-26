#include "terrain.h"

#include <algorithm>

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QVector>

#include <MappingCoreLib/Sc.h>

#include "../openbw/bwglobal_ui.h"

using namespace ChkForge;

Tileset::TileGroup::TileGroup(int tilesetId, int groupId, const QString& name)
  : tilesetId(tilesetId)
  , groupId(groupId)
  , name(name)
{}

int Tileset::TileGroup::getGroupId() const
{
  return groupId;
}

const QString& Tileset::TileGroup::getName() const
{
  return name;
}

const QIcon& Tileset::TileGroup::getIcon()
{
  if (!icon) {
    const bwgame::tileset_image_data& tileset_img = bwgame::global_ui_st.all_tileset_img[tilesetId];

    QImage tile_img{ 32, 32, QImage::Format::Format_Indexed8 };
    tile_img.setColorCount(256);

    for (size_t i = 0; i < 256; ++i) {
      int r = tileset_img.wpe[i * 4 + 0];
      int g = tileset_img.wpe[i * 4 + 1];
      int b = tileset_img.wpe[i * 4 + 2];

      tile_img.setColor(i, qRgb(r, g, b));
    }

    uint16_t draw_tile_index = bwgame::global_st.get_cv5(tilesetId).at(groupId).mega_tile_index[0];
    bwgame::draw_tile(tileset_img, draw_tile_index, tile_img.bits(), tile_img.bytesPerLine(), 0, 0, tile_img.width(), tile_img.height());

    icon = QIcon(QPixmap::fromImage(tile_img));
  }
  return *icon;
}

Tileset::Tileset(int tilesetId, const QString& name, const QList<TileGroup>& brushes, int defaultBrushIndex)
  : tilesetId(tilesetId)
  , name(name)
  , brushes(brushes)
  , defaultBrushIndex(defaultBrushIndex)
{}

// TODO: Buildable structure
Tileset Tileset::Badlands = Tileset{
  Sc::Terrain::Tileset::Badlands,
  //: gluAll:gameMapERA
  QObject::tr("Badlands"),
  QList<TileGroup>{
    {0, 0, QObject::tr("Null")},
    {0, 1, QObject::tr("Creep")},
    //: EditLocal:1:15
    {0, 2, QObject::tr("Dirt")},
    //: EditLocal:33:526
    {0, 20, QObject::tr("Mud")},
    //: EditLocal:2:16
    {0, 4, QObject::tr("High Dirt")},
    //: EditLocal:2:24
    {0, 6, QObject::tr("Water")},
    //: EditLocal:2:17
    {0, 8, QObject::tr("Grass")},
    //: EditLocal:23:356
    {0, 10, QObject::tr("High Grass")},
    //: EditLocal:23:357
    {0, 18, QObject::tr("Structure")},
    //: EditLocal:11:168
    {0, 16, QObject::tr("Asphalt")},
    //: EditLocal:11:169
    {0, 12, QObject::tr("Rocky Ground")}
  },
  2
};

Tileset Tileset::Space = Tileset{
  Sc::Terrain::Tileset::SpacePlatform,
  //: gluAll:gameMapERA1 ("Space")
  QObject::tr("Space", "Tileset"),
  QList<TileGroup>{
    {1, 0, QObject::tr("Null")},
    {1, 1, QObject::tr("Creep")},
    //: EditLocal:2:22
    {1, 2, QObject::tr("Space", "Brush")},
    //: EditLocal:14:221
    {1, 14, QObject::tr("Low Platform")},
    //: EditLocal:14:222
    {1, 18, QObject::tr("Rusty Pit")},
    //: EditLocal:2:19
    {1, 4, QObject::tr("Platform")},
    //: EditLocal:23:363
    {1, 16, QObject::tr("Dark Platform")},
    //: EditLocal:2:18
    {1, 6, QObject::tr("Plating")},
    //: EditLocal:14:220
    {1, 12, QObject::tr("Solar Array")},
    //: EditLocal:2:21
    {1, 8, QObject::tr("High Platform")},
    //: EditLocal:2:20
    {1, 10, QObject::tr("High Plating")},
    //: EditLocal:16:241
    {1, 20, QObject::tr("Elevated Catwalk")}
  },
  5
};

Tileset Tileset::Installation = Tileset{
  Sc::Terrain::Tileset::Installation,
  //: gluAll:gameMapERA2
  QObject::tr("Installation"),
  QList<TileGroup>{
    {2, 0, QObject::tr("Null")},
    //: EditLocal:11:171
    {2, 2, QObject::tr("Substructure")},
    //: EditLocal:11:174
    {2, 4, QObject::tr("Substructure Plating")},
    //: EditLocal:11:172
    {2, 6, QObject::tr("Floor")},
    //: EditLocal:11:173
    {2, 8, QObject::tr("Roof")},
    //: EditLocal:11:175
    {2, 10, QObject::tr("Plating")},
    //: EditLocal:16:245
    {2, 12, QObject::tr("Bottomless Pit")},
    //: EditLocal:16:244
    {2, 14, QObject::tr("Substructure Panels")},
    {2, 16, QObject::tr("Buildable Substructure")}
  },
  3
};

Tileset Tileset::Ashworld = Tileset{
  Sc::Terrain::Tileset::Ashworld,
  //: gluAll:gameMapERA3 ("Ashworld")
  QObject::tr("Ashworld"),
  QList<TileGroup>{
    {3, 0, QObject::tr("Null")},
    {3, 1, QObject::tr("Creep")},
    //: EditLocal:15:231
    {3, 14, QObject::tr("Magma")},
    //: EditLocal:15:232
    {3, 2, QObject::tr("Dirt")},
    //: EditLocal:15:233
    {3, 4, QObject::tr("Lava")},
    //: EditLocal:15:234
    {3, 10, QObject::tr("Shale")},
    //: EditLocal:15:239
    {3, 16, QObject::tr("Broken Rock")},
    //: EditLocal:15:235
    {3, 6, QObject::tr("High Dirt")},
    //: EditLocal:15:236
    {3, 8, QObject::tr("High Lava")},
    //: EditLocal:15:237
    {3, 12, QObject::tr("High Shale")}
  },
  3
};

Tileset Tileset::Jungle = Tileset{
  Sc::Terrain::Tileset::Jungle,
  //: gluAll:gameMapERA4 ("Jungle")
  QObject::tr("Jungle", "Tileset"),
  QList<TileGroup>{
    {4, 0, QObject::tr("Null")},
    {4, 1, QObject::tr("Creep")},
    //: EditLocal:2:24
    {4, 6, QObject::tr("Water")},
    //: EditLocal:1:15
    {4, 2, QObject::tr("Dirt")},
    //: EditLocal:33:527
    {4, 26, QObject::tr("Mud")},
    //: EditLocal:11:167
    {4, 8, QObject::tr("Jungle", "Brush")},
    //: EditLocal:11:169
    {4, 10, QObject::tr("Rocky Ground")},
    //: EditLocal:11:166
    {4, 14, QObject::tr("Ruins")},
    //: EditLocal:12:190
    {4, 12, QObject::tr("Raised Jungle")},
    //: EditLocal:23:358
    {4, 16, QObject::tr("Temple")},
    //: EditLocal:2:16
    {4, 4, QObject::tr("High Dirt")},
    //: EditLocal:23:359
    {4, 18, QObject::tr("High Jungle")},
    //: EditLocal:23:360
    {4, 20, QObject::tr("High Ruins")},
    //: EditLocal:23:361
    {4, 22, QObject::tr("High Raised Jungle")},
    //: EditLocal:23:362
    {4, 24, QObject::tr("High Temple")}
  },
  5
};

Tileset Tileset::Desert = Tileset{
  Sc::Terrain::Tileset::Desert,
  //: gluAll:gameMapERA5
  QObject::tr("Desert", "Tileset"),
  QList<TileGroup>{
    {5, 0, QObject::tr("Null")},
    {5, 1, QObject::tr("Creep")},
    //: SEditENU:188:3001
    {5, 6, QObject::tr("Tar")},
    //: SEditENU:188:3002
    {5, 2, QObject::tr("Dirt")},
    //: SEditENU:188:3003
    {5, 26, QObject::tr("Dried Mud")},
    //: SEditENU:188:3004
    {5, 8, QObject::tr("Sand Dunes")},
    //: SEditENU:188:3005
    {5, 10, QObject::tr("Rocky Ground")},
    //: SEditENU:188:3006
    {5, 14, QObject::tr("Crags")},
    //: SEditENU:188:3007
    {5, 12, QObject::tr("Sandy Sunken Pit")},
    //: SEditENU:189:3008
    {5, 16, QObject::tr("Compound")},
    //: SEditENU:189:3009
    {5, 4, QObject::tr("High Dirt")},
    //: SEditENU:189:3010
    {5, 18, QObject::tr("High Sand Dunes")},
    //: SEditENU:189:3011
    {5, 20, QObject::tr("High Crags")},
    //: SEditENU:189:3012
    {5, 22, QObject::tr("High Sandy Sunken Pit")},
    //: SEditENU:189:3013
    {5, 24, QObject::tr("High Compound")}
  },
  5
};

Tileset Tileset::Ice = Tileset{
  Sc::Terrain::Tileset::Arctic,
  //: gluAll:gameMapERA6
  QObject::tr("Ice", "Tileset"),
  QList<TileGroup>{
    {6, 0, QObject::tr("Null")},
    {6, 1, QObject::tr("Creep")},
    //: SEditENU:189:3014
    {6, 6, QObject::tr("Ice", "Brush")},
    //: SEditENU:189:3015
    {6, 2, QObject::tr("Snow")},
    //: SEditENU:189:3016
    {6, 26, QObject::tr("Moguls")},
    //: SEditENU:189:3017
    {6, 8, QObject::tr("Dirt")},
    //: SEditENU:189:3018
    {6, 10, QObject::tr("Rocky Snow")},
    //: SEditENU:189:3019
    {6, 14, QObject::tr("Grass")},
    //: SEditENU:190:3028
    {6, 12, QObject::tr("Water")},
    //: SEditENU:189:3021
    {6, 16, QObject::tr("Outpost")},
    //: SEditENU:189:3022
    {6, 4, QObject::tr("High Snow")},
    //: SEditENU:189:3023
    {6, 18, QObject::tr("High Dirt")},
    //: SEditENU:190:3024
    {6, 20, QObject::tr("High Grass")},
    //: SEditENU:190:3025
    {6, 22, QObject::tr("High Water")},
    //: SEditENU:190:3026
    {6, 24, QObject::tr("High Outpost")}
  },
  3
};

Tileset Tileset::Twilight = Tileset{
  Sc::Terrain::Tileset::Twilight,
  //: gluAll:gameMapERA7
  QObject::tr("Twilight"),
  QList<TileGroup>{
    {7, 0, QObject::tr("Null")},
    {7, 1, QObject::tr("Creep")},
    //: SEditENU:190:3028
    {7, 6, QObject::tr("Water")},
    //: SEditENU:190:3029
    {7, 2, QObject::tr("Dirt")},
    //: SEditENU:190:3030
    {7, 26, QObject::tr("Mud")},
    //: SEditENU:190:3031
    {7, 8, QObject::tr("Crushed Rock")},
    //: SEditENU:190:3032
    {7, 10, QObject::tr("Crevices")},
    //: SEditENU:190:3033
    {7, 14, QObject::tr("Flagstones")},
    //: SEditENU:190:3034
    {7, 12, QObject::tr("Sunken Ground")},
    //: SEditENU:190:3035
    {7, 16, QObject::tr("Basilica")},
    //: SEditENU:191:3040
    {7, 4, QObject::tr("High Dirt")},
    //: SEditENU:190:3036
    {7, 18, QObject::tr("High Crushed Rock")},
    //: SEditENU:190:3037
    {7, 20, QObject::tr("High Flagstones")},
    //: SEditENU:190:3038
    {7, 22, QObject::tr("High Sunken Ground")},
    //: SEditENU:190:3039
    {7, 24, QObject::tr("High Basilica")}
  },
  3
};

namespace {
  static std::vector<Tileset*> allTilesets = {
    &Tileset::Badlands,
    &Tileset::Space,
    &Tileset::Installation,
    &Tileset::Ashworld,
    &Tileset::Jungle,
    &Tileset::Desert,
    &Tileset::Ice,
    &Tileset::Twilight
  };
}

std::vector<Tileset*> Tileset::getAllTilesets()
{
  return allTilesets;
}

Tileset* Tileset::fromId(int id)
{
  return allTilesets.at(id);
}

int Tileset::getTilesetId() const
{
  return this->tilesetId;
}

const QString& Tileset::getName() const
{
  return this->name;
}

const QList<Tileset::TileGroup>& Tileset::getBrushes() const
{
  return this->brushes;
}

int Tileset::getDefaultBrushIndex() const
{
  return this->defaultBrushIndex;
}

int Tileset::randomTile(int tileGroup, int clutter)
{
  // Source: Starforge Ultimate
  int numUncluttered = 0;
  int numCluttered = 0;

  // Creep
  if (tileGroup == 1) {
    int r = random() % 100;
    if (r < clutter) {
      return random() % 7 + tileGroup * 16 + 6;
    }
    else {
      return random() % 6 + tileGroup * 16;
    }
  }

  auto& megatile_index = bwgame::global_st.get_cv5(tilesetId)[tileGroup].mega_tile_index;

  for (numUncluttered = 0; numUncluttered < 16; numUncluttered++) {
    if (megatile_index[numUncluttered] == 0) break;
  }

  for (numCluttered = numUncluttered + 1; numCluttered < 16; numCluttered++) {
    if (megatile_index[numCluttered] == 0) break;
  }
  numCluttered -= numUncluttered + 1;

  if (numCluttered == 0 && numUncluttered == 0) {
    return random() % 16 + tileGroup * 16;
  }

  int r = random() % 100;
  if (r < clutter && numCluttered > 0) {
    return random() % numCluttered + tileGroup * 16 + numUncluttered + 1;
  }
  else {
    return random() % numUncluttered + tileGroup * 16;
  }
}
