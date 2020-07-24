#ifndef SAVEMAP_H
#define SAVEMAP_H

#include <QDialog>

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
  Ui::SaveMap *ui;
};

#endif // SAVEMAP_H
