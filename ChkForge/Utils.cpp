#include "Utils.h"

#include <QString>

#include <string>

std::string toStdString(const QString& str) {
  return std::string(str.toLatin1());   // Qt can crash when using str.toStdString()
}
