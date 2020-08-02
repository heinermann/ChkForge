#pragma once

#include <MappingCoreLib/Scenario.h>

/**

The purpose of the map context is to
1. Connect ChkDraft and OpenBW with each other.
2. Hold any additional metadata that ChkForge will use.

*/
class MapContext
{
public:

private:
  // TODO: OpenBW context
  Scenario chk;

  // Other stuff/info (i.e. list of views that are holding the map)
};
