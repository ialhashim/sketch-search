#ifndef VIEWER_H
#define VIEWER_H

#include <QGLWidget>
#include <Eigen/Core>
#include <Eigen/Geometry>

class Viewer : public QGLWidget
{
    Q_OBJECT
public:
    explicit Viewer(QWidget *parent = 0);

    std::vector<Eigen::Vector3d> vertices;
    std::vector<Eigen::VectorXi> faces;
    bool isReady;

    QTimer * timer;

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void RenderMesh();

    // Most basic mouse rotation
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    int xRot,yRot,zRot;
    QPoint lastPos;

    void setXRotation(int);
    void setYRotation(int);
    void setZRotation(int);

signals:
    void viewChanged();

public slots:

};

#endif // VIEWER_H
