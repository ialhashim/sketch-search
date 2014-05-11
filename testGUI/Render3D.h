#ifndef RENDER3D_H
#define RENDER3D_H

#include <QWidget>

namespace Ui {
class Render3D;
}

class Render3D : public QWidget
{
    Q_OBJECT

public:
    explicit Render3D(QWidget *parent = 0);
    ~Render3D();

public slots:
    void load();

private:
    Ui::Render3D *ui;
};

#endif // RENDER3D_H
