#pragma once
#include <MappingCoreLib/Chk.h>
#include <MappingCoreLib/Sc.h>

#include <array>
#include <string>

struct ScenarioSettingsData
{
  Chk::SIDE side; // Races
  Chk::COLR colr; // Player colors
  Chk::FORC forc; // Forces
  Chk::OWNR ownr; // Slot owners
  Chk::IOWN iown; // Redundant slot owners

  Chk::UNIS unis; // Unit settings
  Chk::UNIx unix; // Expansion Unit Settings
  Chk::PUNI puni; // Unit availability
  Chk::UPGS upgs; // Upgrade costs
  Chk::UPGx upgx; // Expansion upgrade costs
  Chk::UPGR upgr; // Upgrade leveling
  Chk::PUPx pupx; // Expansion upgrade leveling
  Chk::TECS tecs; // Technology costs
  Chk::TECx tecx; // Expansion technology costs
  Chk::PTEC ptec; // Technology availability
  Chk::PTEx ptex; // Expansion technology availability

  std::array<bool, Chk::TotalForces> useCustomForceNames;
  std::array<std::string, Chk::TotalForces> forceNames;

  std::array<bool, Sc::Unit::TotalTypes> useDefaultUnitNames;
  std::array<std::string, Sc::Unit::TotalTypes> unitNames;
};

