#pragma once

#include <QPainter>
#include <QMouseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QElapsedTimer>
#include <QTimer>

#include "../bowlib/BofSearchManager.h"
extern BofSearchManager * searchManager;
extern QVector<QImage> imageFiles;
extern QVector<QString> imagePaths;

inline cv::Mat QImage2Mat(QImage const& src){
     cv::Mat tmp(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());
     cv::Mat result;
     cvtColor(tmp, result, CV_RGB2GRAY);
     return result;
}
