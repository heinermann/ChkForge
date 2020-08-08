#pragma once

#include <QIcon>
#include <vector>

namespace ChkForge {
  namespace Icons {
    const QIcon& getUnitIcon(int unit_type);
    const std::vector<QIcon>& getAllIcons();

    void init();
  }
}
