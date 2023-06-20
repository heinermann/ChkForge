#include "PluginManager.h"

#include <memory>
#include <list>
#include <filesystem>
#include <sstream>
#include <iostream>

PluginLib::PluginLib(HMODULE lib, FARPROC initPlugin, FARPROC runPlugin, FARPROC pluginGetMenuString, FARPROC getPluginVersion)
  : library(lib)
  , InitPlugin(reinterpret_cast<decltype(InitPlugin)>(initPlugin))
  , RunPlugin(reinterpret_cast<decltype(RunPlugin)>(runPlugin))
  , PluginGetMenuString(reinterpret_cast<decltype(PluginGetMenuString)>(pluginGetMenuString))
  , GetPluginVersion(reinterpret_cast<decltype(GetPluginVersion)>(getPluginVersion))
{}

void PluginManager::runPlugin(std::shared_ptr<PluginLib> plugin, DWORD section) {
  CChunkData trig, mbrf, swnm, uprp, upus;

  //toChunkData(&trig, map->triggers.trig.get());
  //toChunkData(&mbrf, map->triggers.mbrf.get());
  //toChunkData(&swnm, map->triggers.swnm.get());
  //toChunkData(&uprp, map->triggers.uprp.get());
  //toChunkData(&upus, map->triggers.upus.get());

  setupEngineData();

  if (plugin->RunPlugin(&engine_data, section, &trig, &mbrf, &swnm, &uprp, &upus)) {
    //fromChunkData(&trig, map->triggers.trig.get());
    //fromChunkData(&mbrf, map->triggers.mbrf.get());
    //fromChunkData(&swnm, map->triggers.swnm.get());
    //fromChunkData(&uprp, map->triggers.uprp.get());
    //fromChunkData(&upus, map->triggers.upus.get());
    //
    //mapContext->set_unsaved();
  }
}

void PluginManager::setupEngineData() {
  engine_data = TEngineData{};
  engine_data.Size = sizeof(engine_data);
  engine_data.MapStrings = &mapStrings;
  engine_data.MapLocations = locations;
  
  engine_data.UnitCustomNames = unitNameIds;
  engine_data.ForceNames = forceNameIds;
  engine_data.UnitNames = unitNames;
}

void PluginManager::loadPlugins() {
  std::error_code silent_err;
  for (auto& entry : std::filesystem::directory_iterator("plugins", silent_err)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".sdp") continue;

    loadPlugin(entry.path());
  }
}

void PluginManager::loadPlugin(const std::filesystem::path& path) {
  auto library = loadLibrary(path);
  if (!library) return;

  auto pluginEntry = createPluginEntry(library);
  if (!pluginEntry) return;

  if (!initPlugin(pluginEntry)) return;

  libraries.emplace_back(pluginEntry);
}

HMODULE PluginManager::loadLibrary(const std::filesystem::path& path) {
  auto lib_path = path.string();
  auto library = LoadLibrary(lib_path.c_str());

  if (library == NULL) {
    std::cerr << "Failed to load SCMDraft plugin: " << lib_path << std::endl;
  }

  return library;
}

std::shared_ptr<PluginLib> PluginManager::createPluginEntry(HMODULE lib) {
  FARPROC InitPlugin = GetProcAddress(lib, "InitPlugin");
  FARPROC RunPlugin = GetProcAddress(lib, "RunPlugin");
  FARPROC PluginGetMenuString = GetProcAddress(lib, "PluginGetMenuString");
  FARPROC GetPluginVersion = GetProcAddress(lib, "GetPluginVersion");

  if (InitPlugin == nullptr || RunPlugin == nullptr || PluginGetMenuString == nullptr || GetPluginVersion == nullptr) {
    // TODO get module filename
    std::cerr << "Unable to resolve SCMDraft plugin exports: " << "(filename)" << std::endl;
    return nullptr;
  }

  return std::make_shared<PluginLib>(lib, InitPlugin, RunPlugin, PluginGetMenuString, GetPluginVersion);
}

bool PluginManager::initPlugin(std::shared_ptr<PluginLib> plugin) {
  HINSTANCE hinstance = GetModuleHandle(nullptr);
  DWORD requested_sections[8] = {};

  // FIXME: plugins may crash without an HWND, we'll see
  bool result = plugin->InitPlugin(NULL, hinstance, &SCMDAllocRam, &SCMDDeAllocRam, &SCMDReAllocRam, requested_sections);
  for (auto s : requested_sections) {
    if (s == 0) continue;
    plugin->sections.push_back(s);
  }
  return result;
}

void PluginManager::populateEngineMapData() {
  for (int i = 0; i < Chk::TotalForces; ++i) {
    //forceNameIds[i] = map->players.getForceStringId(Chk::Force(i));
  }

  for (int i = 0; i < Chk::TotalUnitTypes; ++i) {
    //Sc::Unit::Type unit = Sc::Unit::Type(i);
    //unitNameIds[i] = map->strings.getUnitNameStringId(unit);
    //unitNameStrings[i] = *map->strings.getUnitName<RawString>(unit);
    //unitNames[i] = Sc::Unit::defaultDisplayNames[i].c_str();
  }

  for (int i = 0; i < Chk::TotalLocations; ++i) {
    locations[i].Data = Chk::Location{};

    //if (i < map->layers.numLocations()) {
    //  auto loc_ptr = map->layers.getLocation(i + 1);
    //  if (loc_ptr) locations[i].Data = *loc_ptr;
    //}
    //locations[i].Exists = !locations[i].Data.isBlank();
  }
}

