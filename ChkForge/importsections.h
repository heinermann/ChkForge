#ifndef IMPORTSECTIONS_H
#define IMPORTSECTIONS_H

#include <QDialog>

namespace Ui {
  class ImportSections;
}

class ImportSections : public QDialog
{
  Q_OBJECT

public:
  explicit ImportSections(QWidget *parent = nullptr);
  ~ImportSections();

private:
  Ui::ImportSections *ui;
};

#endif // IMPORTSECTIONS_H
