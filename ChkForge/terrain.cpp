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
  QObject::tr("Badlands"),
  QList<TileGroup>{
    {0, 0, QObject::tr("Null")},
    {0, 1, QObject::tr("Creep")},
    {0, 2, QObject::tr("Dirt")},
    {0, 20, QObject::tr("Mud")},
    {0, 4, QObject::tr("High Dirt")},
    {0, 6, QObject::tr("Water")},
    {0, 8, QObject::tr("Grass")},
    {0, 10, QObject::tr("High Grass")},
    {0, 18, QObject::tr("Structure")},
    {0, 16, QObject::tr("Asphalt")},
    {0, 12, QObject::tr("Rocky Ground")}
  },
  2
};

Tileset Tileset::Space = Tileset{
  Sc::Terrain::Tileset::SpacePlatform,
  QObject::tr("Space Platform"),
  QList<TileGroup>{
    {1, 0, QObject::tr("Null")},
    {1, 1, QObject::tr("Creep")},
    {1, 2, QObject::tr("Space")},
    {1, 14, QObject::tr("Low Platform")},
    {1, 18, QObject::tr("Rusty Pit")},
    {1, 4, QObject::tr("Platform")},
    {1, 16, QObject::tr("Dark Platform")},
    {1, 6, QObject::tr("Plating")},
    {1, 12, QObject::tr("Solar Array")},
    {1, 8, QObject::tr("High Platform")},
    {1, 10, QObject::tr("High Plating")},
    {1, 20, QObject::tr("Elevated Catwalk")}
  },
  5
};

Tileset Tileset::Installation = Tileset{
  Sc::Terrain::Tileset::Installation,
  QObject::tr("Installation"),
  QList<TileGroup>{
    {2, 0, QObject::tr("Null")},
    {2, 2, QObject::tr("Substructure")},
    {2, 4, QObject::tr("Substructure Plating")},
    {2, 6, QObject::tr("Floor")},
    {2, 8, QObject::tr("Roof")},
    {2, 10, QObject::tr("Plating")},
    {2, 12, QObject::tr("Bottomless Pit")},
    {2, 14, QObject::tr("Substructure Panels")},
    {2, 16, QObject::tr("Buildable Substructure")}
  },
  3
};

Tileset Tileset::Ashworld = Tileset{
  Sc::Terrain::Tileset::Ashworld,
  QObject::tr("Ash World"),
  QList<TileGroup>{
    {3, 0, QObject::tr("Null")},
    {3, 1, QObject::tr("Creep")},
    {3, 14, QObject::tr("Magma")},
    {3, 2, QObject::tr("Dirt")},
    {3, 4, QObject::tr("Lava")},
    {3, 10, QObject::tr("Shale")},
    {3, 16, QObject::tr("Broken Rock")},
    {3, 6, QObject::tr("High Dirt")},
    {3, 8, QObject::tr("High Lava")},
    {3, 12, QObject::tr("High Shale")}
  },
  3
};

Tileset Tileset::Jungle = Tileset{
  Sc::Terrain::Tileset::Jungle,
  QObject::tr("Jungle World"),
  QList<TileGroup>{
    {4, 0, QObject::tr("Null")},
    {4, 1, QObject::tr("Creep")},
    {4, 6, QObject::tr("Water")},
    {4, 2, QObject::tr("Dirt")},
    {4, 26, QObject::tr("Mud")},
    {4, 8, QObject::tr("Jungle")},
    {4, 10, QObject::tr("Rocky Ground")},
    {4, 14, QObject::tr("Ruins")},
    {4, 12, QObject::tr("Raised Jungle")},
    {4, 16, QObject::tr("Temple")},
    {4, 4, QObject::tr("High Dirt")},
    {4, 18, QObject::tr("High Jungle")},
    {4, 20, QObject::tr("High Ruins")},
    {4, 22, QObject::tr("High Raised Jungle")},
    {4, 24, QObject::tr("High Temple")}
  },
  5
};

Tileset Tileset::Desert = Tileset{
  Sc::Terrain::Tileset::Desert,
  QObject::tr("Desert"),
  QList<TileGroup>{
    {5, 0, QObject::tr("Null")},
    {5, 1, QObject::tr("Creep")},
    {5, 6, QObject::tr("Tar")},
    {5, 2, QObject::tr("Dirt")},
    {5, 26, QObject::tr("Dried Mud")},
    {5, 8, QObject::tr("Sand Dunes")},
    {5, 10, QObject::tr("Rocky Ground")},
    {5, 14, QObject::tr("Crags")},
    {5, 12, QObject::tr("Sandy Sunken Pit")},
    {5, 16, QObject::tr("Compound")},
    {5, 4, QObject::tr("High Dirt")},
    {5, 18, QObject::tr("High Sand Dunes")},
    {5, 20, QObject::tr("High Crags")},
    {5, 22, QObject::tr("High Sandy Sunken Pit")},
    {5, 24, QObject::tr("High Compound")}
  },
  5
};

Tileset Tileset::Ice = Tileset{
  Sc::Terrain::Tileset::Arctic,
  QObject::tr("Ice"),
  QList<TileGroup>{
    {6, 0, QObject::tr("Null")},
    {6, 1, QObject::tr("Creep")},
    {6, 6, QObject::tr("Ice")},
    {6, 2, QObject::tr("Snow")},
    {6, 26, QObject::tr("Moguls")},
    {6, 8, QObject::tr("Dirt")},
    {6, 10, QObject::tr("Rocky Snow")},
    {6, 14, QObject::tr("Grass")},
    {6, 12, QObject::tr("Water")},
    {6, 16, QObject::tr("Outpost")},
    {6, 4, QObject::tr("High Snow")},
    {6, 18, QObject::tr("High Dirt")},
    {6, 20, QObject::tr("High Grass")},
    {6, 22, QObject::tr("High Water")},
    {6, 24, QObject::tr("High Outpost")}
  },
  3
};

Tileset Tileset::Twilight = Tileset{
  Sc::Terrain::Tileset::Twilight,
  QObject::tr("Twilight"),
  QList<TileGroup>{
    {7, 0, QObject::tr("Null")},
    {7, 1, QObject::tr("Creep")},
    {7, 6, QObject::tr("Water")},
    {7, 2, QObject::tr("Dirt")},
    {7, 26, QObject::tr("Mud")},
    {7, 8, QObject::tr("Crushed Rock")},
    {7, 10, QObject::tr("Crevices")},
    {7, 14, QObject::tr("Flagstones")},
    {7, 12, QObject::tr("Sunken Ground")},
    {7, 16, QObject::tr("Basilica")},
    {7, 4, QObject::tr("High Dirt")},
    {7, 18, QObject::tr("High Crushed Rock")},
    {7, 20, QObject::tr("High Flagstones")},
    {7, 22, QObject::tr("High Sunken Ground")},
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
