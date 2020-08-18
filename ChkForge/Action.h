#pragma once

namespace ChkForge {
  class MapContext;

  class Action {
  public:
    Action(MapContext* map) : map(map) {}
    virtual ~Action() {}

    virtual void apply() = 0;
    virtual void undo() = 0;
    
    void redo() {
      apply();
    }

  protected:
    MapContext* map;
  };

}