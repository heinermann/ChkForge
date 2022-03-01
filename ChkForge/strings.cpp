#include "strings.h"

#include <array>
#include <QString>
#include <QObject>
#include <cstdlib>

#include <MappingCoreLib/Sc.h>
#include <MappingCoreLib/Chk.h>

namespace ChkForge {

  const QString emptyStr = "";

  const std::array<const QString, 8> raceNames = {
    //: gluAll:lobbyScreen_zerg
    QObject::tr("Zerg"),
    //: gluAll:lobbyScreen_terran
    QObject::tr("Terran"),
    //: gluAll:lobbyScreen_protoss
    QObject::tr("Protoss"),
    QObject::tr("Other"),
    //: EditLocal:29:458
    QObject::tr("Unused"),
    //: EditLocal:34:536
    QObject::tr("User Selectable"),
    //: gluAll:lobbyScreen_random
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
    //: EditLocal:24:377
    QObject::tr("Rescuable"),
    QObject::tr("Dummy"),
    //: gluAll:lobbyScreen_computer
    QObject::tr("Computer"),
    //: EditLocal:8:123
    QObject::tr("Human"),
    //: EditLocal:8:125
    QObject::tr("Neutral"),
    //: gluAll:lobbyScreen_closed
    QObject::tr("Closed"),
    //: network:strOBSERVER
    QObject::tr("Observer"),
    QObject::tr("Computer Left"),
    QObject::tr("Player Left")
  };

  const QString getPlayerOwnerName(unsigned owner) {
    if (owner >= ownerNames.size()) return QString::number(owner);
    return ownerNames[owner];
  }

  const std::array<const QString, Sc::Player::Total> genericPlayerNames = {
    //: EditLocal:8:126
    QObject::tr("Player 1"),
    //: EditLocal:8:127
    QObject::tr("Player 2"),
    //: EditLocal:9:128
    QObject::tr("Player 3"),
    //: EditLocal:9:129
    QObject::tr("Player 4"),
    //: EditLocal:9:130
    QObject::tr("Player 5"),
    //: EditLocal:9:131
    QObject::tr("Player 6"),
    //: EditLocal:9:132
    QObject::tr("Player 7"),
    //: EditLocal:9:133
    QObject::tr("Player 8"),
    //: EditLocal:9:134
    QObject::tr("Independent 1"),
    //: EditLocal:9:135
    QObject::tr("Independent 2"),
    //: EditLocal:9:136
    QObject::tr("Independent 3"),
    //: EditLocal:9:137
    QObject::tr("Neutral")
  };

  const QString getGenericPlayerName(unsigned id) {
    if (id >= genericPlayerNames.size()) return emptyStr;
    return genericPlayerNames[id];
  }

  const std::array<const std::array<const QString, Sc::Player::TotalSlots>, 3> scComputerNames = {
    std::array<const QString, Sc::Player::TotalSlots> {
    //: network:strNATION_ZERG_0
    QObject::tr("Tiamat Brood"),
      //: network:strNATION_ZERG_1
      QObject::tr("Surtur Brood"),
      //: network:strNATION_ZERG_2
      QObject::tr("Fenris Brood"),
      //: network:strNATION_ZERG_3
      QObject::tr("Jormungand Brood"),
      //: network:strNATION_ZERG_4
      QObject::tr("Garm Brood"),
      //: network:strNATION_ZERG_5
      QObject::tr("Grendel Brood"),
      //: network:strNATION_ZERG_6
      QObject::tr("Baelrog Brood"),
      //: network:strNATION_ZERG_7
      QObject::tr("Leviathan Brood")
    },
    {
      //: network:strNATION_TERRAN_0
      QObject::tr("Elite Guard"),
      //: network:strNATION_TERRAN_1
      QObject::tr("Mar Sara"),
      //: network:strNATION_TERRAN_2
      QObject::tr("Kel - Morian Combine"),
      //: network:strNATION_TERRAN_3
      QObject::tr("Antiga"),
      //: network:strNATION_TERRAN_4
      QObject::tr("Delta Squadron"),
      //: network:strNATION_TERRAN_5
      QObject::tr("Omega Squadron"),
      //: network:strNATION_TERRAN_6
      QObject::tr("Alpha Squadron"),
      //: network:strNATION_TERRAN_7
      QObject::tr("Epsilon Squadron")
    },
    {
      //: network:strNATION_PROTOSS_0
      QObject::tr("Ara Tribe"),
      //: network:strNATION_PROTOSS_1
      QObject::tr("Sargas Tribe"),
      //: network:strNATION_PROTOSS_2
      QObject::tr("Akilae Tribe"),
      //: network:strNATION_PROTOSS_3
      QObject::tr("Furinax Tribe"),
      //: network:strNATION_PROTOSS_4
      QObject::tr("Auriga Tribe"),
      //: network:strNATION_PROTOSS_5
      QObject::tr("Venatir Tribe"),
      //: network:strNATION_PROTOSS_6
      QObject::tr("Shelak Tribe"),
      //: network:strNATION_PROTOSS_7
      QObject::tr("Velari Tribe")
    }
  };

  // TODO: Figure out conditions for Terran's
  // p6: Atlas Wing
  // p7: Cronus Wing

  //: EditLocal:8:124
  const QString computerStr = QObject::tr("Computer");
  //: EditLocal:8:125
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
    //: gluAll:lobbyScreen_Red
    QObject::tr("Red"),
    //: gluAll:lobbyScreen_Blue
    QObject::tr("Blue"),
    //: gluAll:lobbyScreen_Teal
    QObject::tr("Teal"),
    //: gluAll:lobbyScreen_Purple
    QObject::tr("Purple"),
    //: gluAll:lobbyScreen_Orange
    QObject::tr("Orange"),
    //: gluAll:lobbyScreen_Brown
    QObject::tr("Brown"),
    //: gluAll:lobbyScreen_White
    QObject::tr("White"),
    //: gluAll:lobbyScreen_Yellow
    QObject::tr("Yellow"),
    //: gluAll:lobbyScreen_Green
    QObject::tr("Green"),
    //: gluAll:lobbyScreen_PaleYellow
    QObject::tr("Pale Yellow"),
    //: gluAll:lobbyScreen_Tan
    QObject::tr("Tan"),
    //: gluAll:lobbyScreen_DarkAqua
    QObject::tr("Dark Aqua"),
    //: gluAll:lobbyScreen_PaleGreen
    QObject::tr("Pale Green"),
    //: gluAll:lobbyScreen_BluishGrey
    QObject::tr("Bluish Grey"),
    //: gluAll:lobbyScreen_PaleYellow
    QObject::tr("Pale Yellow"),
    //: gluAll:lobbyScreen_Cyan
    QObject::tr("Cyan"),
    //: gluAll:lobbyScreen_Pink
    QObject::tr("Pink"),
    //: gluAll:lobbyScreen_Olive
    QObject::tr("Olive"),
    //: gluAll:lobbyScreen_Lime
    QObject::tr("Lime"),
    //: gluAll:lobbyScreen_Navy
    QObject::tr("Navy"),
    //: gluAll:lobbyScreen_Magenta
    QObject::tr("Magenta"),
    //: gluAll:lobbyScreen_Grey
    QObject::tr("Grey"),
    //: gluAll:lobbyScreen_Black
    QObject::tr("Black")
  };

  const QString getColorName(unsigned id) {
    if (id >= colorNames.size()) return QString::number(id);
    return colorNames[id];
  }

  const std::array<const QString, 5> defaultForceNames = {
    //: network:strFORCE1
    QObject::tr("Force 1"),
    //: network:strFORCE2
    QObject::tr("Force 2"),
    //: network:strFORCE3
    QObject::tr("Force 3"),
    //: network:strFORCE4
    QObject::tr("Force 4"),
    QObject::tr("None")
  };

  const QString getDefaultForceName(unsigned force) {
    if (force >= defaultForceNames.size()) return QString::number(force);
    return defaultForceNames[force];
  }

  const QString getSlotOwnerName(unsigned owner) {
    if (owner == Sc::Player::SlotType::Computer)
      //: gluAll:slotCOMPUTER
      return QObject::tr("Computer");
    else if (owner == Sc::Player::SlotType::Human)
      //: gluAll:slotOPEN
      return QObject::tr("Open") + "\u25BC";
    else
      return emptyStr;
  }

  const QString unknownStr = QObject::tr("Unknown");
  //: gluAll:lobbyScreen_random
  const QString randomStr = QObject::tr("Random");

  const std::array<const QString, 8> slotRaceNames = {
    //: gluAll:lobbyScreen_zerg
    QObject::tr("Zerg"),
    //: gluAll:lobbyScreen_terran
    QObject::tr("Terran"),
    //: gluAll:lobbyScreen_protoss
    QObject::tr("Protoss"),
    unknownStr,
    unknownStr,
    //: gluAll:lobbyScreen_selectRace
    QObject::tr("Select Race") + "\u25BC",
    //: gluAll:lobbyScreen_random
    QObject::tr("Random"),
    //: gluAll:lobbyScreen_selectRace
    QObject::tr("Select Race")
  };

  const QString getSlotRaceName(unsigned race) {
    if (race >= slotRaceNames.size()) unknownStr;
    return slotRaceNames[race];
  }

  const std::array<const QString, 3> slotScrColorNames = {
    //: gluAll:lobbyScreen_random
    QObject::tr("Random"),
    //: gluAll:lobbyScreen_random
    QObject::tr("Random") + "\u25BC",
    //: gluAll:lobbyScreen_MapSpecified
    QObject::tr("Map specified")
  };

  const QString getSlotColorName(unsigned scrColor, unsigned color) {
    if (scrColor < slotScrColorNames.size())
      return slotScrColorNames[scrColor];
    else if (color < colorNames.size())
      return colorNames[color];

    return randomStr;
  }

}
