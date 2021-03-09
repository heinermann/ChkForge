#pragma once

#include <QGroupBox>
#include <QWidget>
#include <QString>

class TriStateGroupBox : public QGroupBox {
  Q_OBJECT

public:
  explicit TriStateGroupBox(QWidget* parent = nullptr);
  explicit TriStateGroupBox(const QString& title, QWidget* parent = nullptr);

  // Used to invert the checkbox and draw tri-state
  void paintEvent(QPaintEvent*) override;

  void setPartiallyChecked(bool partialCheck = true);


private:
  Q_DISABLE_COPY(TriStateGroupBox)

  bool partiallyChecked = false;

  void wasClicked(bool checked);
  void init();
};
