#pragma once

#include <QLibrary>
#include <memory>
#include <list>

#include "SCMDPlugin.h"

#define WINAPI __stdcall

struct PluginLib {
  std::shared_ptr<QLibrary> library;

  BOOL(WINAPI* InitPlugin)(HWND MainWindow, HINSTANCE MainInstance, decltype(SCMDAllocRam)* AllocMem, decltype(SCMDDeAllocRam)* DeleteMem, decltype(SCMDReAllocRam)* ResizeMem, DWORD* RequestedSections);
  BOOL(WINAPI* RunPlugin)(TEngineData* EngineData, DWORD CurSection, CChunkData* Triggers, CChunkData* MissionBriefing, CChunkData* SwitchRenaming, CChunkData* UnitProperties, CChunkData* UnitPropUsage);
  BOOL(WINAPI* PluginGetMenuString)(DWORD Section, CHAR* MenuStr, WORD sLen);
  DWORD(WINAPI* GetPluginVersion)(void);
};

class PluginManager {
public:
  void loadPlugins();

  std::list<PluginLib> libraries;
};
