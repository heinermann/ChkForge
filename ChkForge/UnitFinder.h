#pragma once
// Modified from https://github.com/bwapi/bwapi/blob/bwapi5/bwapi/include/BWAPI/UnitFinder.h
// and https://github.com/bwapi/bwapi/blob/bwapi5/bwapi/Frontend/BWAPILIB/Source/UnitFinder.cpp

#include <set>
#include <unordered_set>
#include "../openbw/openbw/bwgame.h"

class UnitFinder
{
public:
  UnitFinder() {}

  void add(bwgame::unit_t* u) {
    bwgame::rect dimensions = { u->sprite->position - u->unit_type->dimensions.from, u->sprite->position + u->unit_type->dimensions.to };
    unitFinderX.insert(UnitCompare(dimensions.from.x, u));
    unitFinderX.insert(UnitCompare(dimensions.to.x, u));
    unitFinderY.insert(UnitCompare(dimensions.from.y, u));
    unitFinderY.insert(UnitCompare(dimensions.to.y, u));
  }

  std::unordered_set<bwgame::unit_t*> find(int left, int top, int right, int bottom) const
  {
    int searchRight = right, searchBottom = bottom;

    // Extension is to take into account units whose dimensions completely eclipse the location being searched.
    // The unit with the largest width/height can't eclipse the search 
    if (right - left <= maxUnitWidth)
      searchRight = left + maxUnitWidth;
    if (bottom - top <= maxUnitHeight)
      searchBottom = top + maxUnitHeight;

    std::unordered_set<bwgame::unit_t*> foundUnitsX;
    auto lowerXIter = unitFinderX.lower_bound(UnitCompare{ left, 0 });
    // We add 1 to right and bottom search because upper_bound is not inclusive - we want it to be
    auto upperXIter = unitFinderX.upper_bound(UnitCompare{ searchRight + 1, 0 });
    for (auto it = lowerXIter; it != upperXIter; ++it)
    {
      if (it->unit->sprite->position.x - it->unit->unit_type->dimensions.from.x <= right)
        foundUnitsX.insert(it->unit);
    }

    std::unordered_set<bwgame::unit_t*> result;

    auto lowerYIter = unitFinderY.lower_bound(UnitCompare{ top, 0 });
    auto upperYIter = unitFinderY.upper_bound(UnitCompare{ searchBottom + 1, 0 });
    for (auto it = lowerYIter; it != upperYIter; ++it)
    {
      // Ignore if the unit isn't within the X-bound
      if (foundUnitsX.count(it->unit) == 0) continue;

      if (it->unit->sprite->position.y - it->unit->unit_type->dimensions.from.y <= bottom)
        result.insert(it->unit);
    }
    return result;
  }

private:
  struct UnitCompare
  {
    UnitCompare(int value, bwgame::unit_t* unit) : value(value), unit(unit) {}

    int value;
    bwgame::unit_t* unit;

    bool operator <(UnitCompare const& other) const
    {
      return value < other.value;
    }
  };

  int maxUnitWidth = 256, maxUnitHeight = 256;
  std::multiset<UnitCompare> unitFinderX;
  std::multiset<UnitCompare> unitFinderY;
};
