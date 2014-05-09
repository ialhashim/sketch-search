#include "Globals.h"
BofSearchManager * searchManager = NULL;
QVector<QImage> imageFiles;
QVector<QString> imagePaths;

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
