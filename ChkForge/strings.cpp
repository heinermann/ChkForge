#include "strings.h"

#include <array>
#include <QString>
#include <QObject>
#include <cstdlib>

#include <MappingCoreLib/Sc.h>
#include <MappingCoreLib/Chk.h>

const QString emptyStr = "";

const std::array<const QString, 8> raceNames = {
  QObject::tr("Zerg"),
  QObject::tr("Terran"),
  QObject::tr("Protoss"),
  QObject::tr("Other"),
  QObject::tr("Unused"),
  QObject::tr("User Select"),
  QObject::tr("Random"),
  QObject::tr("None")
};

const QString getRaceName(unsigned race) {
  if (race >= raceNames.size()) return QString::number(race);
  return raceNames[race];
}

const std::array<const QString, 12> ownerNames = {
  QObject::tr("None"),
  QObject::tr("Occupied Computer"),
  QObject::tr("Occupied Human"),
  QObject::tr("Rescuable"),
  QObject::tr("Dummy"),
  QObject::tr("Computer"),
  QObject::tr("Human"),
  QObject::tr("Neutral"),
  QObject::tr("Closed"),
  QObject::tr("Observer"),
  QObject::tr("Computer Left"),
  QObject::tr("Player Left")
};

const QString getPlayerOwnerName(unsigned owner) {
  if (owner >= ownerNames.size()) return QString::number(owner);
  return ownerNames[owner];
}

const std::array<const QString, Sc::Player::Total> genericPlayerNames = {
  QObject::tr("Player 1"),
  QObject::tr("Player 2"),
  QObject::tr("Player 3"),
  QObject::tr("Player 4"),
  QObject::tr("Player 5"),
  QObject::tr("Player 6"),
  QObject::tr("Player 7"),
  QObject::tr("Player 8"),
  QObject::tr("Independent 1"),
  QObject::tr("Independent 2"),
  QObject::tr("Independent 3"),
  QObject::tr("Neutral")
};

const QString getGenericPlayerName(unsigned id) {
  if (id >= genericPlayerNames.size()) return emptyStr;
  return genericPlayerNames[id];
}

const std::array<const std::array<const QString, Sc::Player::TotalSlots>, 3> scComputerNames = {
  std::array<const QString, Sc::Player::TotalSlots> {
    QObject::tr("Tiamat Brood"),
    QObject::tr("Surtur Brood"),
    QObject::tr("Fenris Brood"),
    QObject::tr("Jormungand Brood"),
    QObject::tr("Garm Brood"),
    QObject::tr("Grendel Brood"),
    QObject::tr("Baelrog Brood"),
    QObject::tr("Leviathan Brood")
  },
  {
    QObject::tr("Elite Guard"),
    QObject::tr("Mar Sara"),
    QObject::tr("Kel - Morian Combine"),
    QObject::tr("Antiga"),
    QObject::tr("Delta Squadron"),
    QObject::tr("Omega Squadron"),
    QObject::tr("Alpha Squadron"),
    QObject::tr("Epsilon Squadron")
  },
  {
    QObject::tr("Ara Tribe"),
    QObject::tr("Sargas Tribe"),
    QObject::tr("Akilae Tribe"),
    QObject::tr("Furinax Tribe"),
    QObject::tr("Auriga Tribe"),
    QObject::tr("Venatir Tribe"),
    QObject::tr("Shelak Tribe"),
    QObject::tr("Velari Tribe")
  }
};

// TODO: Figure out conditions for Terran's
// p6: Atlas Wing
// p7: Cronus Wing

const QString computerStr = QObject::tr("Computer");
const QString neutralStr = QObject::tr("Neutral");

const QString getComputerPlayerName(unsigned slot, unsigned race) {
  if (slot >= 8) return computerStr;

  if (race == Chk::Race::Random || race == Chk::Race::UserSelectable) race = std::rand() % 3;
  else if (race >= 3) race = Chk::Race::Terran;

  return scComputerNames[race][slot];
}

bool isNamedComputerController(unsigned controller) {
  return controller == Sc::Player::SlotType::GameComputer ||
    controller == Sc::Player::SlotType::RescuePassive ||
    controller == Sc::Player::SlotType::Unused ||
    controller == Sc::Player::SlotType::Computer ||
    controller == Sc::Player::SlotType::Neutral;
}

const QString getPlayerName(unsigned slot, unsigned race, unsigned controller) {
  if (slot == Sc::Player::Id::Player12_Neutral) {
    return neutralStr;
  }
  else if (isNamedComputerController(controller) || slot >= 8) {
    return getComputerPlayerName(slot, race);
  }
  return emptyStr;
}

const std::array<const QString, 23> colorNames = {
  QObject::tr("Red"),
  QObject::tr("Blue"),
  QObject::tr("Teal"),
  QObject::tr("Purple"),
  QObject::tr("Orange"),
  QObject::tr("Brown"),
  QObject::tr("White"),
  QObject::tr("Yellow"),
  QObject::tr("Green"),
  QObject::tr("Gold"),
  QObject::tr("Tan"),
  QObject::tr("Azure"),
  QObject::tr("Pale Green"),
  QObject::tr("Bluish Grey"),
  QObject::tr("Pale Yellow"),
  QObject::tr("Cyan"),
  QObject::tr("Pink"),
  QObject::tr("Olive"),
  QObject::tr("Lime"),
  QObject::tr("Navy"),
  QObject::tr("Magenta"),
  QObject::tr("Grey"),
  QObject::tr("Black")
};

const QString getColorName(unsigned id) {
  if (id >= colorNames.size()) return QString::number(id);
  return colorNames[id];
}

const std::array<const QString, 5> defaultForceNames = {
  QObject::tr("Force 1"),
  QObject::tr("Force 2"),
  QObject::tr("Force 3"),
  QObject::tr("Force 4"),
  QObject::tr("None")
};

const QString getDefaultForceName(unsigned force) {
  if (force >= defaultForceNames.size()) return QString::number(force);
  return defaultForceNames[force];
}

const QString getSlotOwnerName(unsigned owner) {
  if (owner == Sc::Player::SlotType::Computer)
    return QObject::tr("Computer");
  else if (owner == Sc::Player::SlotType::Human)
    return QObject::tr("Open") + "\u25BC";
  else
    return emptyStr;
}

const QString unknownStr = QObject::tr("Unknown");
const QString randomStr = QObject::tr("Random");

const std::array<const QString, 8> slotRaceNames = {
  QObject::tr("Zerg"),
  QObject::tr("Terran"),
  QObject::tr("Protoss"),
  unknownStr,
  unknownStr,
  QObject::tr("Select Race") + "\u25BC",
  QObject::tr("Random"),
  QObject::tr("Select Race")
};

const QString getSlotRaceName(unsigned race) {
  if (race >= slotRaceNames.size()) unknownStr;
  return slotRaceNames[race];
}

const std::array<const QString, 3> slotScrColorNames = {
  QObject::tr("Random"),
  QObject::tr("Random") + "\u25BC",
  QObject::tr("Map specified")
};

const QString getSlotColorName(unsigned scrColor, unsigned color) {
  if (scrColor < slotScrColorNames.size())
    return slotScrColorNames[scrColor];
  else if (color < colorNames.size())
    return colorNames[color];

  return randomStr;
}

