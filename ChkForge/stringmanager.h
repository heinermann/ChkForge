#ifndef STRINGMANAGER_H
#define STRINGMANAGER_H

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
  Ui::StringManager *ui;
};

#endif // STRINGMANAGER_H
