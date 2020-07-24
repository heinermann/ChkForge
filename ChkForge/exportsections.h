#ifndef EXPORTSECTIONS_H
#define EXPORTSECTIONS_H

#include <QDialog>

namespace Ui {
  class ExportSections;
}

class ExportSections : public QDialog
{
  Q_OBJECT

public:
  explicit ExportSections(QWidget *parent = nullptr);
  ~ExportSections();

private:
  Ui::ExportSections *ui;
};

#endif // EXPORTSECTIONS_H
