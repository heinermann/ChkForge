#ifndef DOCKWIDGETWRAPPER_H
#define DOCKWIDGETWRAPPER_H

#include <memory>
#include <QFrame>

#include <DockManager.h>

template <class Ui_T>
class DockWidgetWrapper : public ads::CDockWidget
{
public:
  explicit DockWidgetWrapper(const QString &title, QWidget *parent = nullptr)
    : ads::CDockWidget(title, parent)
  {
    ui->setupUi(&frame);

    this->setFeature(DockWidgetDeleteOnClose, false);
    this->setFeature(DockWidgetFloatable, false);

    this->setMinimumSizeHintMode(ads::CDockWidget::eMinimumSizeHintMode::MinimumSizeHintFromContent);

    this->setWidget(&frame);
  }

  virtual ~DockWidgetWrapper() {};

  std::unique_ptr<Ui_T> ui { new Ui_T() };

private:
  QFrame frame {};
};

#endif // DOCKWIDGETWRAPPER_H
