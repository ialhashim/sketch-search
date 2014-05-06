#pragma once

#include <set>
#include <vector>
#include <stdint.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// QT
#include <QMap>
#include <QVariant>
#include <QTime>

// Types:
typedef QMap<QString,QVariant> MapType;
typedef std::vector<float> Array1Df;
typedef std::vector<Array1Df> Array2Df;
typedef std::vector<unsigned int> Array1Duint;
typedef size_t index_t;
typedef std::pair<double, index_t> dist_idx_t;

// Mapped types
Q_DECLARE_METATYPE(cv::Mat)
