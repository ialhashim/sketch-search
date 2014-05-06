/*
Copyright (C) 2012 Mathias Eitz and Ronald Richter.
All rights reserved.

This file is part of the imdb library and is made available under
the terms of the BSD license (see the LICENSE file).
*/

#include "inverted_index.h"
#include "histvw.h"

#include <functional>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cassert>
#include <set>
#include <utility>
#include <queue>
#include <fstream>

#include "iofiles.h"

namespace imdb {


InvertedIndex::InvertedIndex()
{
    init();
}

InvertedIndex::InvertedIndex(unsigned int num_words)
{
    init(num_words);
}

void InvertedIndex::prepare(QString folderPath, const vocabulary & voc)
{
	QDir d(folderPath);
	QString index_file = QString("%1/index.bin").arg(d.absolutePath());

	if( QFileInfo(index_file).exists() ){
		load( index_file.toStdString() );
		return;
	}

	init( voc.centers.size() );

	std::string tf = "video_google", idf = "video_google";

	for( auto h : imdb::compute_histvw(voc) ) 
		addHistogram( h );

	finalize(*this, *imdb::make_tf(tf), *imdb::make_idf(idf));

	save( index_file.toStdString() );
}

void InvertedIndex::addHistogram(const Array1Df &histogram) {

    assert(histogram.size() == _numWords);

    // when the last document has been added, the index needs
    // to be finalized (call to finalize()). Only then
    // we are able to compute statistics over *all* documents
    _finalized = false;

    // must be float to correctly count floating point entries from
    // the histogram of visual words which is of type vector<float>
    float numWords = 0;
    int numUniqueWords = 0;

    for (size_t t = 0; t < histogram.size(); t++)
    {

        float f_dt = histogram[t];

        if (f_dt)
        {
            numWords+=f_dt;
            numUniqueWords++;

            _ft[t]++;      // count number of docs that term t occurs in
            _Ft[t]+=f_dt;  // count total number of occurences of t

            // _numDocuments is here "misused" as the index of the currently added document
            _docFrequencyList[t].push_back(std::make_pair(_numDocuments, f_dt));

            // keep track of all unique words from all histograms added to the index so far
            _uniqueWords.insert(t);
        }
    }

    _documentSizes.push_back(numWords);
    _documentUniqueSizes.push_back(numUniqueWords);

    // count number of documents added so far
    _numDocuments++;
}

void InvertedIndex::finalize(const InvertedIndex& collection_index, const tf_function& tf, const idf_function& idf) {

    // compute average document length
    _avgDocLen = 0.0f;
    for (size_t i = 0; i < _documentSizes.size(); i++) _avgDocLen += _documentSizes[i];
    _avgDocLen /= _documentSizes.size();

    // compute average unique document length
    _avgUniqueDocLen = 0.0f;
    for (size_t i = 0; i < _documentUniqueSizes.size(); i++) _avgUniqueDocLen += _documentUniqueSizes[i];
    _avgUniqueDocLen /= _documentUniqueSizes.size();

    // apply weighting
    apply_tfidf(collection_index, tf, idf);

    _finalized = true;
}



void InvertedIndex::apply_tfidf(const InvertedIndex& collection_index, const tf_function& tf, const idf_function& idf) {

    // _docWeightList should already have the correct size
    // from the init() function
    assert(_docWeightList.size() == _docFrequencyList.size());

    // compute document lengths under tf-idf weighting function
    std::vector<float> documentLengths(_numDocuments, 0);
    for (uint32_t term_id = 0; term_id < _numWords; term_id++)
    {
        size_t numListItems = _docFrequencyList[term_id].size();
        _docWeightList[term_id].resize(numListItems);

        for (size_t list_id = 0; list_id < numListItems; list_id++)
        {
            uint32_t doc_id = _docFrequencyList[term_id][list_id].first;

             // term frequency is always relative to 'this' index
            float w_tf = tf(this, term_id, doc_id, list_id);

            // inverse document frequency is always computed using
            // the statistics from the collection_index. The only purpose
            // to do this is that we can easily re-use InvertedIndex in a
            // query to compute stats of a single query histogram -- but of course
            // we need to use the idf information from the larger collection index
            float w_idf = idf(&collection_index, term_id);

            // tf * idf
            float weight = w_tf * w_idf;

            // prepare for l2 normalization
            documentLengths[doc_id] += weight*weight;

            // store tf-idf weights in an extra index
            _docWeightList[term_id][list_id] = weight;
        }
    }

    // l2 normalization
    for (uint32_t i = 0; i < _numDocuments; i++)
        documentLengths[i] = std::sqrt(documentLengths[i]);



    // one final pass over the index to normalize all tf-idf weights
    // such that the length of each document is 1 according to l2 norm
    for (uint32_t term_id = 0; term_id < _numWords; term_id++)
    {
        size_t numListItems = _docWeightList[term_id].size();
        for (size_t list_id = 0; list_id < numListItems; list_id++)
        {
            uint32_t doc_id = _docFrequencyList[term_id][list_id].first;
            _docWeightList[term_id][list_id] /= documentLengths[doc_id];
        }
    }
}



void InvertedIndex::query(const Array1Df& histogram, const tf_function &tf, const idf_function &idf, uint numResults, std::vector<dist_idx_t>& result) const
{
    using namespace std;

    // limit numResults to the maximum number of possible results
    numResults = std::min(numResults, _numDocuments);


    // Clear the vector and resize it exactly to the required size
    // so we do not experience multiple automatic vector-internal resizing steps.
    // Both operations should be extremely cheap if the vector
    // already has the correct size, i.e. when we re-use a vector from a
    // prvious query.
    result.clear();
    result.reserve(numResults);



    // Build an inverted index from the query document alone and apply
    // tf-idf weighting function (note that we need to use the collection
    // statistic from this index as only it contains the required term
    // frequency stats over all documents).
    InvertedIndex indexQuery(_numWords);
    indexQuery.addHistogram(histogram);
    indexQuery.finalize(*this, tf, idf);

    // TODO: maybe make this a member so we do not have frequent re-allocations for each query
    // TODO: test making this a map
    vector<float> accumulators(_numDocuments, 0);

    const set<uint32_t>& uniqueTerms = indexQuery.unique_terms();
    set<uint32_t>::const_iterator cit;
    for (cit = uniqueTerms.begin(); cit != uniqueTerms.end(); ++cit)
    {
        uint32_t term_id = *cit;

        // tf-idf weight of the current term in the query
        float wqt = indexQuery.doc_weight_list()[term_id][0];

        // iterate over the list of document/frequency pairs for
        // the current term term_id
        const vector<doc_freq_pair>& df_list = _docFrequencyList[term_id];
        const vector<float>& weight_list = _docWeightList[term_id];

        for (size_t list_id = 0; list_id < df_list.size(); list_id++)
        {
            uint32_t doc_id = df_list[list_id].first;

            // tf-idf weight of the current term and the document
            // in this index at list_id
            float wdt = weight_list[list_id];

            // compute dot product
            accumulators[doc_id] +=  wdt*wqt;
        }
    }


    // we use a priority_queue with std::greater as the comparator,
    // this means, that only elements will get added with a push()
    // that are greater than the currently smallest element in the queue,
    // i.e. the queue retains the largest entries from the accumulator with
    // the smallest element in the queue sorted on top of the queue
    std::priority_queue<dist_idx_t, std::vector<dist_idx_t>, std::greater<dist_idx_t> > queue;

    for (uint i = 0; i < _numDocuments; i++)
    {
        queue.push(dist_idx_t(accumulators[i], i));
        if (queue.size() > numResults) queue.pop();
    }
    assert(queue.size() <= numResults);

    // DO NOT CHANGE the limit to queue.size() in the loop,
    // since queue becomes smaller each iteration!
    for (uint i = 0; i < numResults; i++)
    {
        result.push_back(queue.top());
        queue.pop();
    }

    // need to reverse, since the smallest element out of the queue is sorted on top
    std::reverse(result.begin(), result.end());
}


void InvertedIndex::init(unsigned int num_words)
{
    _finalized = false;

    _ft.clear();
    _docFrequencyList.clear();
    _docWeightList.clear();
    _documentSizes.clear();
    _documentUniqueSizes.clear();
    _Ft.clear();
    _uniqueWords.clear();

    _numWords = num_words;
    _numDocuments = 0;
    _avgDocLen = 0;
    _avgUniqueDocLen = 0;

    _ft.resize(_numWords, 0);
    _docFrequencyList.resize(_numWords);
    _docWeightList.resize(_numWords);
    _Ft.resize(_numWords, 0);
}

void InvertedIndex::load(const std::string& filename)
{
    QFile file( QString::fromStdString(filename) );
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    in >> _numWords;
    in >> _numDocuments;
    in >> _avgDocLen;
    in >> _avgUniqueDocLen;

    _Ft = readArray1Df(in);
    _uniqueWords = readSetUnit(in);
    _ft = readArray1Duint(in);
    _docFrequencyList = readFreqList(in);
    _docWeightList = readArray2Df(in);
    _documentSizes = readArray1Df(in);
    _documentUniqueSizes = readArray1Duint(in);

	qDebug() << "Loaded index :" << QString::fromStdString(filename);
}

void InvertedIndex::save(const std::string &filename) const
{
    QFile file( QString::fromStdString(filename) );
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);

    out << uint32_t(_numWords);
    out << uint32_t(_numDocuments);
    out << float(_avgDocLen);
    out << float(_avgUniqueDocLen);

	/*
    writeArray1Df(out, _Ft);
    writeSetUint(out, _uniqueWords);
    writeArray1Duint(out, _ft);
    writeFreqList(out, _docFrequencyList);
    writeArray2Df(out, _docWeightList);
    writeArray1Df(out, _documentSizes);
    writeArray1Duint(out, _documentUniqueSizes);*/
}


} // end namespace imdb
