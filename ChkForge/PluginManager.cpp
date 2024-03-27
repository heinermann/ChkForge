#include "PluginManager.h"

#include <QObject>
#include <QLibrary>
#include <QMessageBox>
#include <QMenu>

#include <memory>
#include <list>
#include <filesystem>
#include <sstream>

#include "MapContext.h"

PluginLib::PluginLib(std::shared_ptr<QLibrary> lib, QFunctionPointer initPlugin, QFunctionPointer runPlugin, QFunctionPointer pluginGetMenuString, QFunctionPointer getPluginVersion)
  : library(lib)
  , InitPlugin(reinterpret_cast<decltype(InitPlugin)>(initPlugin))
  , RunPlugin(reinterpret_cast<decltype(RunPlugin)>(runPlugin))
  , PluginGetMenuString(reinterpret_cast<decltype(PluginGetMenuString)>(pluginGetMenuString))
  , GetPluginVersion(reinterpret_cast<decltype(GetPluginVersion)>(getPluginVersion))
{}


PluginManager::PluginManager(QMainWindow* main_window)
  : mainWnd(main_window)
{}

std::vector<QAction*> PluginManager::addMenuOptions(QMenu* menu) {
  std::vector<QAction*> result;

  for (auto plugin : libraries) {
    for (auto section : plugin->sections) {
      char menuString[256] = {};
      if (!plugin->PluginGetMenuString(section, menuString, sizeof(menuString))) continue;

      QAction* action = menu->addAction(menuString, [=] { this->runPlugin(plugin, section); });
      result.push_back(action);
    }
  }
  return result;
}

void PluginManager::runPlugin(std::shared_ptr<PluginLib> plugin, DWORD section) {
  CChunkData trig, mbrf, swnm, uprp, upus;

  toChunkData(&trig, Chk::SectionName::TRIG);
  toChunkData(&mbrf, Chk::SectionName::MBRF);
  toChunkData(&swnm, Chk::SectionName::SWNM);
  toChunkData(&uprp, Chk::SectionName::UPRP);
  toChunkData(&upus, Chk::SectionName::UPUS);

  setupEngineData();

  if (plugin->RunPlugin(&engine_data, section, &trig, &mbrf, &swnm, &uprp, &upus)) {
    fromChunkData(&trig, Chk::SectionName::TRIG);
    fromChunkData(&mbrf, Chk::SectionName::MBRF);
    fromChunkData(&swnm, Chk::SectionName::SWNM);
    fromChunkData(&uprp, Chk::SectionName::UPRP);
    fromChunkData(&upus, Chk::SectionName::UPUS);

    mapContext->set_unsaved();
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

void PluginManager::toChunkData(CChunkData* data, Chk::SectionName sectionName) {
  std::stringstream ss;
  map->writeSection(ss, sectionName, false);

  data->ChunkSize = ss.tellp();
  data->ChunkData = reinterpret_cast<BYTE*>(SCMDAllocRam(data->ChunkSize));
  
  ss.read(reinterpret_cast<char*>(data->ChunkData), data->ChunkSize);
}

void PluginManager::fromChunkData(CChunkData* data, Chk::SectionName sectionName) {
  Chk::SectionHeader hdr{ sectionName, Chk::SectionSize(data->ChunkSize) };

  std::stringstream ss;
  ss.write(reinterpret_cast<char*>(data->ChunkData), data->ChunkSize);

  map->read(ss, sectionName, data->ChunkSize);
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

std::shared_ptr<QLibrary> PluginManager::loadLibrary(const std::filesystem::path& path) {
  auto lib_path = QString::fromStdString(path.string());
  auto library = std::make_shared<QLibrary>(lib_path);

  if (!library->load()) {
    QMessageBox::warning(mainWnd, QString(), QObject::tr("Failed to load SCMDraft plugin: %1").arg(lib_path));
    return nullptr;
  }

  return library;
}

std::shared_ptr<PluginLib> PluginManager::createPluginEntry(std::shared_ptr<QLibrary> lib) {
  QFunctionPointer InitPlugin = lib->resolve("InitPlugin");
  QFunctionPointer RunPlugin = lib->resolve("RunPlugin");
  QFunctionPointer PluginGetMenuString = lib->resolve("PluginGetMenuString");
  QFunctionPointer GetPluginVersion = lib->resolve("GetPluginVersion");

  if (InitPlugin == nullptr || RunPlugin == nullptr || PluginGetMenuString == nullptr || GetPluginVersion == nullptr) {
    QMessageBox::warning(mainWnd, QString(), QObject::tr("Unable to resolve SCMDraft plugin exports: %1").arg(lib->fileName()));
    return nullptr;
  }

  return std::make_shared<PluginLib>(lib, InitPlugin, RunPlugin, PluginGetMenuString, GetPluginVersion);
}

bool PluginManager::initPlugin(std::shared_ptr<PluginLib> plugin) {
  HWND hwnd = reinterpret_cast<HWND>(mainWnd->winId());
  HINSTANCE hinstance = GetModuleHandle(nullptr);
  DWORD requested_sections[8] = {};

  bool result = plugin->InitPlugin(hwnd, hinstance, &SCMDAllocRam, &SCMDDeAllocRam, &SCMDReAllocRam, requested_sections);
  for (auto s : requested_sections) {
    if (s == 0) continue;
    plugin->sections.push_back(s);
  }
  return result;
}

void PluginManager::setTrackingMap(std::shared_ptr<ChkForge::MapContext> mapContext) {
  this->mapContext = mapContext;
  this->map = mapContext->chk;
  mapStrings.setTrackingMap(map);

  populateEngineMapData();
}

void PluginManager::populateEngineMapData() {
  for (int i = 0; i < Chk::TotalForces; ++i) {
    forceNameIds[i] = map->getForceNameStringId(Chk::Force(i));
  }

  for (int i = 0; i < Sc::Unit::TotalTypes; ++i) {
    Sc::Unit::Type unit = Sc::Unit::Type(i);
    unitNameIds[i] = map->getUnitNameStringId(unit);
    unitNameStrings[i] = *map->getUnitName<RawString>(unit);
    unitNames[i] = Sc::Unit::defaultDisplayNames[i].c_str();
  }

  for (int i = 0; i < Chk::TotalLocations; ++i) {
    locations[i].Data = Chk::Location{};

    if (i < map->numLocations()) {
      locations[i].Data = map->getLocation(i+1);
    }
    locations[i].Exists = !locations[i].Data.isBlank();
  }
}

