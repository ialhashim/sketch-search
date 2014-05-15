#include "Globals.h"

#include "Render3D.h"
#include "ui_Render3D.h"

#include "Viewer.h"

Render3D::Render3D(QWidget *parent) : QWidget(parent), ui(new Ui::Render3D)
{
    ui->setupUi(this);

    this->connect(ui->loadButton, SIGNAL(clicked()), SLOT(load()));

    connect(ui->search, &QPushButton::released, [=]() {
        QImage renderImg = ui->glwidget->grabFrameBuffer();
        if(renderImg.width() != 128) renderImg = renderImg.scaledToWidth(128);
        QImage input = renderImg;

        cv::Mat mat = QImageToCvMat( input );
        std::vector<dist_idx_t> results = searchManager->search( mat );

        QLayoutItem * child;
        while((child = ui->resultslayout->takeAt(0)) != 0){
            delete child->widget();
            delete child;
        }

        // Only top 'X' results
        results.resize( 6 );

        for(auto dist_idx : results){
            QLabel * w = new QLabel( QString("%2 : %1").arg(dist_idx.first).arg(dist_idx.second)  );

            QImage img = imageFiles[(int)dist_idx.second];
            {
                img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);

                QPainter painter(&img);
                painter.drawText(10,10, QString("Score : %1").arg(dist_idx.first));
                painter.fillRect( QRect(0,img.height()-5,img.width(), 5), qtJetColor(dist_idx.first, 0.2, 0.4));
            }

            w->setPixmap(QPixmap::fromImage(img));

            ui->resultslayout->addWidget(w);
        }
    });

    connect(ui->rotatebutton, &QPushButton::released, [=]() {
        ui->glwidget->timer->start(100);
    });

    ui->search->connect(ui->glwidget, SIGNAL(viewChanged()), SLOT(click()));

    QTimer::singleShot(10, this, SLOT(load()));
}

Render3D::~Render3D()
{
    delete ui;
}

void Render3D::load()
{
    ui->glwidget->isReady = false;
    ui->glwidget->vertices.clear();
    ui->glwidget->faces.clear();

    QString objFilepath = QFileDialog::getOpenFileName(0, "Load OBJ", "", "Model Files (*.obj)");

    // Read obj file
    QFile file( objFilepath );
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream inF(&file);

    while( !inF.atEnd() ){
        QString line = inF.readLine();
        if(!line.size()) continue;

        if(line.startsWith("v "))
        {
            Eigen::Vector3d point;
            QStringList verts = line.split(" ",QString::SkipEmptyParts);
            int v = 0; verts.removeFirst();
            for(auto coord : verts) point(v++) = coord.toDouble();
            ui->glwidget->vertices.push_back( point );
        }
        else if(line.startsWith("f "))
        {
            Eigen::Vector3i face;
            QStringList verts = line.split(" ",QString::SkipEmptyParts);
            int vi = 0; verts.removeFirst();
            for(auto coord : verts) face(vi++) = coord.toInt() - 1;
            ui->glwidget->faces.push_back( face );
        }
    }

    ui->glwidget->isReady = true;

    this->raise();
    this->update();
    ui->glwidget->updateGL();
}
