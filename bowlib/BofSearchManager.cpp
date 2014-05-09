#include "BofSearchManager.h"
#include "vocabulary.h"
#include "quantizer.h"
#include "kmeans.h"
#include "histvw.h"

#include <fstream>
#include <QDir>
#include <QElapsedTimer>

BofSearchManager::BofSearchManager(QString folderPath) : folderPath(folderPath)
{
    std::cout << "Building / loading vocabulary..\n";

    // Read vocabulary from desk
    imdb::vocabulary voc(folderPath, 3000);
	vocab = voc.centers;

    std::cout << "Building / loading index..\n";

    // Indexing
    std::string tf = "video_google", idf = "video_google";
    _tf  = imdb::make_tf(tf);
    _idf = imdb::make_idf(idf);
	index.prepare(folderPath, voc);
}

std::vector<dist_idx_t> BofSearchManager::search(const cv::Mat &image)
{
    MapType data;

    data["image"].setValue( image );

    // Compute descriptor
    gen.compute( data );

    // quantize
    imdb::quantize_fn quantizer = imdb::quantize_hard<Array1Df, clustering::l2norm_squared<Array1Df> >();
    Array2Df quantized_samples;

    const Array2Df& samples = data["features"].value<Array2Df>();
    imdb::quantize_samples_parallel(samples, vocab, quantized_samples, quantizer);

    Array1Df histvw;
    imdb::build_histvw(quantized_samples, vocab.size(), histvw, false);

    // Search
    size_t in_numresults = std::numeric_limits<size_t>::max();
    std::vector<dist_idx_t> results;
    this->query( histvw, in_numresults, results );

	return results;
}

std::vector<dist_idx_t> BofSearchManager::search(QString imageFilename)
{
	cv::Mat img = cv::imread( imageFilename.toStdString(), CV_LOAD_IMAGE_GRAYSCALE );
	return search( img );
}

void BofSearchManager::query(const Array1Df &histvw, size_t num_results, std::vector<dist_idx_t> &results) const
{
    index.query(histvw, *_tf, *_idf, (imdb::uint)num_results, results);
}

void BofSearchManager::buildSimiliartyMatrix()
{
    std::cout << "Now we will build similiarty matrix..\n";

    QDir d(folderPath);
    QString absPath = d.absolutePath() + "/";
    QStringList files = d.entryList( QStringList() << "*.png" );

    size_t N = files.size();

    std::vector< std::vector<double> > M(N, std::vector<double>(N, 0));

    // Build matrix
    {
        qDebug() << "Building matrix..";
        QElapsedTimer timer; timer.start();

        // Fill up matrix
        #pragma omp parallel for
        for(int i = 0; i < N; i++)
        {
            cv::Mat curImg = cv::imread((absPath + files.at(i)).toStdString(), CV_LOAD_IMAGE_GRAYSCALE);

            std::vector<dist_idx_t> row = search(curImg);

            for(int j = 0; j < (int)row.size(); j++)
            {
                double similiarty = row[j].first;
                if(i == j) similiarty = 1;

                M[i][j] = M[j][i] = similiarty;
            }
        }

        qDebug() << "Matrix took " << timer.elapsed() << " ms\n";
    }

    // Save to file
    {
        std::ofstream out;
        out.open( qPrintable( d.absolutePath() + "/" + QString("%1_affinity.csv").arg( "affinity" ) ) );

        for(int i = 0; i < N; i++){
            QStringList vector;
            for(int j = 0; j < N; j++)
                vector << QString::number( M[i][j] );
            out << vector.join(",").toStdString() << "\n";
        }
        out.close();
    }
}
