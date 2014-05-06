#include "BofSearchManager.h"
#include "vocabulary.h"
#include "quantizer.h"
#include "kmeans.h"
#include "histvw.h"

BofSearchManager::BofSearchManager(QString folderPath)
{
    // Read vocabulary from desk
	imdb::vocabulary voc(folderPath, 2500);
	vocab = voc.centers;

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
