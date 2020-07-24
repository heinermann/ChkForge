#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <DockManager.h>

#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow *ui;

  ads::CDockManager* m_DockManager;

  template <class T>
  ads::CDockAreaWidget* createToolWindow(ads::DockWidgetArea dockWidgetArea, ads::CDockAreaWidget* areaWidget = nullptr) {
    T* widget = new T();
    ui->menu_Tool_Windows->addAction(widget->toggleViewAction());
    return m_DockManager->addDockWidget(dockWidgetArea, widget, areaWidget);
  }

  template <typename Func>
  void connectTrigger(QAction *action, const Func &method) {
    this->connect(action, &QAction::triggered, this, method);
  }

  void toggleToolWindows(bool isOpen);
};
#endif // MAINWINDOW_H
