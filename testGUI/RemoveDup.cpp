#include "Globals.h"

#include "RemoveDup.h"
#include "ui_RemoveDup.h"

QSet<int> duplicateSet;

RemoveDup::RemoveDup(QWidget *parent) : QWidget(parent), ui(new Ui::RemoveDup)
{
    ui->setupUi(this);

    this->connect(ui->recompute, SIGNAL(clicked()), SLOT(load()));
    this->connect(ui->apply, SIGNAL(clicked()), SLOT(removeDuplicates()));

    QTimer::singleShot(10, this, SLOT(load()));
}

RemoveDup::~RemoveDup()
{
    delete ui;
}

void RemoveDup::load()
{
    duplicateSet.clear();

    ui->tree->clear();

    double threshold = ui->threshold->value();

    int N = imageFiles.size();

    QProgressDialog progress("Task in progress...", "Cancel", 0, N, this);
    progress.setWindowModality(Qt::WindowModal);

    for (int i = 0; i < N; i++)
    {
        progress.setValue(i);
        if (progress.wasCanceled())  break;

        std::vector<dist_idx_t> results = searchManager->search( QImage2Mat(imageFiles[i]) );

        double bestScore = results.at(0).first;
        results.erase(results.begin()); // remove first

        // Add similar up to threshold
        std::vector<dist_idx_t> very_similars;
        for(auto r : results) if( abs(r.first-bestScore) < threshold ) very_similars.push_back(r);

        if(very_similars.empty()) continue;

        QTreeWidgetItem * item = new QTreeWidgetItem(ui->tree);
        item->setText(0, QString("%1").arg(i));
        item->setData(0, Qt::DecorationRole, imageFiles[i].scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        item->setText(1, QString("Count %1").arg(very_similars.size()));

        for(auto r : very_similars)
        {
            QTreeWidgetItem * ritem = new QTreeWidgetItem(item);
            ritem->setText(0, QString("%1").arg(r.first));
            ritem->setData(0, Qt::DecorationRole, imageFiles[(int)r.second].scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            duplicateSet.insert( (int)r.second );
        }
    }

    ui->tree->expandAll();
    for(int i = 0; i < 3; i++) ui->tree->resizeColumnToContents(i);
}

void RemoveDup::removeDuplicates()
{
    for(auto idx : duplicateSet)
    {
        QFile( imagePaths[idx] ).remove();
    }
}
