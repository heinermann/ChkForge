#ifndef EXPORTIMAGE_H
#define EXPORTIMAGE_H

#include <QDialog>

namespace Ui {
  class ExportImage;
}

class ExportImage : public QDialog
{
  Q_OBJECT

public:
  explicit ExportImage(QWidget *parent = nullptr);
  ~ExportImage();

private:
  Ui::ExportImage *ui;
};

#endif // EXPORTIMAGE_H
