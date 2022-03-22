#pragma once

#include <QString>
#include <QStringList>
#include <QList>
#include <QIcon>

#include <vector>
#include <optional>
#include <map>
#include <random>

namespace ChkForge {

  class Tileset {
  public:
    class TileGroup {
    public:
      TileGroup(int tilesetId, int groupId, const QString& name);

      int getGroupId() const;
      const QString& getName() const;
      const QIcon& getIcon() const;

    private:
      int tilesetId = -1;
      int groupId = -1;
      QString name;

      static std::map<int, std::map<int, QIcon>> icon_cache;
    };

    Tileset(int id, const QString& name, const QList<TileGroup>& brushes, int defaultBrushIndex);

    int getTilesetId() const;
    int getDefaultBrushIndex() const;
    const QString& getName() const;
    const QList<TileGroup>& getBrushes() const;

    //--------------

    int randomTile(int tileGroup, int clutter);

    //--------------

    static std::vector<Tileset*> getAllTilesets();
    static Tileset* fromId(int id);

    static Tileset Badlands;
    static Tileset Space;
    static Tileset Installation;
    static Tileset Ashworld;
    static Tileset Jungle;
    static Tileset Desert;
    static Tileset Ice;
    static Tileset Twilight;

  private:
    int tilesetId = -1;
    QString name;
    QList<TileGroup> brushes;
    int defaultBrushIndex;

    std::random_device rnd_d;
    std::mt19937 random{ rnd_d() };
  };


}
