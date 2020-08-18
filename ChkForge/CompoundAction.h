#pragma once
#include "Action.h"
#include <vector>

namespace ChkForge {
  class CompoundAction : public Action {
  public:
    using Action::Action;

    CompoundAction(MapContext* map, std::initializer_list<Action> actions) : Action(map), actions(actions) {}
    virtual ~CompoundAction() {}

    virtual void apply() {
      for (Action& a : actions) {
        a.apply();
      }
    }
    virtual void undo() {
      for (auto a = actions.rbegin(); a != actions.rend(); ++a) {
        a->undo();
      }
    }

  private:
    std::vector<Action> actions;
  };
}