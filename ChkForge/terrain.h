#pragma once

#include <QString>
#include <QStringList>
#include <QList>

#include <vector>

namespace ChkForge {
  class Tileset {
  public:
    Tileset(int id, const QString& name, const QStringList& brushes, QList<int> brushTileValues, int defaultBrushIndex);

    int getId() const;
    int getDefaultBrushIndex() const;
    QString getName() const;
    QStringList getBrushes() const;
    QList<int> getBrushValues() const;

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
    int id = -1;
    QString name;
    QStringList brushes;
    QList<int> brushTileValues;
    int defaultBrushIndex;
  };


}
