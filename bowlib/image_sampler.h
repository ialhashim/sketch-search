#include "types.h"

namespace imdb {

struct grid_sampler{
    // the resulting samples contain (x,y) coordinates within the sampling area
    static void sample(Array2Df& samples, const cv::Mat& image, int _numSamples = 625)
    {
        cv::Rect samplingArea(0, 0, image.size().width, image.size().height);

        uint numSamples1D = std::ceil(std::sqrt(static_cast<float>(_numSamples)));
        float stepX = samplingArea.width / static_cast<float>(numSamples1D+1);
        float stepY = samplingArea.height / static_cast<float>(numSamples1D+1);

        for (uint x = 1; x <= numSamples1D; x++) {
            uint posX = x*stepX;
            for (uint y = 1; y <= numSamples1D; y++) {
                uint posY = y*stepY;
                Array1Df p(2);
                p[0] = posX;
                p[1] = posY;
                samples.push_back(p);
            }
        }
    }
};

} // end namespace
