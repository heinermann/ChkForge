#include "tree.h"

#include <QString>
#include <QStandardItem>
#include <QFile>
#include <QMessageBox>
#include <QRegularExpression>

#include <functional>

using namespace ChkForge::Tree;

namespace ChkForge::Tree {
  QStandardItem* createTreeItem(const QString& text)
  {
    QStandardItem* item = new QStandardItem(text);
    item->setEditable(false);
    return item;
  }

  void txtToTree(QStandardItem* parent, QTextStream& txt, int category, const std::function<void(QStandardItem*)>& item_cb)
  {
    static QRegularExpression VALUE_REGEX("\\s*(?<id>\\d+)\\s+(?<name>.*)\\s*");
    static QRegularExpression START_CATEGORY_REGEX("\\s*#(?<group>.*)\\s*");
    static QRegularExpression END_CATEGORY_REGEX("\\s*##\\s*");

    while (!txt.atEnd()) {
      QString line = txt.readLine();

      auto endCategoryMatch = END_CATEGORY_REGEX.match(line);
      if (endCategoryMatch.hasMatch()) return;

      auto startCategoryMatch = START_CATEGORY_REGEX.match(line);
      if (startCategoryMatch.hasMatch()) {
        QStandardItem* newGroup = createTreeItem(startCategoryMatch.captured("group"));
        txtToTree(newGroup, txt, category, item_cb);
        parent->appendRow(newGroup);
      }
      else
      {
        auto valueMatch = VALUE_REGEX.match(line);
        if (valueMatch.hasMatch())
        {
          QStandardItem* item = createTreeItem(valueMatch.captured("name"));

          int id = valueMatch.captured("id").toInt();
          item->setData(id, ROLE_ID);
          item->setData(category, ROLE_CATEGORY);
          item->setData(category << 16 | id, ROLE_SEARCHKEY);

          if (item_cb) item_cb(item);
          parent->appendRow(item);
        }
      }
    }
  }

  void createTreeFromFile(QStandardItem* parent, const QString& resourceName, int category, const std::function<void(QStandardItem*)>& item_cb)
  {
    QFile treeFile(":/Trees/" + resourceName);
    if (!treeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("Unable to load Trees/%1: %2").arg(resourceName).arg(treeFile.errorString()));
    }
    QTextStream contents(treeFile.readAll());
    txtToTree(parent, contents, category, item_cb);
  }

}
