#include "PlaceUnitAction.h"
#include "MapContext.h"

using namespace ChkForge;

void PlaceUnitAction::apply() {
  this->index = map->rawPlaceUnit(x, y, unitType, owner);
}

void PlaceUnitAction::undo() {
  if (index == -1) return;
  map->rawRemoveUnit(index);
}

