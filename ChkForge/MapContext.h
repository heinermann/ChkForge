#pragma once
#ifndef CHKFORGE_MAPCONTEXT_H
#define CHKFORGE_MAPCONTEXT_H

#include <MappingCoreLib/Scenario.h>

#include "../openbw/openbw/ui/ui.h"

namespace ChkForge
{
  /**

  The purpose of the map context is to
  1. Connect ChkDraft and OpenBW with each other.
  2. Hold any additional metadata that ChkForge will use.

  */
  class MapContext
  {
  public:
    MapContext();

    void reset();
    void update();

    void load_map(const std::string& map_file_str);

    bwgame::ui_functions openbw_ui;

  private:
    Scenario chk;

    // Other stuff/info (i.e. list of views that are holding the map)
  };
}

#endif
