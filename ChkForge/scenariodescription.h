#pragma once
#include <QDialog>
#include <QStandardItemModel>

#include <memory>

namespace Ui {
  class ScenarioDescription;
}

class ScenarioDescription : public QDialog
{
  Q_OBJECT

public:
  explicit ScenarioDescription(QWidget* parent = nullptr);
  ~ScenarioDescription();

  QString name;
  QString description;

  virtual int exec() override;
  virtual void accept() override;

private:
  std::unique_ptr<Ui::ScenarioDescription> ui;

private slots:
  void on_toolButton_pressed();
};
