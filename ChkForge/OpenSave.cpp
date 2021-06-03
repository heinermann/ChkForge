#include "OpenSave.h"

#include <QString>
#include <QObject>
#include <QStandardPaths>
#include <QFileDialog>
#include <QWidget>

#include <filesystem>

#include "Utils.h"

namespace {
  static const QString file_filter =
    QObject::tr("All Starcraft Maps") + " (*.scm *.scx *.rep);;" +
    QObject::tr("Vanilla Maps") + " (*.scm);;" +
    QObject::tr("Expansion Maps") + " (*.scx);;" +
    QObject::tr("Replays") + " (*.rep);;" +
    QObject::tr("All files") + " (*)";
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
