#pragma once

#include <QWidget>

#include <filesystem>

namespace OpenSave {
  std::filesystem::path getMapOpenFilename(QWidget* parent = nullptr);
  std::filesystem::path getMapSaveFilename(const QString& dir = QString(), QWidget* parent = nullptr);
}
