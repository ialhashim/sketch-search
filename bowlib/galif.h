// Adapted code from imdb_framework
/* Copyright (C) 2012 Mathias Eitz and Ronald Richter - BSD license */

#pragma once

#include "types.h"

namespace imdb
{

class galif_generator
{
    public:
    galif_generator();

    void compute(MapType& data) const;
    void detect(const cv::Mat& image, Array2Df& keypoints) const;
    void extract(const cv::Mat& image, const Array2Df &keypoints, Array2Df& features, std::vector<index_t> &emptyFeatures) const;

    private:

    const uint         _width;
    const uint         _numOrients;
    const double       _peakFrequency;
    const double       _line_width;
    const double       _lambda;
    const double       _featureSize;
    const uint         _tiles;
    const bool         _smoothHist;
    const std::string  _normalizeHist;
    const std::string  _samplerName;

    cv::Size _filterSize;
    std::vector< cv::Mat_< std::complex<double> > > _gaborFilter;
};


} // namespace imdb
