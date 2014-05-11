#include "Globals.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "PaintWidget.h"

#include "RemoveDup.h"
#include "Render3D.h"

#include <QGLWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	PaintWidget * pw = new PaintWidget();
    ui->drawinglayout->addWidget(pw);
	pw->setMainWindow( this->ui );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::removedupTool()
{
    RemoveDup * rw = new RemoveDup();
    rw->show();
    rw->raise();
}

void MainWindow::from3DShape()
{
    Render3D * rw = new Render3D();
    rw->show();
    rw->raise();
}
