#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QWindowKit/QWKWidgets/widgetwindowagent.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto agent = new QWK::WidgetWindowAgent(this);
    agent->setup(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
