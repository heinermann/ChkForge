#pragma once

#include <QString>

const QString getRaceName(unsigned race);
const QString getPlayerOwnerName(unsigned owner);
const QString getGenericPlayerName(unsigned id);
const QString getPlayerName(unsigned slot, unsigned race, unsigned controller);
const QString getColorName(unsigned id);
const QString getDefaultForceName(unsigned force);

const QString getSlotOwnerName(unsigned owner);
const QString getSlotRaceName(unsigned race);
const QString getSlotColorName(unsigned scrColor, unsigned color);
