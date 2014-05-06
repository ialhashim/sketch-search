/*
Copyright (C) 2012 Mathias Eitz and Ronald Richter.
All rights reserved.

This file is part of the imdb library and is made available under
the terms of the BSD license (see the LICENSE file).
*/

#include "tf_idf.h"
#include "inverted_index.h"

namespace imdb {

std::shared_ptr<idf_function> make_idf(const std::string& name)
{
    if (name == "constant")      return std::make_shared<idf_constant>();
    if (name == "video_google")  return std::make_shared<idf_video_google>();
    if (name == "simple")        return std::make_shared<idf_simple>();
    if (name == "lucene")        return std::make_shared<idf_lucene>();

    // print warning message and return the most basic function
    //std::cerr << "idf_function named " << name << " not registered, returning constant function" << std::endl;
    return make_idf("constant");
}


std::shared_ptr<tf_function> make_tf(const std::string& name)
{
    if (name == "constant")      return std::make_shared<tf_constant>();
    if (name == "video_google")  return std::make_shared<tf_video_google>();
    if (name == "simple")        return std::make_shared<tf_simple>();
    if (name == "lucene")        return std::make_shared<tf_lucene>();

    // print warning message and return the most basic function
    //std::cerr << "tf_function named " << name << " not registered, returning constant function" << std::endl;
    return make_tf("constant");
}

// idf function from the Video Google paper
float idf_video_google::operator()(const InvertedIndex* index, uint term_id) const
{
    // according to the Video Google paper, we need to use Ft here, i.e.
    // "the number of occurrences of term i in the whole database".
    // This can theoretically be larger than the number of documents,
    // resulting in a result < 0. Also, a div by zero is not handled
    float ft = index->Ft()[term_id];
    return std::log(index->num_documents() / ft);
}

float tf_video_google::operator()(const InvertedIndex* index, uint term_id, uint doc_id, uint list_id) const
{
    float f_dt = index->doc_frequency_list()[term_id][list_id].second;
    uint32_t nd = index->document_sizes()[doc_id];
    return f_dt / nd;
}

float idf_simple::operator()(const InvertedIndex* index, uint term_id) const
{
    uint32_t ft = index->ft()[term_id];
    return std::log(1 + index->num_documents() / static_cast<float>(ft));
}

float tf_simple::operator()(const InvertedIndex* index, uint term_id, uint /*doc_id*/, uint list_id) const
{
    float f_dt = index->doc_frequency_list()[term_id][list_id].second;
    return 1 + std::log(f_dt);
}


float idf_lucene::operator()(const InvertedIndex* index, uint term_id) const
{
    uint32_t ft = index->ft()[term_id];
    return 1 + std::log(index->num_documents() / (1 + static_cast<float>(ft)));
}


float tf_lucene::operator()(const InvertedIndex* index, uint term_id, uint /*doc_id*/, uint list_id) const
{
    return std::sqrt(index->doc_frequency_list()[term_id][list_id].second);
}


float idf_identity::operator()(const InvertedIndex* index, uint term_id) const
{
    return static_cast<float>(index->ft()[term_id]);
}


float tf_identity::operator()(const InvertedIndex* index, uint term_id, uint /*doc_id*/, uint list_id) const
{
    return static_cast<float>(index->doc_frequency_list()[term_id][list_id].second);
}

}

