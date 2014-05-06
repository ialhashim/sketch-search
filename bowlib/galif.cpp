#include <iostream>
#include <cmath>

#include "galif.h"
#include "image_sampler.h"
#include "utilities.h"

#pragma warning(disable: 4267)

namespace imdb
{

template <class T>
void generate_gaussian_filter_ocv(cv::Mat_<T>& image, double sigma)
{
    const int w = image.size().width;
    const int h = image.size().height;
    const int wh = w / 2;
    const int hh = h / 2;

    const double s = 1.0 / (sigma*sigma);

    for (int y = -hh; y < hh; y++)
    {
        size_t yy = (y + h) % h;
        for (int x = -wh; x < wh; x++)
        {
            size_t xx = (x + w) % w;
            double fx = x;
            double fy = y;
            image(yy, xx) = std::exp(-(fx*fx + fy*fy) * s);
        }
    }
}


template <class T>
void generate_gabor_filter(cv::Mat_<T>& image, double peakFreq, double theta, double sigma_x, double sigma_y)
{
    const size_t w = image.size().width;
    const size_t h = image.size().height;
    const double step_u = 1.0 / static_cast<double>(w);
    const double step_v = 1.0 / static_cast<double>(h);
    const double cos_theta = std::cos(theta);
    const double sin_theta = std::sin(theta);

    const double sigmaXSquared = sigma_x*sigma_x;
    const double sigmaYSquared = sigma_y*sigma_y;

    image.setTo(cv::Scalar(0));

    for (int ny = -1; ny <= 1; ny++)
        for (int nx = -1; nx <= 1; nx++)
        {
            double v = ny;
            for (size_t y = 0; y < h; y++)
            {
                double u = nx;
                for (size_t x = 0; x < w; x++)
                {
                    double ur = u * cos_theta - v * sin_theta;
                    double vr = u * sin_theta + v * cos_theta;

                    double tmp = ur-peakFreq;
                    double value = std::exp(-2*M_PI*M_PI * (tmp*tmp*sigmaXSquared + vr*vr*sigmaYSquared));
                    image(y, x) += value;

                    u += step_u;
                }
                v += step_v;
            }
        }
}


galif_generator::galif_generator() :
    // Size of the gabor filter: images are scaled and the remaining borders are
    // padded with white to fit exactly this size. Named image_width for compatibility
    // reasons with the sift_sketch generator
      _width              (128)
    , _numOrients         (4)
    , _peakFrequency      (0.1)
    , _line_width         (0.02)    // typical linewidth in sketch as fraction of imagesize, determines sigma_x
    , _lambda             (0.3)     // sigma_y = lambda*sigma_x
    , _featureSize        (0.1)
    , _tiles              (4)
    , _smoothHist         (true)
    , _normalizeHist      ("l2")    // can be "lowe", "l2", or "none"
    , _samplerName        ("grid")
{
    double sigma_x = _line_width*_width;
    double sigma_y = _lambda*sigma_x;

    // pad the image by 3*sigma_max, this avoids any boundary effects
    // afterwards increase size to something that fft is working efficiently on
    int paddedSize = cv::getOptimalDFTSize(_width + 3*std::max(sigma_x, sigma_y));

    std::cout << "galo padded size: " << paddedSize << std::endl;

    _filterSize = cv::Size(paddedSize, paddedSize);

    for (uint i = 0; i < _numOrients; i++)
    {
        cv::Mat_<std::complex<double> > filter(_filterSize);
        //        cv::Mat_<std::complex<double> > filter_shifted(_filterSize);
        double theta = i*M_PI/_numOrients;
        //        generate_gabor_filter_unshifted(filter, _peakFrequency, theta, sigma_x, sigma_y);
        //        fftshift_even(filter, filter_shifted);

        generate_gabor_filter(filter, _peakFrequency, theta, sigma_x, sigma_y);

        //        // TODO: check this
        //        // kill DC -- this removes the average value in the response,
        //        // which we do not want/need in our response images
        //        filter_shifted(0, 0) = 0;
        filter(0, 0) = 0;

        //        _gaborFilter.push_back(filter_shifted);
        _gaborFilter.push_back(filter);
    }


    // debug: output the filters
    //    for (uint i = 0; i < _numOrients; i++) {
    //        QString filename = "filter_" + QString::number(i) + ".png";
    //        const cv::Mat_<std::complex<double> >& filter = _gaborFilter[i];

    //        // compute magnitude of response
    //        cv::Mat mag(filter.size(), CV_32FC1);
    //        for (int r = 0; r < mag.rows; r++)
    //        for (int c = 0; c < mag.cols; c++)
    //        {
    //            const std::complex<double>& v = filter(r, c);
    //            float m = std::sqrt(v.real() * v.real() + v.imag() * v.imag());
    //            mag.at<float>(r, c) = m*255;
    //        }
    //        cv::imwrite(filename.toStdString(), mag);
    //    }
}

void galif_generator::compute(MapType& data) const
{
    // --------------------------------------------------------------
    // prerequisites:
    //
    // this generator expects a 3-channel image, with
    // each channel containing exactly the same pixel values
    //
    // the image must have a white background with black sketch lines
    // --------------------------------------------------------------

    using namespace std;
    using namespace cv;

    Mat img = data["image"].value<Mat>();
	Mat imgGray;

	img.convertTo(imgGray, CV_8UC1);

    assert(imgGray.type() == CV_8UC1);

    Mat scaled = imgGray;

    // detect keypoints on the scaled image
    // the keypoint cooredinates lie in the domain defined by
    // the scaled image size, i.e. if the image has been scaled
    // to 256x256, keypoint coordinates lie in [0,255]x[0,255]
    Array2Df keypoints;
    detect(scaled, keypoints);

    // extract local features at the given keypoints
    Array2Df features;
    vector<index_t> emptyFeatures;
    extract(scaled, keypoints, features, emptyFeatures);
    assert(features.size() == keypoints.size());
    assert(emptyFeatures.size() == keypoints.size());
    (features.size() == keypoints.size());


    // normalize keypoints to range [0,1]x[0,1] so they are
    // independent of image size
    Array2Df keypointsNormalized;
    normalizePositions(keypoints, scaled.size(), keypointsNormalized);


    // remove features that are empty, i.e. that contain
    // no sketch stroke within their area
    Array2Df featuresFiltered;
    Array2Df keypointsNormalizedFiltered;
    filterEmptyFeatures(features, keypointsNormalized, emptyFeatures, featuresFiltered, keypointsNormalizedFiltered);
    assert(featuresFiltered.size() == keypointsNormalizedFiltered.size());

    // store
    data["features"].setValue( featuresFiltered );
    data["positions"].setValue( keypointsNormalizedFiltered );
    data["numfeatures"].setValue( featuresFiltered.size() );
}

void galif_generator::detect(const cv::Mat& image, Array2Df& keypoints) const
{
    assert(image.type() == CV_8UC1);

    // let the sampler generate its sample points
    grid_sampler::sample(keypoints, image);
}

void galif_generator::extract(const cv::Mat& image, const Array2Df& keypoints, Array2Df& features, std::vector<index_t> &emptyFeatures) const
{
    assert(image.type() == CV_8UC1);

    // copy input image centered onto a white background image with
    // exactly the size of our gabor filters
    // WARNING: white background assumed!!!
    cv::Mat_<std::complex<double> > src(_filterSize, 1.0);
    cv::Mat_<unsigned char> inverted = cv::Mat_<unsigned char>::zeros(_filterSize);
    for (int r = 0; r < image.rows; r++)
        for (int c = 0; c < image.cols; c++)
        {
            // this should set the real part to the desired value
            // in the range [0,1] and the complex part to 0
            src(r, c) = static_cast<double>(image.at<unsigned char>(r, c)) * (1.0/255.0);
            inverted(r, c) = 255 - image.at<unsigned char>(r, c);
        }

    cv::Mat_<int> integral;
    cv::integral(inverted, integral, CV_32S);

    // just a sanity check that the complex part
    // is correctly default initialized to 0
    assert(src(0,0).imag() == 0);

    // filter scaled input image by directional filter bank
    // transform source to frequency domain
    cv::Mat_<std::complex<double> > src_ft(_filterSize);
    cv::dft(src, src_ft);

    // apply each filter
    std::vector<cv::Mat> responses;
    for (uint i = 0; i < _numOrients; i++)
    {
        // convolve in frequency domain (i.e. multiply spectrums)
        cv::Mat_<std::complex<double> > dst_ft(_filterSize);

        // it remains unclear what the 4th parameter stands for
        // OpenCV 2.1 doc: "The same flags as passed to dft() ; only the flag DFT_ROWS is checked for"
        cv::mulSpectrums(src_ft, _gaborFilter[i], dst_ft, 0);

        // transform back to spatial domain
        cv::Mat_<std::complex<double> > dst;
        cv::dft(dst_ft, dst, cv::DFT_INVERSE | cv::DFT_SCALE);

        // compute magnitude of response
        cv::Mat mag(image.size(), CV_32FC1);
        for (int r = 0; r < mag.rows; r++)
            for (int c = 0; c < mag.cols; c++)
            {
                const std::complex<double>& v = dst(r, c);
                float m = std::sqrt(v.real() * v.real() + v.imag() * v.imag());
                mag.at<float>(r, c) = m;
            }

        //cv::imwrite("mag.png", mag*255);

        responses.push_back(mag);
    }

    // local region size is relative to image size
    int featureSize = std::sqrt(image.size().area() * _featureSize);

    // if not multiple of _tiles then round up
    if (featureSize % _tiles)
    {
        featureSize += _tiles - (featureSize % _tiles);
    }

    int tileSize = featureSize / _tiles;
    float halfTileSize = (float) tileSize / 2;

    for (size_t i = 0; i < _numOrients; i++)
    {
        // copy response image centered into a new, larger image that contains an empty border
        // of size tileSize around all sides. This additional  border is essential to be able to
        // later compute values outside of the original image bounds
        cv::Mat framed = cv::Mat::zeros(image.rows + 2*tileSize, image.cols + 2*tileSize, CV_32FC1);
        cv::Mat image_rect_in_frame = framed(cv::Rect(tileSize, tileSize, image.cols, image.rows));
        responses[i].copyTo(image_rect_in_frame);

        if (_smoothHist)
        {
            int kernelSize = 2 * tileSize + 1;
            float gaussBlurSigma = tileSize / 3.0;

            // TODO: border type?
            cv::GaussianBlur(framed, framed, cv::Size(kernelSize, kernelSize), gaussBlurSigma, gaussBlurSigma);
        }
        else
        {
            int kernelSize = tileSize;

            // TODO: border type?
            cv::boxFilter(framed, framed, CV_32F, cv::Size(kernelSize, kernelSize), cv::Point(-1, -1), false);
        }

        // response have now size of image + 2*tileSize in each dimension
        responses[i] = framed;
    }

    // will contain a 1 at each index where the underlying patch in the
    // sketch is completely empty, i.e. contains no stroke, 0 at all other
    // indices. Therefore it is essentail that this vector has the same size
    // as the keypoints and features vector
    emptyFeatures.resize(keypoints.size(), 0);

    // collect filter responses for each keypoint/region
    for (size_t i = 0; i < keypoints.size(); i++)
    {
        const Array1Df& keypoint = keypoints[i];

        // create histogram: row <-> tile, column <-> histogram of directional responses
        Array1Df histogram(_tiles * _tiles * _numOrients, 0.0f);

        // define region
        cv::Rect rect(keypoint[0] - featureSize/2, keypoint[1] - featureSize/2, featureSize, featureSize);

        cv::Rect isec = rect & cv::Rect(0, 0, src.cols, src.rows);

        // adjust rect position by frame width
        rect.x += tileSize;
        rect.y += tileSize;

        // check if patch contains any strokes of the sketch
        int patchsum = integral(isec.tl())
                + integral(isec.br())
                - integral(isec.y, isec.x + isec.width)
                - integral(isec.y + isec.height, isec.x);

        if (patchsum == 0)
        {
            // skip this patch. It contains no strokes.
            // add empty histogram, filled with zeros,
            // will be (optionally) filtered in a later descriptor computation step
            features.push_back(histogram);
            emptyFeatures[i] = 1;
            continue;
        }

        const int ndims[3] = { _tiles, _tiles, _numOrients };
        cv::Mat_<float> hist(3, ndims, 0.0f);

        for (size_t k = 0; k < responses.size(); k++)
        {
            for (int y = rect.y + halfTileSize; y < rect.br().y; y += tileSize)
                for (int x = rect.x + halfTileSize; x < rect.br().x; x += tileSize)
                {
                    // check for out of bounds condition
                    // NOTE: we have added a frame with the size of a tile
                    if (y < 0 || x < 0 || y >= responses[k].rows || x >= responses[k].cols)
                    {
                        continue;
                    }

                    // get relative coordinates in current patch
                    int ry = y - rect.y;
                    int rx = x - rect.x;

                    // get tile indices
                    int tx = rx / tileSize;
                    int ty = ry / tileSize;

                    assert(tx >= 0 && ty >= 0);
                    assert(static_cast<uint>(tx) < _tiles && static_cast<uint>(ty)  < _tiles);

                    hist(ty, tx, k) = responses[k].at<float>(y, x);
                }
        }

        std::copy(hist.begin(), hist.end(), histogram.begin());

        if (_normalizeHist == "l2")
        {
            float sum = 0;
            for (size_t i = 0; i < histogram.size(); i++) sum += histogram[i]*histogram[i];
            sum = std::sqrt(sum)  + std::numeric_limits<float>::epsilon(); // + eps avoids div by zero
            for (size_t i = 0; i < histogram.size(); i++) histogram[i] /= sum;
        }
        else if (_normalizeHist == "lowe")
        {
            cv::Mat histwrap(histogram);
            cv::Mat tmp;
            cv::normalize(histwrap, tmp, 1, 0, cv::NORM_L1);
            tmp = cv::min(tmp, 0.2);
            cv::normalize(histwrap, histwrap, 1, 0, cv::NORM_L1);
            histogram = histwrap;
        }

        // do not normalize if user has explicitly asked for that
        else if (_normalizeHist == "none") {}

        // let the user know about the wrong parameter
        else throw std::runtime_error("unsupported histogram normalization method passed (" + _normalizeHist + ")." + "Allowed methods are : lowe, l2, none." );

        // add histogram to the set of local features for that image
        features.push_back(histogram);
    }



    //    // DEBUG
    //    _responses.clear();
    //    for (int k = 0; k < responses.size(); k++)
    //    {
    //        cv::Mat rect = responses[k](cv::Rect(tileSize, tileSize, image.cols, image.rows));
    //        _responses.push_back(rect.clone());
    //    }
}

} // namespace imdb

