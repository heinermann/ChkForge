#pragma once

#include <QGroupBox>
#include <QWidget>
#include <QString>

class TriStateGroupBox : public QGroupBox {
  Q_OBJECT;

public:
  explicit TriStateGroupBox(QWidget* parent = nullptr);
  explicit TriStateGroupBox(const QString& title, QWidget* parent = nullptr);

  // Used to draw tri-state
  void paintEvent(QPaintEvent*) override;

  void setPartiallyChecked(bool partialCheck = true);
  bool isPartiallyChecked();

protected:
  bool partiallyChecked = false;

private:
  Q_DISABLE_COPY(TriStateGroupBox);
  
  void wasClicked(bool checked);
  void init();
};
