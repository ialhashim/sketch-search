#include <QDir>

#include "vocabulary.h"
#include "kmeans.h"
#include "galif.h"

#include "iofiles.h"

namespace imdb{

vocabulary::vocabulary(QString folderPath, int numclusters)
{
    imdb::galif_generator gen;
    Array2Df samples;

    QDir d(folderPath);

	QString voc_file = QString("%1/vocabulary.bin").arg(d.absolutePath());
	QString desc_file = QString("%1/features.bin").arg(d.absolutePath());
	QString pos_file = QString("%1/positions.bin").arg(d.absolutePath());
    QString index_file = QString("%1/index.bin").arg(d.absolutePath());

	// Load from disk if existing
	if( QFileInfo(voc_file).exists() && QFileInfo(desc_file).exists() && QFileInfo(pos_file).exists() )
	{
		load( voc_file.toStdString() );

        if(QFileInfo(index_file).exists()) return; // everything is ready

		QFile dfile( desc_file ); dfile.open(QIODevice::ReadOnly); QDataStream din(&dfile);
		QFile pfile( pos_file  ); pfile.open(QIODevice::ReadOnly); QDataStream pin(&pfile);
		descriptors = readVectorArray2Df(din);
		positions = readVectorArray2Df(pin);
		return;
	}

	QStringList files = d.entryList( QStringList() << "*.png" );

	std::vector<MapType> allData(files.size());

    // Load files and compute description
	#pragma omp parallel for
    for(int f = 0; f < files.size(); f++)
    {
        QString file = d.absolutePath() + "/" + files[f];

        cv::Mat input = cv::imread(file.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
        //temp = imread(file.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
        //threshold(temp, input, 200, 255, 0);

        MapType & data = allData[f];
        data["image"].setValue( input );

        // Compute features
        gen.compute( data );

		#pragma omp critical
		{
			// Add to set of features
			for(auto feature : data["features"].value<Array2Df>())
			{
				samples.push_back( feature );
			}
		}
    }

	if(samples.empty()) return;

    // Cluster the set of features
    cluster(samples, numclusters);

	// Save vocabulary to file
	std::string voc_filename = voc_file.toStdString();
	save(voc_filename);

	// Save descriptors & positions to file
	{	
		for(int f = 0; f < files.size(); f++){
			descriptors.push_back(allData[f]["features"].value<Array2Df>());
			positions.push_back(allData[f]["positions"].value<Array2Df>());
		}

		QFile dfile( desc_file ); dfile.open(QIODevice::WriteOnly); QDataStream dout(&dfile);
		QFile pfile( pos_file  ); pfile.open(QIODevice::WriteOnly); QDataStream pout(&pfile);
		writeVectorArray2Df(dout, descriptors);
		writeVectorArray2Df(pout, positions);
	}
}

void vocabulary::load(const std::string &filename){
	QFile file( QString::fromStdString(filename) );
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);

	// Read centers from file
	centers = readArray2Df(in);

	qDebug() << "Loaded vocab :" << QString::fromStdString(filename);
}

void vocabulary::save(const std::string &filename) const
{
    QFile file( QString::fromStdString(filename) );
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);

    // Cluster centers of features space
    writeArray2Df(out, centers);
}

void vocabulary::cluster(const Array2Df &samples, int numclusters)
{
    typedef clustering::l2norm_squared<Array1Df> dist_fn;

    clustering::kmeans< Array2Df, dist_fn > km(samples, numclusters);
    km.run(100, 0.01);

    centers = km.centers();
}

}
