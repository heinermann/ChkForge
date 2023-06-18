#include "Utils.h"

#include <QString>

#include <string>

using namespace ChkForge;

std::string toStdString(const QString& str) {
  return std::string(str.toLatin1());   // Qt can crash when using str.toStdString()
}

pt pt::clamped(rect rct) const {
  return {
    std::clamp(x(), rct.left(), rct.right()),
    std::clamp(y(), rct.top(), rct.bottom())
  };
}
