#pragma once
#include <vector>
#include <memory>
#include "Action.h"

namespace ChkForge {
  class MapContext;

  class UndoManager {
  public:
    UndoManager(MapContext* map) : map(map) {}

    void addAction(std::shared_ptr<Action> action) {
      actions.erase(actions.begin() + index, actions.end());
      actions.push_back(action);
      index = actions.size();
    }

    template <class T, typename... Args>
    void applyAction(Args... args) {
      auto action = std::make_shared<T>(map, args...);
      action->apply();
      addAction(action);
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
      actions.at(index)->undo();
    }

    void redo() {
      if (!hasRedo()) return;
      actions.at(index)->redo();
      index++;
    }

  private:
    MapContext* map;
    std::vector<std::shared_ptr<Action>> actions;
    size_t index = 0;
  };
}