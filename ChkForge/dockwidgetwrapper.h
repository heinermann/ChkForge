#pragma once
#include <QFrame>

#include <DockManager.h>

class DockWidgetWrapper : public ads::CDockWidget
{
public:
  explicit DockWidgetWrapper(const QString &title, QWidget *parent = nullptr)
    : ads::CDockWidget(title, parent)
  {}

  virtual ~DockWidgetWrapper() {};

protected:
  void setupDockWidget() {
    this->setFeature(DockWidgetDeleteOnClose, false);
    this->setFeature(DockWidgetFloatable, false);

    this->setMinimumSizeHintMode(ads::CDockWidget::eMinimumSizeHintMode::MinimumSizeHintFromContent);

    this->setWidget(&frame);
  }

  QFrame frame {};
};
