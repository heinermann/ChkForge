#pragma once
#include <vector>
#include "Action.h"

namespace ChkForge {
  class UndoManager {
  public:
    void addAction(const Action& action) {
      actions.erase(actions.begin() + index, actions.end());
      actions.emplace_back(action);
      index = actions.size();
    }

    bool hasUndo() {
      return !actions.empty() && index > 0;
    }

    bool hasRedo() {
      return !actions.empty() && index < actions.size();
    }

    void undo() {
      if (!hasUndo()) return;
      index--;
      actions.at(index).undo();
    }

    void redo() {
      if (!hasRedo()) return;
      actions.at(index).redo();
      index++;
    }

  private:
    std::vector<Action> actions;
    size_t index = 0;
  };
}