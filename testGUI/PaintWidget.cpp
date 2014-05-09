#include "Globals.h"

#include "PaintWidget.h"
#include "ui_PaintWidget.h"
#include "ui_mainwindow.h"

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
    for(auto file : files)
    {
        QString fullpath = d.absolutePath() + "/" + file;
        QImage img(fullpath);
        imageFiles.push_back( img );

        imagePaths << fullpath;
    }
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
        if( event->modifiers().testFlag(Qt::ControlModifier) )
        {
            QString replacmentImage = QFileDialog::getOpenFileName(0,"Open Image", "", "Image Files (*.png *.jpg *.bmp)");
            sketch = QImage(replacmentImage);
        }
        run();
    }

    scribbling = false;
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

        QImage img = imageFiles[(int)dist_idx.second];
        {
            QPainter painter(&img);
            painter.drawText(10,10, QString("Score : %1").arg(dist_idx.first));
        }

        w->setPixmap(QPixmap::fromImage(img));

		window->resultslayout->addWidget(w);
	}
}

void PaintWidget::setMainWindow(Ui::MainWindow * mainwindow)
{
	this->window = mainwindow;
}
