#pragma once

#include <QIcon>
#include <vector>

namespace ChkForge {
  namespace Icons {
    const QIcon& getUnitIcon(int unit_type);
    const QIcon& getUpgradeIcon(int upgrade_type);
    const QIcon& getTechIcon(int tech_type);
    const QIcon& getWeaponIcon(int weapon_type);

    const std::vector<QIcon>& getAllIcons();

    void init();
  }
}
