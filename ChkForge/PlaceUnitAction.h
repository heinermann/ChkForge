#pragma once
#include "Action.h"
#include <MappingCoreLib/Sc.h>

namespace ChkForge {
  class PlaceUnitAction : public Action {
  public:
    PlaceUnitAction(MapContext* map, int x, int y, Sc::Unit::Type unitType, int owner)
      : Action(map)
      , owner(owner)
      , unitType(unitType)
      , x(x), y(y)
    {}
    virtual ~PlaceUnitAction() {}

    virtual void apply() override;
    virtual void undo() override;
  private:
    int owner;
    Sc::Unit::Type unitType;
    int x, y;
    int index = -1;
  };
}