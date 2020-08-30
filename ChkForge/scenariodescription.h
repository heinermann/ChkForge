#ifndef SCENARIODESCRIPTION_H
#define SCENARIODESCRIPTION_H

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

private:
  Ui::ScenarioDescription* ui;

};

#endif // NEWMAP_H
