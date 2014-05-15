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
#include <QDebug>
#include <QImage>

// Types:
typedef QMap<QString,QVariant> MapType;
typedef std::vector<float> Array1Df;
typedef std::vector<Array1Df> Array2Df;
typedef std::vector<unsigned int> Array1Duint;
typedef size_t index_t;
typedef std::pair<double, index_t> dist_idx_t;

// Mapped types
Q_DECLARE_METATYPE(cv::Mat)

// Conversion from/to cv::Mat to QImage
inline QImage cvMatToQImage( const cv::Mat &inMat ){
    switch ( inMat.type() )
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB32 );
        return image;
    }
        // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB888 );
        return image.rgbSwapped();
    }
        // 8-bit, 1 channel
    case CV_8UC1:
    {
        static QVector<QRgb>  sColorTable;
        // only create our color table once
        if ( sColorTable.isEmpty() )
        {
            for ( int i = 0; i < 256; ++i )
                sColorTable.push_back( qRgb( i, i, i ) );
        }
        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_Indexed8 );
        image.setColorTable( sColorTable );
        return image;
    }
    default:
        qDebug() << "cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
        break;
    }
    return QImage();
}

// Will force a conversion to grayscale
inline cv::Mat QImageToCvMat( const QImage &inputImage, bool inCloneImageData = true ){
    QImage swapped = inputImage.convertToFormat(QImage::Format_RGB888).rgbSwapped();
    cv::Mat colored = cv::Mat( swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).clone();
    cv::Mat gray;
    cvtColor(colored, gray, CV_BGR2GRAY);
    return gray;
}
