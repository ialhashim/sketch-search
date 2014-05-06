#pragma once
#include "galif.h"
#include "inverted_index.h"

class BofSearchManager
{
public:
	BofSearchManager(QString folderPath);

	std::vector<dist_idx_t> search( QString imageFilename );
    std::vector<dist_idx_t> search( const cv::Mat& image );

    /**
     * @brief Perform a query for the most similar 'documents' on the inverted index.
     * @param histvw Histogram of visual words encoding the query 'document' (image)
     * @param num_results Desired number of results
     * @param results A vector of dist_idx_t that holds the result indices in descending order of
     * similarity (i.e. best matches are first in the vector). Any potentially existing contents
     * of this vector are cleared before the new results are added.
     */
    void query(const Array1Df& histvw, size_t num_results, std::vector<dist_idx_t>& results) const;

private:
	imdb::galif_generator gen;
	std::shared_ptr<imdb::tf_function>  _tf;
	std::shared_ptr<imdb::idf_function> _idf;

	Array2Df vocab;
	imdb::InvertedIndex index;
};
