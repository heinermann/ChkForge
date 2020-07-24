#ifndef PREVIEWPAINTER_H
#define PREVIEWPAINTER_H

#include <QDialog>

namespace Ui {
  class PreviewPainter;
}

class PreviewPainter : public QDialog
{
  Q_OBJECT

public:
  explicit PreviewPainter(QWidget *parent = nullptr);
  ~PreviewPainter();

private:
  Ui::PreviewPainter *ui;
};

#endif // PREVIEWPAINTER_H
