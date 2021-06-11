#include "PluginManager.h"

#include <QObject>
#include <QLibrary>
#include <QMessageBox>

#include <memory>
#include <list>
#include <filesystem>

void PluginManager::loadPlugins() {
  std::error_code silent_err;
  for (auto& entry : std::filesystem::directory_iterator("plugins", silent_err)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".sdp") continue;

    auto lib_path = QString::fromStdString(entry.path().string());
    auto library = std::make_shared<QLibrary>(lib_path);

    if (!library->load()) {
      QMessageBox::warning(nullptr, QString(), QObject::tr("Failed to load SCMDraft plugin: %1").arg(lib_path));
      continue;
    }

    QFunctionPointer InitPlugin = library->resolve("InitPlugin");
    QFunctionPointer RunPlugin = library->resolve("RunPlugin");
    QFunctionPointer PluginGetMenuString = library->resolve("PluginGetMenuString");
    QFunctionPointer GetPluginVersion = library->resolve("GetPluginVersion");
    
    if (InitPlugin == nullptr || RunPlugin == nullptr || PluginGetMenuString == nullptr || GetPluginVersion == nullptr) {
      QMessageBox::warning(nullptr, QString(), QObject::tr("Unable to resolve SCMDraft plugin exports: %1").arg(lib_path));
      continue;
    }

    PluginLib pluginEntry = { library,
      reinterpret_cast<decltype(PluginLib::InitPlugin)>(InitPlugin),
      reinterpret_cast<decltype(PluginLib::RunPlugin)>(RunPlugin),
      reinterpret_cast<decltype(PluginLib::PluginGetMenuString)>(PluginGetMenuString),
      reinterpret_cast<decltype(PluginLib::GetPluginVersion)>(GetPluginVersion)
    };

    libraries.emplace_back(pluginEntry);
  }
}
