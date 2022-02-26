#include "OpenSave.h"

#include <QString>
#include <QObject>
#include <QStandardPaths>
#include <QFileDialog>
#include <QWidget>

#include <filesystem>

#include "Utils.h"

namespace {
  static const QString allMaps = QObject::tr("All Starcraft Maps");
  static const QString vanillaMaps = QObject::tr("Vanilla Maps");
  static const QString expansionMaps = QObject::tr("Expansion Maps");
  //: gluAll:gameCloudReplays
  static const QString replays = QObject::tr("Replays");
  //: DO NOT TRANSLATE
  static const QString allFiles = QFileDialog::tr("All Files (*)");

  static const QString file_filter =
    allMaps + " (*.scm *.scx *.rep);;" +
    vanillaMaps + " (*.scm);;" +
    expansionMaps + " (*.scx);;" +
    replays + " (*.rep);;" +
    allFiles;
}

namespace OpenSave {
  std::filesystem::path getMapOpenFilename(QWidget *parent) {
    QString documents = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation).first();
    QString result = QFileDialog::getOpenFileName(parent, QString(), documents + "/Starcraft/maps", file_filter);
    
    return std::filesystem::path(toStdString(result));
  }

  std::filesystem::path getMapSaveFilename(const QString& dir, QWidget* parent) {
    QString result = QFileDialog::getSaveFileName(parent, QString(), dir, file_filter);
    
    return std::filesystem::path(toStdString(result));
  }
}
