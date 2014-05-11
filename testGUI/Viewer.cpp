#include "Globals.h"

#include "Viewer.h"

#define glVector3( v ) glVertex3d( v.x(), v.y(), v.z() )
#define glNormal3( v ) glNormal3d( v.x(), v.y(), v.z() )
#define glLine(v1,v2) glVector3(v1);glVector3(v2)

const double PI = std::atan(1.0)*4;
double t = PI * 1.32;

template<typename Scalar>
Eigen::Matrix<Scalar,4,4> perspective(Scalar fovy, Scalar aspect, Scalar zNear, Scalar zFar){
    Eigen::Transform<Scalar,3,Eigen::Projective> tr;
    tr.matrix().setZero();
    assert(aspect > 0);
    assert(zFar > zNear);
    Scalar radf = M_PI * fovy / 180.0;
    Scalar tan_half_fovy = std::tan(radf / 2.0);
    tr(0,0) = 1.0 / (aspect * tan_half_fovy);
    tr(1,1) = 1.0 / (tan_half_fovy);
    tr(2,2) = - (zFar + zNear) / (zFar - zNear);
    tr(3,2) = - 1.0;
    tr(2,3) = - (2.0 * zFar * zNear) / (zFar - zNear);
    return tr.matrix();
}

template<typename Derived>
Eigen::Matrix<typename Derived::Scalar,4,4> lookAt(Derived const & eye, Derived const & center, Derived const & up){
    typedef Eigen::Matrix<typename Derived::Scalar,4,4> Matrix4;
    typedef Eigen::Matrix<typename Derived::Scalar,3,1> Vector3;
    Vector3 f = (center - eye).normalized();
    Vector3 u = up.normalized();
    Vector3 s = f.cross(u).normalized();
    u = s.cross(f);
    Matrix4 mat = Matrix4::Zero();
    mat(0,0) = s.x();
    mat(0,1) = s.y();
    mat(0,2) = s.z();
    mat(0,3) = -s.dot(eye);
    mat(1,0) = u.x();
    mat(1,1) = u.y();
    mat(1,2) = u.z();
    mat(1,3) = -u.dot(eye);
    mat(2,0) = -f.x();
    mat(2,1) = -f.y();
    mat(2,2) = -f.z();
    mat(2,3) = f.dot(eye);
    mat.row(3) << 0,0,0,1;
    return mat;
}

Viewer::Viewer(QWidget *parent) : QGLWidget(parent), isReady(false)
{
    // Anti-aliasing when using QGLWidget or subclasses
    if( false )
    {
        QGLFormat glf = QGLFormat::defaultFormat();
        glf.setSamples(8);
        this->setFormat(glf);
    }

    // Rotate
    if( true )
    {
        timer = new QTimer();
        QObject::connect(timer, &QTimer::timeout, [=]() {t += 0.01;this->update(); emit( viewChanged() );});
        //timer->start(200);
    }

    xRot = yRot = zRot = 0;
}

void Viewer::initializeGL()
{
    // Set up the rendering context, define display lists etc.:
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    // Setup lights and material
    GLfloat ambientLightColor[] = {0.2f,0.2f,0.2f,1};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLightColor);

    GLfloat diffuseLightColor[] = {0.9f,0.9f,0.9f,1};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLightColor);

    GLfloat specularLightColor[] = {0.95f,0.95f,0.95f,1};
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLightColor);

    float posLight0[] = { 3, 3, 3, 0 };
    glLightfv(GL_LIGHT0, GL_POSITION, posLight0);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Specular lighting
    float specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
    glMateriali(GL_FRONT, GL_SHININESS, 56);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
}

void Viewer::resizeGL(int w, int h)
{
    // setup viewport, projection etc.:
    glViewport(0, 0, (GLint)w, (GLint)h);
}

void Viewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(false)
    {
        // Default camera:
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-2, 2, -1.5, 1.5, 1, 40);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0, 0, -3);
        glRotatef(50, 1, 0, 0);
        glRotatef(70, 0, 1, 0);

        // Draw a white grid "floor" for the tetrahedron to sit on.
        glColor3d(0.8, 0.8, 0.7);
        glBegin(GL_LINES);
        for (GLfloat i = -2.5; i <= 2.5; i += 0.25) {
            glVertex3f(i, 2.5, 0); glVertex3f(i, -2.5, 0);
            glVertex3f(2.5, i, 0); glVertex3f(-2.5, i, 0);
        }
        glEnd();

        // Draw the tetrahedron.
        if(false)
        {
            glBegin(GL_TRIANGLE_STRIP);
            glColor3f(1, 1, 1); glVertex3f(0, 0, 2);
            glColor3f(1, 0, 0); glVertex3f(-1, 1, 0);
            glColor3f(0, 1, 0); glVertex3f(1, 1, 0);
            glColor3f(0, 0, 1); glVertex3f(0, -1.4f, 0);
            glColor3f(1, 1, 1); glVertex3f(0, 0, 2);
            glColor3f(1, 0, 0); glVertex3f(-1, 1, 0);
            glEnd();
        }
    }

    if( !isReady ) return;

    // Setup camera
    Eigen::Vector3d center, eye;
    {
        // Bounding volume
        {
            Eigen::Vector3d dir( cos(t), sin(t), 0.25);

            Eigen::AlignedBox3d bbox; for(auto v : vertices) bbox.extend(v);
            double radius = bbox.sizes().norm() * 2.75;
            center = bbox.center();

            eye = center + (dir.normalized() * radius);
        }

        auto projectionMatrix = perspective<double>(20, 1.0, 0.01, 1000);
        auto cameraMatrix = lookAt< Eigen::Vector3d >(eye, center, Eigen::Vector3d(0,0,1));

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glLoadMatrixd( projectionMatrix.data() );
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixd( cameraMatrix.data() );
    }

    // Added user rotation
    {
        glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
        glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
        glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);
    }

    glColor3d(1,0,0);

    // Render geometry
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3d(0,0,0);
        glDisable(GL_LIGHTING);
        glLineWidth(6);
        //RenderMesh();
        glClear(GL_DEPTH_BUFFER_BIT);

        //glEnable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3d(0,0,0);
        RenderMesh();
    }
}

void Viewer::RenderMesh()
{
    glBegin(GL_TRIANGLES);
    for(size_t fi = 0; fi < faces.size(); fi++)
    {
        Eigen::Vector3d v0 = vertices[faces[fi][0]];
        Eigen::Vector3d v1 = vertices[faces[fi][1]];
        Eigen::Vector3d v2 = vertices[faces[fi][2]];

        Eigen::Vector3d e0 = (v1 - v0).normalized();
        Eigen::Vector3d e1 = (v2 - v0).normalized();
        Eigen::Vector3d normal = e0.cross(e1);

        glNormal3( normal );
        glVector3( v0 ); glVector3( v1 ); glVector3( v2 );
    }
    glEnd();
}

static void qNormalizeAngle(int &angle){
    while (angle < 0) angle += 360 * 16;
    while (angle > 360) angle -= 360 * 16;
}

void Viewer::setXRotation(int angle){
    qNormalizeAngle(angle);
    if (angle != xRot) xRot = angle;
}

void Viewer::setYRotation(int angle){
    qNormalizeAngle(angle);
    if (angle != yRot) yRot = angle;
}

void Viewer::setZRotation(int angle){
    qNormalizeAngle(angle);
    if (angle != zRot) zRot = angle;
}

void Viewer::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void Viewer::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }

    lastPos = event->pos();

    updateGL();
}

void Viewer::mouseReleaseEvent(QMouseEvent *)
{
    emit( viewChanged() );
}
