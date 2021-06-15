#include "SCMDPlugin.h"

#include <cstdlib>

void* __cdecl SCMDAllocRam(DWORD Size) {
  return std::malloc(Size);
}

void __cdecl SCMDDeAllocRam(void* Ram) {
  std::free(Ram);
}

void* __cdecl SCMDReAllocRam(void* Ram, DWORD Size) {
  return std::realloc(Ram, Size);
}

_CChunkData::~_CChunkData() {
  if (ChunkData != nullptr)
    SCMDDeAllocRam(ChunkData);
}
