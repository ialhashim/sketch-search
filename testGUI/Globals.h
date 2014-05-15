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
#include <QFile>
#include <QTextStream>

#include "../bowlib/BofSearchManager.h"
extern BofSearchManager * searchManager;
extern QVector<QImage> imageFiles;
extern QVector<QString> imagePaths;

static inline QColor qtJetColor(double v, double vmin = 0,double vmax = 1)
{
   double dv;
   if (v < vmin) v = vmin;
   if (v > vmax) v = vmax;
   dv = vmax - vmin;
   double r = 1, g = 1, b = 1;
   if (v < (vmin + 0.25 * dv)) {
      r = 0;
      g = 4 * (v - vmin) / dv;
   } else if (v < (vmin + 0.5 * dv)) {
      r = 0;
      b = 1 + 4 * (vmin + 0.25 * dv - v) / dv;
   } else if (v < (vmin + 0.75 * dv)) {
      r = 4 * (v - vmin - 0.5 * dv) / dv;
      b = 0;
   } else {
      g = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
      b = 0;
   }
   return QColor::fromRgbF(qMax(0.0, qMin(r,1.0)), qMax(0.0, qMin(g,1.0)), qMax(0.0, qMin(b,1.0)));
}
