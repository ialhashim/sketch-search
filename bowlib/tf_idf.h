/*
Copyright (C) 2012 Mathias Eitz and Ronald Richter.
All rights reserved.

This file is part of the imdb library and is made available under
the terms of the BSD license (see the LICENSE file).
*/

#ifndef TF_IDF_HPP
#define TF_IDF_HPP

#include "types.h"
#include <memory>

namespace imdb {

/**
 * \addtogroup tfidf
 * @{
 */

typedef unsigned int uint;

// We need to use a forward declaration here (rather than directly including
// the header file) as Index also includes this file
class InvertedIndex;

/// Base class for all idf (inverse document frequency) functions
struct idf_function {
    virtual float operator()(const InvertedIndex* index, uint term_id) const = 0;
};

/// Base class for all tf (term frequency) functions
struct tf_function {
    virtual float operator()(const InvertedIndex* index, uint term_id, uint doc_id, uint list_id) const = 0;
};

/// Constant idf_function function, returns 1.0 independently of input
struct idf_constant : public idf_function {
    float operator()(const InvertedIndex* /*index*/, uint /*term_id*/) const { return 1.0f; }
};

/// Constant tf_function, returns 1.0 independently of input
struct tf_constant : public tf_function {
    float operator()(const InvertedIndex* /*index*/, uint /*term_id*/, uint /*doc_id*/, uint /*list_id*/) const { return 1.0f; }
};

/// Indentity idf_function function, exactly returns the input frequency
struct idf_identity : public idf_function {
    float operator()(const InvertedIndex* index, uint term_id) const;
};

/// Identity tf_function, exactly returns the input frequency
struct tf_identity : public tf_function {
    float operator()(const InvertedIndex* index, uint term_id, uint doc_id, uint list_id) const;
};

/// 'Video Google' idf_function: idf = log(num_documents / freq_term_coll)
struct idf_video_google : public idf_function {
    float operator()(const InvertedIndex* index, uint term_id) const;
};

/// 'Video Google' tf_function: tf = freq_term_doc / doc_size
struct tf_video_google : public tf_function {
    float operator()(const InvertedIndex* index, uint term_id, uint doc_id, uint list_id) const;
};


/// simple idf_function, computes idf = log(1 + (num_docs / freq_term_coll))
struct idf_simple : public idf_function {
    float operator()(const InvertedIndex* index, uint term_id) const;
};

/// simple tf_function, computes tf = 1 + log(freq_term_doc)
struct tf_simple : public tf_function {
    float operator()(const InvertedIndex* index, uint term_id, uint /*doc_id*/, uint list_id) const;
};


/// default idf function as used by Lucene: idf = 1 +  log(num_documents / (1 + freq_term_coll))
struct idf_lucene : public idf_function {
    float operator()(const InvertedIndex* index, uint term_id) const;
};

/// default tf function as used by Lucene: tf = sqrt(freq_term_doc)
struct tf_lucene : public tf_function {
    float operator()(const InvertedIndex* index, uint term_id, uint /*doc_id*/, uint list_id) const;
};

/// @brief Create an idf_function by name
/// @param name Can be "constant", "identity", "video_google", "simple" or "lucene"
std::shared_ptr<idf_function> make_idf(const std::string& name);

/// @brief Create an tf_function by name
/// @param name Can be "constant", "identity", "video_google", "simple" or "lucene"
std::shared_ptr<tf_function> make_tf(const std::string& name);

/** @} */  // end ingroup

} // end namespace imdb

#endif // TF_IDF_HPP
