#pragma once

#include "SICStringList.h"

void* __cdecl SCMDAllocRam(DWORD Size);
void __cdecl SCMDDeAllocRam(void* Ram);
void* __cdecl SCMDReAllocRam(void* Ram, DWORD Size);

// Various structures for arguments passed from SCMDraft2.
#pragma pack(1)

// ---> Chk::Location
// Location structure. Structure follows MRGN section format.
/*typedef struct _SCLocation {
	DWORD     x0;        // Left
	DWORD     y0;        // Top
	DWORD     x1;        // Right
	DWORD     y1;        // Bottom
	WORD     Name;       // String ID for location name.
	WORD     Elevation;  // Elevation flags.
} SCLocation;*/

// What SCMDraft2 really passes to us.
typedef struct _LocationNode {
	Chk::Location Data;   // Location data.
	BYTE          Exists;  // True: Location exists. False: Unused location
} LocationNode;
#pragma pack()


// Structure for CHK Sections
typedef struct _CChunkData {
	DWORD	ChunkSize;   // Size of section
	BYTE* ChunkData;   // Data pointer. Maybe you can use scmd_alloc, scmd_realloc
} CChunkData;


typedef struct _TEngineData {
	WORD Size;
	SCMDStringList* StatTxt;
	SCMDStringList* MapStrings;
	SCMDStringList* MapInternalStrings;
	LocationNode* MapLocations;
	DWORD* WavStringIndexii;
	void* ActionLog;			//	Also NULL
	WORD	ActionLogLevel;
	void* DataInterface;		//	Also NULL, until virtual class is implemented
	int* CurSelLocation;
	int* UnitCustomNames;	//	0 == none, everything else is off by 1, 228 entries
	int* ForceNames;			//	0 == none, everything else is off by 1, 4 entries
	const char** UnitNames;			//	char*[228] with non identical unit names
} TEngineData;

typedef struct _TPluginInfo {
	TEngineData* EngineData;
	decltype(SCMDAllocRam)* AllocMem;
	decltype(SCMDDeAllocRam)* DeleteMem;
	decltype(SCMDReAllocRam)* ResizeMem;
	HWND hWindow;
	BOOL fDone;
} TPluginInfo;
