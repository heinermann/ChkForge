#pragma once

#include <QString>
#include <QStringList>
#include <QList>
#include <QIcon>

#include <vector>
#include <optional>

namespace ChkForge {

  class Tileset {
  public:
    class TileGroup {
    public:
      TileGroup(int tilesetId, int groupId, const QString& name);

      int getGroupId() const;
      const QString& getName() const;
      const QIcon& getIcon();

    private:
      int tilesetId = -1;
      int groupId = -1;
      QString name;
      std::optional<QIcon> icon = std::nullopt;
    };

    Tileset(int id, const QString& name, const QList<TileGroup>& brushes, int defaultBrushIndex);

    int getTilesetId() const;
    int getDefaultBrushIndex() const;
    const QString& getName() const;
    const QList<TileGroup>& getBrushes() const;

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
  };


}
