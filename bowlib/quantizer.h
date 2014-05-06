#pragma once
#include "types.h"

#include <functional>

namespace imdb {

/**
 * @addtogroup util
 * @{
 */


/**
 * @brief Functor performing hard quantization of a sample against a given codebook of samples
 */
template <typename sample_t, typename dist_fn>
struct quantize_hard {


    /**
     * @brief Performs hard quantization of \p sample against the passed \p vocabulary.
     *
     * Computes the index of the sample in the vocabulary that has the smallest
     * distance (under the distance functor passed in via the second template parameter) to
     * the sample to be quantized. Note that we actually return a vector that contains a single 1
     * at the corresponding index. This is not really optimal performance-wise, but makes
     * the 'interface' similar to that of quantize_fuzzy such that both functors can be easily
     * exchanged for each other.
     *
     * @param sample Sample to be quantized
     * @param vocabulary Vocabulary
     * @param quantized_sample
     */
    void operator()(const sample_t& sample, const std::vector<sample_t>& vocabulary, Array1Df& quantized_sample)
    {

        // this should be very efficient in case the
        // result vector already has the correct size
        // TODO: we need to make sure that all entries
        // are zero!
        quantized_sample.resize(vocabulary.size(), 0);

        size_t closest = 0;
        float minDistance = std::numeric_limits<float>::max();

        dist_fn dist;

        for (size_t i = 0; i < vocabulary.size(); i++)
        {
            float distance = dist(sample, vocabulary[i]);
            if (distance <= minDistance)
            {
                closest = i;
                minDistance = distance;
            }
        }
        quantized_sample[closest] = 1;
    }
};


/**
 * @brief Functor performing soft quantization of a sample against a given codebook of samples
 */
template <typename sample_t, typename dist_fn>
struct quantize_fuzzy
{

    /**
     * @brief quantize_fuzzy
     * @param Standard deviation of the Gaussian used for weighting a sample
     */
    quantize_fuzzy(float sigma) : _sigma(sigma)
    {
        assert(_sigma > 0);
    }

    void operator()(const sample_t& sample, const std::vector<sample_t>& vocabulary, Array1Df& quantized_sample)
    {
        // this should be very efficient in case the
        // result vector already has the correct size
        quantized_sample.resize(vocabulary.size());

        dist_fn dist;

        float sigma2 = 2*_sigma*_sigma;
        float sum = 0;

        for (size_t i = 0; i < vocabulary.size(); i++)
        {
            float d = dist(sample, vocabulary[i]);
            float e = exp(-d*d / sigma2);
            sum += e;
            quantized_sample[i] = e;
        }

        // Normalize such that sum(result) = 1 (L1 norm)
        // The reason is that each local feature contributes the same amount of energy (=1) to the
        // resulting histogram, If we wouldn't normalize, some features (that are close to several
        // entries in the vocabulary) would contribute more energy than others.
        // This is exactly the approach taken by Chatterfield et al. -- The devil is in the details
        for (size_t i = 0; i < quantized_sample.size(); i++)
            quantized_sample[i] /= sum;
    }


    float _sigma;
};



/**
 * @brief 'Base-class' for a quantization function.
 *
 * Instead of creating a common base class we use a boost::function
 * that achieves the same effect, i.e. both quantize_hard
 * and quantize_fuzzy can be assigned to this function type
 */
//typedef boost::function<void (const Array1Df&, const Array2Df&, Array1Df&)> quantize_fn;
typedef std::function<void (const Array1Df&, const Array2Df&, Array1Df&)> quantize_fn;




/**
 * @brief Convenience function that quantizes a vector of samples in parallel
 * @param samples Vector of samples to be quantized, each sample is of vector<float>
 * @param vocabulary Vocabulary to quantize the samples against
 * @param quantized_samples A vector of the same size as the \p samples vector with each
 * entry being a vector the size of the \p vocabulary.
 * @param quantizer quantization function to be used
 */
inline void quantize_samples_parallel(const Array2Df& samples, const Array2Df& vocabulary, Array2Df& quantized_samples, quantize_fn& quantizer)
{
    quantized_samples.resize(samples.size());

    // for each word compute distances to each entry in the vocabulary ...
    #pragma omp parallel for
    for (int i = 0; i < (int)samples.size(); i++)
    {
        quantizer(samples[i], vocabulary, quantized_samples[i]);
    }
}


// Given a list of quantized samples and corresponding coordinates
// compute the (spatialized) histogram of visual words out of that.
// normalize=true normalizes the resulting histogram by the number
// of samples, this is typically only used in case of a fuzzy histogram!
//
// Note a):
// If you don't want to add any spatial information only pass in the
// first three parameters, this gives a standard BoF histogram
//
// Note b):
// we assume that the positions lie in [0,1]x[0,1]
inline void build_histvw(const Array2Df& quantized_features, size_t vocabularySize, Array1Df& histvw, bool normalize, const Array2Df& positions = Array2Df(), int res = 1)
{

    // sanity checking of the input arguments
    assert(res > 0);
    assert(vocabularySize > 0);
    //assert(quantized_features.size() > 0);
    if (res > 1) assert(positions.size() == quantized_features.size());


    //size_t vocabularySize = quantized_features[0].size();

    // length of the vector is number of cells x histogram length
    // i.e. it actually stores one histogram per cell
    histvw.resize(res*res*vocabularySize, 0);


    // Note that if quantize_features.size() == 0, the whole for
    // loop does not get executed and we end up with an all-zero
    // histogram of visual words, as expected
    for (size_t i = 0; i < quantized_features.size(); i++)
    {
        // we assume that each feature has the same length as we
        // expect them all to have been quantized against the same vocabulary
        assert(quantized_features[i].size() == vocabularySize);


        // in the case of res = 1, offset will be zero and
        // we only have a single histogram (no pyramid) and
        // thus the offset into this overall histogram will be zero
        int offset = 0;

        // ----------------------------------------------------------------
        // Special path for building a spatial pyramid
        //
        // If the user has chosen res = 1 we do not care about the content
        // of the positions vector as they are only accessed for res > 1
        if (res > 1)
        {
            int x = static_cast<int>(positions[i][0] * res);
            int y = static_cast<int>(positions[i][1] * res);
            if (x == res) x--; // handles the case positions[i][0] = 1.0
            if (y == res) y--; // handles the case positions[i][1] = 1.0

            // generate a linear index from 2D (x,y) index
            int idx = y*res + x;
            assert(idx >= 0 && idx < res*res);

            // identify the spatial histogram we want to add to
            offset = int(vocabularySize*idx);
        }
        // -----------------------------------------------------------------


        // Build up histogram by adding the quantized feature to the
        // intermediate histogram. Offset defines the spatial bin we add into
        for (size_t j = 0; j < vocabularySize; j++) {
            histvw[offset+j] += quantized_features[i][j];
        }
    }


    // for the soft features we should normalize by the number of samples
    // but we do not really want that for the hard quantized features...
    // follow the approach by Chatterfield et al.
    // The second check is to avoid division by zero. In case an empty quantized_features
    // vector is passed in, the result will be an all zero histogram
    if (normalize && quantized_features.size() > 0)
    {
        size_t numSamples = quantized_features.size();
        for (size_t i = 0; i < histvw.size(); i++)
            histvw[i] /= numSamples;
    }
}


/** @} */

} // end namespace
