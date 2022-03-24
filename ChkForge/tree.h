#pragma once

#include <QString>
#include <QStandardItem>

#include <functional>

namespace ChkForge::Tree {
  static const int ROLE_CATEGORY = Qt::UserRole + 1;
  static const int ROLE_ID = Qt::UserRole + 2;
  static const int ROLE_SEARCHKEY = Qt::UserRole + 3;

  QStandardItem* createTreeItem(const QString& text);
  void createTreeFromFile(QStandardItem* parent, const QString& resourceName, int category, const std::function<void(QStandardItem*)>& item_cb);
}
