#pragma once

#include <QLibrary>
#include <QMainWindow>
#include <memory>
#include <list>
#include <optional>
#include <vector>

#include "SCMDPlugin.h"
#include "SICStringList.h"
#include "MapContext.h"

struct PluginLib {
  PluginLib(std::shared_ptr<QLibrary> lib, QFunctionPointer initPlugin, QFunctionPointer runPlugin, QFunctionPointer pluginGetMenuString, QFunctionPointer getPluginVersion);

  std::shared_ptr<QLibrary> library;
  std::vector<DWORD> sections;

  BOOL(__stdcall* InitPlugin)(HWND MainWindow, HINSTANCE MainInstance, decltype(SCMDAllocRam)* AllocMem, decltype(SCMDDeAllocRam)* DeleteMem, decltype(SCMDReAllocRam)* ResizeMem, DWORD* RequestedSections);
  BOOL(__stdcall* RunPlugin)(TEngineData* EngineData, DWORD CurSection, CChunkData* Triggers, CChunkData* MissionBriefing, CChunkData* SwitchRenaming, CChunkData* UnitProperties, CChunkData* UnitPropUsage);
  BOOL(__stdcall* PluginGetMenuString)(DWORD Section, char* MenuStr, WORD sLen);
  DWORD(__stdcall* GetPluginVersion)(void);
};

class PluginManager {
public:
  PluginManager(QMainWindow* main_window);

  void loadPlugins();
  void setTrackingMap(std::shared_ptr<ChkForge::MapContext> mapContext);

  std::vector<QAction*> addMenuOptions(QMenu* menu);

private:
  std::shared_ptr<QLibrary> loadLibrary(const std::filesystem::path& path);
  bool loadPlugin(const std::filesystem::path &path);
  std::optional<std::shared_ptr<PluginLib>> createPluginEntry(std::shared_ptr<QLibrary> lib);
  bool initPlugin(std::shared_ptr<PluginLib> plugin);

  void runPlugin(std::shared_ptr<PluginLib> plugin, DWORD section);
  void toChunkData(CChunkData* data, ChkSection* section);
  void fromChunkData(CChunkData* data, ChkSection* section);
  void destroyChunkData(CChunkData* data);

  void setupEngineData();

private:
  QMainWindow* mainWnd;

  std::list<std::shared_ptr<PluginLib>> libraries;
  std::shared_ptr<MapFile> map;
  std::shared_ptr<ChkForge::MapContext> mapContext; // for marking maps as unsaved only

  SCMDStringList mapStrings;
  TEngineData engine_data = {};
  LocationNode locations[Chk::TotalLocations];
  int unitNameIds[Sc::Unit::TotalTypes] = {};
  int forceNameIds[Chk::TotalForces] = {};
  std::string unitNameStrings[Sc::Unit::TotalTypes];
  const char* unitNames[Sc::Unit::TotalTypes] = {};
};
