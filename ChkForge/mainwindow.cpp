#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "itemtree.h"
#include "minimap.h"
#include "terrainbrush.h"
#include "mapview.h"

#include <DockAreaWidget.h>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // Create tool panels
  m_DockManager = new ads::CDockManager(this);
  m_DockManager->setStyleSheet("QMainWindow::separator{ width: 2px }");

  Minimap* minimap = new Minimap();
  ui->menu_Tool_Windows->addAction(minimap->toggleViewAction());
  minimap->SDLInit();

  createMapView();

  ads::CDockAreaWidget* leftPane = m_DockManager->addDockWidget(ads::LeftDockWidgetArea, minimap);
  leftPane->setMaximumWidth(256);

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

MapView* MainWindow::createMapView()
{
  MapView* widget = new MapView();
  ui->menu_Window->addAction(widget->toggleViewAction());

  m_DockManager->addDockWidget(ads::CenterDockWidgetArea, widget);

  widget->SDLInit();

  Minimap::g_minimap->setActiveMapView(widget);
  return widget;
}

void MainWindow::afterCreated()
{
  //createMapView();
}

MainWindow::~MainWindow()
{
  delete ui;
}