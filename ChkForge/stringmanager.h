#pragma once
#include <QDialog>

namespace Ui {
  class StringManager;
}

class StringManager : public QDialog
{
  Q_OBJECT

public:
  explicit StringManager(QWidget *parent = nullptr);
  ~StringManager();

private:
  std::unique_ptr<Ui::StringManager> ui;
};
