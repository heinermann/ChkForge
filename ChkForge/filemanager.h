#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QDialog>

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
  Ui::FileManager *ui;
};

#endif // FILEMANAGER_H
