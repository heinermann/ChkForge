#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "itemtree.h"
#include "minimap.h"
#include "terrainbrush.h"
#include "mapview.h"

#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // Create tool panels
  m_DockManager = new ads::CDockManager(this);

  createMapView();

  ads::CDockAreaWidget* leftPane = this->createToolWindow<Minimap>(ads::LeftDockWidgetArea);
  this->createToolWindow<ItemTree>(ads::BottomDockWidgetArea, leftPane);
  this->createToolWindow<TerrainBrush>(ads::BottomDockWidgetArea, leftPane);

  // Map menu actions
  connectTrigger(ui->action_Show_All, std::bind(&MainWindow::toggleToolWindows, this, true));
  connectTrigger(ui->action_Close_All, std::bind(&MainWindow::toggleToolWindows, this, false));

  connectTrigger(ui->actionE_xit, &MainWindow::close);
}

void MainWindow::toggleToolWindows(bool isOpen)
{
  for (ads::CDockWidget* dockWidget : m_DockManager->dockWidgetsMap())
    dockWidget->toggleView(isOpen);
}

void MainWindow::createMapView()
{
  MapView* widget = new MapView();
  ui->menu_Window->addAction(widget->toggleViewAction());

  m_DockManager->addDockWidget(ads::CenterDockWidgetArea, widget);

  widget->SDLInit();
}

MainWindow::~MainWindow()
{
  delete ui;
}
