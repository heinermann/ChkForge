#pragma once

#include <QDialog>
#include <QStringListModel>
#include <QSortFilterProxyModel>

#include <unordered_map>

namespace Ui {
  class CharMap;
}

class CharacterWidget;

class CharMap : public QDialog {
  Q_OBJECT

public:
  explicit CharMap(QWidget* parent, const QFont& font);
  ~CharMap();

private:
  QFont font;
  Ui::CharMap* ui;

  CharacterWidget* charWidget;

  std::unordered_map<int, QStringList> categories;

  //QStringListModel mainModel;
  //QSortFilterProxyModel proxyModel;



private:
  void initCategories();

private slots:
  void on_cmb_categories_currentIndexChanged(int index);
};
