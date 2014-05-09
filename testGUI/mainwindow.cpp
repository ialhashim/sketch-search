#include "Globals.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "PaintWidget.h"
#include "RemoveDup.h"

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
