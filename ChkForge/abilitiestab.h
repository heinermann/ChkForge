#pragma once
#include <QObject>
#include <QWidget>

namespace Ui {
  class AbilitiesTab;
}

class AbilitiesTab : public QWidget {
  Q_OBJECT
public:
  explicit AbilitiesTab(QWidget* parent = nullptr);
  ~AbilitiesTab();

  void setTabFocus();

private:
  Ui::AbilitiesTab* ui;

};
