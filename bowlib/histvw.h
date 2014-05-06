#pragma once
#include "types.h"
#include "quantizer.h"
#include "kmeans.h"
#include "vocabulary.h"

namespace imdb{
	inline Array2Df compute_histvw(const vocabulary& voc)
	{
		Array2Df result;

		quantize_fn quantizer = quantize_hard<Array1Df, clustering::l2norm_squared<Array1Df> >();

		int N = (int)voc.descriptors.size();

		for (index_t i = 0; i < N; i++)
		{
			Array2Df samples = voc.descriptors[i];
			Array2Df positions = voc.positions[i];

			// quantize all samples contained in the current Array2Df in parallel, the
			// result is again a Array2Df which has the same size as the samples vector,
			// i.e. one quantized sample for each original sample.
			Array2Df quantized_samples;
			quantize_samples_parallel(samples, voc.centers, quantized_samples, quantizer);

			int pyramidlevels = 1;
			bool normalizeHistvw = false;

			Array1Df hist;

			for (size_t j = 0; j < pyramidlevels; j++)
			{
				Array1Df tmp;
				int res = 1 << j; // 2^j
				build_histvw(quantized_samples, voc.centers.size(), tmp, normalizeHistvw, positions, res);

				// append the current pyramid level histograms to
				// the overall histogram
				hist.insert(hist.end(), tmp.begin(), tmp.end());
			}

			result.push_back( hist );
		}

		return result;
	}
}
