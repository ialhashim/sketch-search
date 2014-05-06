#include "PaintWidget.h"
#include "ui_PaintWidget.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFileDialog>
#include <QLabel>

#include "../bowlib/BofSearchManager.h"
BofSearchManager * searchManager = NULL;
QVector<QImage> imageFiles;

PaintWidget::PaintWidget(QWidget *parent) : QWidget(parent), ui(new Ui::PaintWidget)
{
    ui->setupUi(this);

    this->setMouseTracking(true);
    this->scribbling = false;

    this->sketch = QImage(this->width(), this->height(), QImage::Format_ARGB32_Premultiplied);
    this->sketch.fill(QColor(255,255,255));

    QString folderPath = QFileDialog::getExistingDirectory(0,"Database folder");
    searchManager = new BofSearchManager( folderPath );

    QDir d(folderPath);
    QStringList files = d.entryList( QStringList() << "*.png" );
    for(auto file : files) imageFiles.push_back( QImage(d.absolutePath() + "/" + file) );
}

PaintWidget::~PaintWidget()
{
    delete ui;
}

void PaintWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(0,0,sketch);
}

void PaintWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && scribbling)
    {
        int prev_y = lastPoint.y();
        int prev_x = lastPoint.x();

        int y = event->pos().y();
        int x = event->pos().x();

        QPainter painter(&sketch);
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(QPoint(prev_x,prev_y), QPoint(x,y));
    }

    lastPoint = event->pos();

    update();
}

void PaintWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastPoint = event->pos();
        scribbling = true;
    }
}

void PaintWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        sketch.fill(QColor(255,255,255));
        update();
    }

    if (event->button() == Qt::LeftButton)
    {
        run();
    }

    scribbling = false;
}

cv::Mat QImage2Mat(QImage const& src)
{
     cv::Mat tmp(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());
     cv::Mat result; // deep copy just in case (my lack of knowledge with open cv)
     cvtColor(tmp, result, CV_RGB2GRAY);

     return result;
}

void PaintWidget::run()
{
    if(!searchManager) return;

    std::vector<dist_idx_t> results = searchManager->search( QImage2Mat(sketch.scaledToWidth(128)) );

	QLayoutItem * child;

	while((child = window->resultslayout->takeAt(0)) != 0){
		delete child->widget();
		delete child;
	}

	// Only top 3
	results.resize(3);

	for(auto dist_idx : results){
		QLabel * w = new QLabel( QString("%2 : %1").arg(dist_idx.first).arg(dist_idx.second)  );

		w->setPixmap(QPixmap::fromImage(imageFiles[dist_idx.second]));

		window->resultslayout->addWidget(w);
	}
}

void PaintWidget::setMainWindow(Ui::MainWindow * mainwindow)
{
	this->window = mainwindow;
}
