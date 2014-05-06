/*
Copyright (C) 2012 Mathias Eitz and Ronald Richter.
All rights reserved.

This file is part of the imdb library and is made available under
the terms of the BSD license (see the LICENSE file).
*/

#ifndef BOF_INDEX_H
#define BOF_INDEX_H

#include "types.h"
#include "tf_idf.h"
#include "vocabulary.h"

namespace imdb {


/**
 * @ingroup search
 * @brief Inverted index, operating on document frequency histograms represented as a vector<float>.
 *
 * There are two ways to construct an InvertedIndex:
 * -# Using the default constructor, this is only useful when loading an InvertedIndex from harddisk
 * -# Passing in the number of words that your document frequency histograms will have, this
 *    prepares the InvertedIndex such that in the next step you can add all histograms using addHistogram()
 *
 * Usage:
 * -# Building an InvertedIndex
 *  - Construct using constructor 2)
 *  - Add as many documents as desired using addHistogram()
 *  - call finalize() when done adding documents [required]
 *  - optionally call apply_tfidf(), to replace raw frequency counts by their tf-idf weights
 *  - optionally call save() to store on haddisk
 * -# Using an index to perform a query
 *  - Construct using Constructor 1)
 *  - load from harddisk
 *  - call query()
 */
class InvertedIndex
{

public:

    /**
     * @brief Document/Frequency pair storing the frequency of a term and its document id
     *
     * This is the fundamental datatype used in the InvertedIndex. It stores
     * an unsigned 32-bit integer (identifying a document/image) and a floating point
     * (describing the corresponding frequency).
     * Note that by explicitly using uint32_t, we limit ourselves
     * to a maximum of about 4 billion documents/images possibly
     * being added to the InvertedIndex.
     */
    typedef std::pair<uint32_t, float> doc_freq_pair;

    /**
     * @brief Only used for reading in a serialized version of an InvertedIndex from harddisk.
     */
    InvertedIndex();

    /**
     * @brief Used when creating a new InvertedIndex from a given set of frequency histograms.
     *
     * We assume that the vocabulary contains the terms [0, num_words-1]
     * @param num_words Total number of words in the vocabulary, i.e. each histogram you add using addHistogram() must have exactly this size.
     */
    InvertedIndex(unsigned int num_words);

	// Custom loader
	void prepare(QString folderPath, const vocabulary & voc);

    /**
     * @brief Add a frequency histogram to the InvertedIndex.
     *
     * The order in which you add documents is important: the first document added will
     * be identified by id 0, the second by id 1 (and so on) in the result of a query().
     *
     * @param histogram histogram[i] denotes the frequency of term i in the document/image the histogram has been computed from
     */
    void addHistogram(const Array1Df& histogram);


    /**
     * @brief Finalizes the index \b after the last document has been added.
     *
     * Additionally computes tf_idf weights using the raw frequency counts from the passed in collection_index.
     * So if you want to apply tf-idf weighting to 'this' index, you need to pass 'this' as the collection_index.
     * Note that the t-idf weights are stored additionally to the raw frequency counts. The tf-idf weights are normalized
     * such that the length of each document is 1 under the l2 norm. Once an index is finalized, you can store it to
     * harddisk or run query().
     *
     * @param collection_index The InvertedIndex to use term frequency statistics from when evaluating the tf_function. Note
     * that the idf_function always automatically uses this InvertedIndex.
     * @param tf tf_function to be used for weighting
     * @param idf idf_function to be used for weighting
     */
    void finalize(const InvertedIndex& collection_index, const tf_function &tf, const idf_function &idf);


    /**
     * @brief Perform a query on the InvertedIndex using the passed histogram
     *
     * The InvertedIndex computes the dot product between the passed in histogram and all documents stored in the InvertedIndex. Since
     * the query histogram is usually sparse (i.e. histogram[i] = 0 for most of the i's) this operation can be very fast. The InvertedIndex
     * returns the most similar set of documents in order of descending similarity to the query histogram. Note that we assume that the
     * tf-idf weighting has been applied to the InvertedIndex before running a query (thus all document lengths being normalized). tf-idf
     * weighting (and normalization) is applied to the query histogram using the passed in tf-idf functions.
     *
     * @param histogram Query histogram, must have the same size as the histograms added to the index
     * @param tf tf_function used for weighting the query histogram
     * @param idf idf_function used for weighting the query histogram
     * @param numResults number of best-matching documents to return
     * @param result vector of results, containing
     */
    void query(const Array1Df& histogram, const tf_function &tf, const idf_function &idf, uint numResults, std::vector<dist_idx_t>& result) const;


    inline const std::vector<std::vector<doc_freq_pair> >&    doc_frequency_list() const {return _docFrequencyList;}
    inline const std::vector<std::vector<float> >&            doc_weight_list()    const {return _docWeightList;}
    inline const Array1Duint&                       ft()                    const {return _ft;}
    inline const Array1Df&                          Ft()                    const {return _Ft;}
    inline const Array1Df&                          document_sizes()        const {return _documentSizes;}
    inline const Array1Duint&                       document_unique_sizes() const {return _documentUniqueSizes;}
    inline const std::set<uint32_t>&                unique_terms()          const {return _uniqueWords;}
    inline uint32_t                                 num_terms()             const {return _numWords;}
    inline uint32_t                                 num_documents()         const {return _numDocuments;}


    /// Convenience function to load a serialized InvertedIndex
    /// @throw std::ios_base::failure in case reading fails
    void load(const std::string& filename);

    /// Convenience function to load a serialized InvertedIndex
    /// @throw std::ios_base::failure in case writing fails
    void save(const std::string& filename) const;

    // serialization operators
    //friend std::ofstream& operator<<(std::ofstream& stream, const InvertedIndex& index);
    //friend std::ifstream& operator>>(std::ifstream& stream, InvertedIndex& index);

private:

    void apply_tfidf(const InvertedIndex& collection_index, const tf_function& tf, const idf_function& idf);

    // completely "clears" the index, we provide the default parameter
    // num_words = 0 for those cases where the number of words is not
    // known beforehand (e.g. in the default constructor, required when
    // streaming the data from hd) in order to be able to use the same
    // init() function everywhere
    void init(unsigned int num_words = 0);

    // index: term t
    // _ft[t] stores the number of documents that contain term t
    // (for a given doc, t is only counted once), so the
    // maximum value of ft for any term may be N
    Array1Duint _ft;

    // index: term t
    // _Ft[t] stores the total number of occurances of t in all documents
    // (i.e. including multiple occurences in a single doc)
    Array1Df _Ft;

    // index: document d
    // _documentSizes[d] stores the total number of terms per document
    // (multiple occurences of a term are counted)
    // Note that this needs to be a float-based measure as our index
    // supports non-integer histograms
    Array1Df _documentSizes;

    // index: document d
    // _documentUniqueSizes[d] stores the total number of
    // unique terms per document
    Array1Duint _documentUniqueSizes;

    // the set of unique terms that have been added to the Index.
    // Each word only occurs once in this set, even if it has
    // been contained in more than one document
    std::set<uint32_t> _uniqueWords;

    // total number of (unique) terms in the *vocabulary*, this
    // can be more than _uniqueWords.size()
    uint32_t _numWords;

    // total number of documents added to the index
    uint32_t _numDocuments;

    // average number of terms per document
    float _avgDocLen;

    // average number of unique terms per documents
    float _avgUniqueDocLen;

    // f_{d,t} lists, contains the raw frequency counts
    std::vector<std::vector<std::pair<uint32_t, float> > > _docFrequencyList;

    // contains the tf-idf weighted and normalized version of the frequencies
    // i.e. _docWeightList[term_id][list_id] = tf-idf(_docFrequencyList[term_id][list_id]
    Array2Df _docWeightList;


    // helps us to check that the index has been finalized before it gets saved
    bool _finalized;
};


} // end namespace

#endif // BOF_INDEX_H
