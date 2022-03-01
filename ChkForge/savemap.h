#pragma once
#include <QDialog>
#include <memory>

namespace Ui {
  class SaveMap;
}

class SaveMap : public QDialog
{
  Q_OBJECT

public:
  explicit SaveMap(QWidget *parent = nullptr);
  ~SaveMap();

private:
  std::unique_ptr<Ui::SaveMap> ui;
};
