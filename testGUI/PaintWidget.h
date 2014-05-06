#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QWidget>

namespace Ui {
class PaintWidget;
class MainWindow;
}

class PaintWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PaintWidget(QWidget *parent = 0);
    ~PaintWidget();

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    bool scribbling;
    QPoint lastPoint;
    QImage sketch;

	void setMainWindow(Ui::MainWindow * mainwindow);
	Ui::MainWindow * window;

private:
    Ui::PaintWidget *ui;

public slots:
    void run();
};

#endif // PAINTWIDGET_H
