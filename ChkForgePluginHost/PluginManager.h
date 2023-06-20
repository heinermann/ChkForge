#pragma once

#include <memory>
#include <list>
#include <vector>
#include <filesystem>

#include "win32.h"
#include "Chk.h"

#include "SCMDPlugin.h"
#include "SICStringList.h"

typedef BOOL __stdcall InitPluginFn(HWND MainWindow, HINSTANCE MainInstance, decltype(SCMDAllocRam)* AllocMem, decltype(SCMDDeAllocRam)* DeleteMem, decltype(SCMDReAllocRam)* ResizeMem, DWORD* RequestedSections);
typedef BOOL __stdcall RunPluginFn(TEngineData* EngineData, DWORD CurSection, CChunkData* Triggers, CChunkData* MissionBriefing, CChunkData* SwitchRenaming, CChunkData* UnitProperties, CChunkData* UnitPropUsage);
typedef BOOL __stdcall PluginGetMenuStringFn(DWORD Section, char* MenuStr, WORD sLen);
typedef DWORD __stdcall GetPluginVersionFn(void);

struct PluginLib {
  PluginLib(HMODULE lib, FARPROC initPlugin, FARPROC runPlugin, FARPROC pluginGetMenuString, FARPROC getPluginVersion);

  HMODULE library;
  std::vector<DWORD> sections;

  InitPluginFn* InitPlugin;
  RunPluginFn* RunPlugin;
  PluginGetMenuStringFn* PluginGetMenuString;
  GetPluginVersionFn* GetPluginVersion;
};

class PluginManager {
public:
  //PluginManager(QMainWindow* main_window);

  void loadPlugins();
  //void setTrackingMap(std::shared_ptr<ChkForge::MapContext> mapContext);

  //std::vector<QAction*> addMenuOptions(QMenu* menu);

private:
  HMODULE loadLibrary(const std::filesystem::path& path);
  void loadPlugin(const std::filesystem::path &path);
  std::shared_ptr<PluginLib> createPluginEntry(HMODULE lib);
  bool initPlugin(std::shared_ptr<PluginLib> plugin);

  void runPlugin(std::shared_ptr<PluginLib> plugin, DWORD section);
  //void toChunkData(CChunkData* data, ChkSection* section);
  //void fromChunkData(CChunkData* data, ChkSection* section);

  void setupEngineData();
  void populateEngineMapData();

private:
  //QMainWindow* mainWnd;

  std::list<std::shared_ptr<PluginLib>> libraries;
  //std::shared_ptr<MapFile> map;
  //std::shared_ptr<ChkForge::MapContext> mapContext; // for marking maps as unsaved only

  SCMDStringList mapStrings;
  TEngineData engine_data = {};
  LocationNode locations[Chk::TotalLocations];
  int unitNameIds[Chk::TotalUnitTypes] = {};
  int forceNameIds[Chk::TotalForces] = {};
  std::string unitNameStrings[Chk::TotalUnitTypes];
  const char* unitNames[Chk::TotalUnitTypes] = {};
};
