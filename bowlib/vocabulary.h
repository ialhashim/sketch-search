#pragma once

#include "types.h"

namespace imdb{

// numclusters = number of clusters/visual words to generate
class vocabulary
{
public:
    vocabulary(QString folderPath, int numclusters = 2500);

    void save(const std::string& filename) const;
	void load(const std::string &filename);

    void cluster(const Array2Df &samples, int numclusters);

	Array2Df centers;
	std::vector<Array2Df> descriptors, positions;
};

}
