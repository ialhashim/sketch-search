#pragma once
#include "types.h"
#include <QFile>
#include <QDir>

// poor-man's serialization

inline void writeArray1Df(QDataStream & out, const Array1Df& vec){
    out << quint64(vec.size());
    for(auto v : vec) out << v;
}
inline void writeArray2Df(QDataStream & out, const Array2Df& vec){
    out << quint64(vec.size());
    for(auto v : vec) writeArray1Df(out, v);
}
inline void writeVectorArray2Df(QDataStream & out, const std::vector<Array2Df>& vec){
	out << quint64(vec.size());
	for(auto v : vec) writeArray2Df(out, v);
}
inline Array1Df readArray1Df(QDataStream & in){
    Array1Df result;
    quint64 length;
    in >> length;
    for(size_t i = 0; i < length; i++){
        float v;
        in >> v;
        result.push_back(v);
    }
    return result;
}
inline Array2Df readArray2Df(QDataStream & in){
    Array2Df result;
    quint64 length;
    in >> length;
    for(size_t i = 0; i < length; i++){
        Array1Df v = readArray1Df( in );
        result.push_back(v);
    }
    return result;
}
inline std::vector<Array2Df> readVectorArray2Df(QDataStream & in){
	std::vector<Array2Df> result;
	quint64 length;
	in >> length;
	for(size_t i = 0; i < length; i++){
		Array2Df v = readArray2Df( in );
		result.push_back(v);
	}
	return result;
}

inline void writeArray1Duint(QDataStream & out, const Array1Duint& vec){
    out << quint64(vec.size());
    for(auto v : vec) out << quint32(v);
}
inline Array1Duint readArray1Duint(QDataStream & in){
    Array1Duint result;
    quint64 length;
    in >> length;
    for(size_t i = 0; i < length; i++){
        quint32 v;
        in >> v;
        result.push_back(v);
    }
    return result;
}

// Set
inline void writeSetUint(QDataStream & out, const std::set<uint32_t> & set){
    out << quint64(set.size());
    for(auto v : set) out << v;
}
inline std::set<uint32_t> readSetUnit(QDataStream & in){
    std::set<uint32_t> set;
    quint64 length;
    in >> length;
    for(size_t i = 0; i < length; i++){
        uint32_t v;
        in >> v;
        set.insert(v);
    }
    return set;
}

// Special type
inline void writeFreqList(QDataStream & out, const std::vector<std::vector<std::pair<uint32_t, float> > >& freqList){
    out << quint64(freqList.size());
    for(auto vec : freqList){
        out << quint64(vec.size());
        for(auto p : vec){
            out << p.first;
            out << p.second;
        }
    }
}

inline std::vector<std::vector<std::pair<uint32_t, float> > > readFreqList(QDataStream & in){
    std::vector<std::vector<std::pair<uint32_t, float> > > freqList;
    quint64 length;
    in >> length;
    for(size_t i = 0; i < length; i++){
        std::vector<std::pair<uint32_t, float> > vec;
        quint64 vec_length;
        in >> vec_length;
        for(size_t j = 0; j < vec_length; j++){
            std::pair<uint32_t, float> pair;
            in >> pair.first;
            in >> pair.second;
            vec.push_back(pair);
        }
        freqList.push_back(vec);
    }
    return freqList;
}
