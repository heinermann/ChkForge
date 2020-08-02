#include "terrain.h"

#include <QObject>

#include <MappingCoreLib/Sc.h>

using namespace ChkForge;

Tileset::Tileset(int id, const QString& name, const QStringList& brushes, QList<int> brushTileValues, int defaultBrushIndex)
  : id(id)
  , name(name)
  , brushes(brushes)
  , brushTileValues(brushTileValues)
  , defaultBrushIndex(defaultBrushIndex)
{}

Tileset Tileset::Badlands = Tileset{
  Sc::Terrain::Tileset::Badlands,
  QObject::tr("Badlands"),
  QStringList{
    QObject::tr("Null"),
    QObject::tr("Creep"),
    QObject::tr("Dirt"),
    QObject::tr("Mud"),
    QObject::tr("High Dirt"),
    QObject::tr("Water"),
    QObject::tr("Grass"),
    QObject::tr("High Grass"),
    QObject::tr("Structure"),
    QObject::tr("Asphalt"),
    QObject::tr("Rocky Ground")
  },
  QList<int>{
    0, 1, 2, 20, 4, 6, 8, 10, 18, 16, 12
  },
  2
};

Tileset Tileset::Space = Tileset{
  Sc::Terrain::Tileset::SpacePlatform,
  QObject::tr("Space Platform"),
  QStringList{
    QObject::tr("Null"),
    QObject::tr("Creep"),
    QObject::tr("Space"),
    QObject::tr("Low Platform"),
    QObject::tr("Rusty Pit"),
    QObject::tr("Platform"),
    QObject::tr("Dark Platform"),
    QObject::tr("Plating"),
    QObject::tr("Solar Array"),
    QObject::tr("High Platform"),
    QObject::tr("High Plating"),
    QObject::tr("Elevated Catwalk")
  },
  QList<int>{
    0, 1, 2, 14, 18, 4, 16, 6, 12, 8, 10, 20
  },
  5
};

Tileset Tileset::Installation = Tileset{
  Sc::Terrain::Tileset::Installation,
  QObject::tr("Installation"),
  QStringList{
    QObject::tr("Null"),
    QObject::tr("Substructure"),
    QObject::tr("Substructure Plating"),
    QObject::tr("Floor"),
    QObject::tr("Roof"),
    QObject::tr("Plating"),
    QObject::tr("Bottomless Pit"),
    QObject::tr("Substructure Panels"),
    QObject::tr("Buildable Substructure")
  },
  QList<int>{
    0, 2, 4, 6, 8, 10, 12, 14, 16
  },
  3
};

Tileset Tileset::Ashworld = Tileset{
  Sc::Terrain::Tileset::Ashworld,
  QObject::tr("Ash World"),
  QStringList{
    QObject::tr("Null"),
    QObject::tr("Creep"),
    QObject::tr("Magma"),
    QObject::tr("Dirt"),
    QObject::tr("Lava"),
    QObject::tr("Shale"),
    QObject::tr("Broken Rock"),
    QObject::tr("High Dirt"),
    QObject::tr("High Lava"),
    QObject::tr("High Shale")
  },
  QList<int>{
    0, 1, 14, 2, 4, 10, 16, 6, 8, 12
  },
  3
};

Tileset Tileset::Jungle = Tileset{
  Sc::Terrain::Tileset::Jungle,
  QObject::tr("Jungle World"),
  QStringList{
    QObject::tr("Null"),
    QObject::tr("Creep"),
    QObject::tr("Water"),
    QObject::tr("Dirt"),
    QObject::tr("Mud"),
    QObject::tr("Jungle"),
    QObject::tr("Rocky Ground"),
    QObject::tr("Ruins"),
    QObject::tr("Raised Jungle"),
    QObject::tr("Temple"),
    QObject::tr("High Dirt"),
    QObject::tr("High Jungle"),
    QObject::tr("High Ruins"),
    QObject::tr("High Raised Jungle"),
    QObject::tr("High Temple")
  },
  QList<int>{
    0, 1, 6, 2, 26, 8, 10, 14, 12, 16, 4, 18, 20, 22, 24
  },
  5
};

Tileset Tileset::Desert = Tileset{
  Sc::Terrain::Tileset::Desert,
  QObject::tr("Desert"),
  QStringList{
    QObject::tr("Null"),
    QObject::tr("Creep"),
    QObject::tr("Tar"),
    QObject::tr("Dirt"),
    QObject::tr("Dried Mud"),
    QObject::tr("Sand Dunes"),
    QObject::tr("Rocky Ground"),
    QObject::tr("Crags"),
    QObject::tr("Sandy Sunken Pit"),
    QObject::tr("Compound"),
    QObject::tr("High Dirt"),
    QObject::tr("High Sand Dunes"),
    QObject::tr("High Crags"),
    QObject::tr("High Sandy Sunken Pit"),
    QObject::tr("High Compound")
  },
  QList<int>{
    0, 1, 6, 2, 26, 8, 10, 14, 12, 16, 4, 18, 20, 22, 24
  },
  5
};

Tileset Tileset::Ice = Tileset{
  Sc::Terrain::Tileset::Arctic,
  QObject::tr("Ice"),
  QStringList{
    QObject::tr("Null"),
    QObject::tr("Creep"),
    QObject::tr("Ice"),
    QObject::tr("Snow"),
    QObject::tr("Moguls"),
    QObject::tr("Dirt"),
    QObject::tr("Rocky Snow"),
    QObject::tr("Grass"),
    QObject::tr("Water"),
    QObject::tr("Outpost"),
    QObject::tr("High Snow"),
    QObject::tr("High Dirt"),
    QObject::tr("High Grass"),
    QObject::tr("High Water"),
    QObject::tr("High Outpost")
  },
  QList<int>{
    0, 1, 6, 2, 26, 8, 10, 14, 12, 16, 4, 18, 20, 22, 24
  },
  3
};

Tileset Tileset::Twilight = Tileset{
  Sc::Terrain::Tileset::Twilight,
  QObject::tr("Twilight"),
  QStringList{
    QObject::tr("Null"),
    QObject::tr("Creep"),
    QObject::tr("Water"),
    QObject::tr("Dirt"),
    QObject::tr("Mud"),
    QObject::tr("Crushed Rock"),
    QObject::tr("Crevices"),
    QObject::tr("Flagstones"),
    QObject::tr("Sunken Ground"),
    QObject::tr("Basilica"),
    QObject::tr("High Dirt"),
    QObject::tr("High Crushed Rock"),
    QObject::tr("High Flagstones"),
    QObject::tr("High Sunken Ground"),
    QObject::tr("High Basilica")
  },
  QList<int>{
    0, 1, 6, 2, 26, 8, 10, 14, 12, 16, 4, 18, 20, 22, 24
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

int Tileset::getId() const
{
  return this->id;
}

QString Tileset::getName() const
{
  return this->name;
}

QStringList Tileset::getBrushes() const
{
  return this->brushes;
}

QList<int> Tileset::getBrushValues() const
{
  return this->brushTileValues;
}

int Tileset::getDefaultBrushIndex() const
{
  return this->defaultBrushIndex;
}

