#include "PlaceUnitAction.h"
#include "MapContext.h"

using namespace ChkForge;

void PlaceUnitAction::apply() {
  auto unit = std::make_shared<Chk::Unit>();
  unit->classId = 0;
  unit->xc = x;
  unit->yc = y;
  unit->type = unitType;
  unit->relationFlags = 0;
  unit->validStateFlags = 0xFFFF;
  unit->validFieldFlags = 0xFFFF;
  unit->owner = owner;
  unit->hitpointPercent = 100;
  unit->shieldPercent = 100;
  unit->energyPercent = 100;
  unit->resourceAmount = 0;
  unit->hangerAmount = 0;
  unit->stateFlags = 0;
  unit->unused = 0;
  unit->relationClassId = 0;

  this->chkDraftIndex = map->chk->layers.addUnit(unit);
  this->openbwIndex = map->placeOpenBwUnit(unit);
}

void PlaceUnitAction::undo() {
  if (chkDraftIndex != -1) {
    map->chk->layers.deleteUnit(chkDraftIndex);
  }
  if (openbwIndex != -1) {
    map->removeOpenBwUnit(openbwIndex);
  }

}

