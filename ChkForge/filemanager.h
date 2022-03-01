#pragma once
#include <QDialog>
#include <memory>

namespace Ui {
  class FileManager;
}

class FileManager : public QDialog
{
  Q_OBJECT

public:
  explicit FileManager(QWidget *parent = nullptr);
  ~FileManager();

private:
  std::unique_ptr<Ui::FileManager> ui;
};
