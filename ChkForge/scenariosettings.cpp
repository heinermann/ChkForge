#include "scenariosettings.h"
#include "ui_scenariosettings.h"

ScenarioSettings::ScenarioSettings(QWidget* parent, int startTab) :
  QDialog(parent),
  ui(new Ui::ScenarioSettings)
{
  ui->setupUi(this);

  ui->tabs->setCurrentIndex(startTab);
}

ScenarioSettings::~ScenarioSettings()
{
  delete ui;
}

void ScenarioSettings::readFromMap(const MapFile& map) {
  settings.side = map.players.side->get();
  settings.colr = map.players.colr->get();
  settings.forc = map.players.forc->get();
  settings.ownr = map.players.ownr->get();
  settings.iown = map.players.iown->get();

  settings.unis = map.properties.unis->get();
  settings.unix = map.properties.unix->get();
  settings.puni = map.properties.puni->get();
  settings.upgs = map.properties.upgs->get();
  settings.upgx = map.properties.upgx->get();
  settings.upgr = map.properties.upgr->get();
  settings.pupx = map.properties.pupx->get();
  settings.tecs = map.properties.tecs->get();
  settings.tecx = map.properties.tecx->get();
  settings.ptec = map.properties.ptec->get();
  settings.ptex = map.properties.ptex->get();

  for (unsigned i = 0; i < Chk::TotalForces; ++i) {
    Chk::Force force = Chk::Force(i);
    bool useDefaultForceName = map.strings.getForceNameStringId(force) == Chk::StringId::NoString;

    settings.useDefaultForceNames[i] = useDefaultForceName;
    if (!useDefaultForceName)
      settings.forceNames[i] = *map.strings.getForceName<RawString>(force);
  }

  for (unsigned i = 0; i < Sc::Unit::TotalTypes; ++i) {
    Sc::Unit::Type unitType = Sc::Unit::Type(i);
    bool useDefaultUnitName = map.strings.getUnitNameStringId(unitType) == Chk::StringId::NoString;

    settings.useDefaultUnitNames[i] = useDefaultUnitName;
    if (!useDefaultUnitName)
      settings.unitNames[i] = *map.strings.getUnitName<RawString>(unitType);
  }

  settings.useDefaultUnitNames[0] = false;
  settings.unitNames[0] = "BOOGALOO";
}

void ScenarioSettings::writeToMap(MapFile& map) const {
  map.players.side->get() = settings.side;
  map.players.colr->get() = settings.colr;
  map.players.forc->get() = settings.forc;
  map.players.ownr->get() = settings.ownr;
  map.players.iown->get() = settings.iown;

  map.properties.unis->get() = settings.unis;
  map.properties.unix->get() = settings.unix;
  map.properties.puni->get() = settings.puni;
  map.properties.upgs->get() = settings.upgs;
  map.properties.upgx->get() = settings.upgx;
  map.properties.upgr->get() = settings.upgr;
  map.properties.pupx->get() = settings.pupx;
  map.properties.tecs->get() = settings.tecs;
  map.properties.tecx->get() = settings.tecx;
  map.properties.ptec->get() = settings.ptec;
  map.properties.ptex->get() = settings.ptex;

  for (unsigned i = 0; i < Chk::TotalForces; ++i) {
    Chk::Force force = Chk::Force(i);
    if (settings.useDefaultForceNames[i])
      map.strings.setForceNameStringId(force, Chk::StringId::NoString);
    else
      map.strings.setForceName(force, RawString(settings.forceNames[i]));
  }

  for (unsigned i = 0; i < Sc::Unit::TotalTypes; ++i) {
    Sc::Unit::Type unitType = Sc::Unit::Type(i);
    if (settings.useDefaultUnitNames[i])
      map.strings.setUnitNameStringId(unitType, Chk::StringId::NoString);
    else
      map.strings.setUnitName(unitType, RawString(settings.unitNames[i]));
  }
}
