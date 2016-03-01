#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "glinc.h"
#include "shader.h"
#include "particlesystem.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <QtOpenGL/QGLWidget>

/**
  * Our GLWidget class is pretty standard. It will manage the shader and the particle system
  * object m_psys which is drawn using the special particle system shader m_particle.
  */
class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    /// Use the format parameter to force core OpenGL functionality
    explicit GLWidget(const QGLFormat& format, QWidget *parent = 0);
    
protected:
    void initializeGL();        //< Standard function for initialising the GL window
    void resizeGL(int w, int h);//< Standard function for resizing the GL window
    void paintGL();             //< Standard paint function (called a lot)

    /// Use this to trigger an update in the positions of the particles
    void timerEvent(QTimerEvent *);
signals:
    
public slots:

private:
    void _resize(int, int);     //< Do the actual resizing without updating GL
    Shader m_particle;          //< Our particle shader
    ParticleSystem m_psys;      //< The particle system we'll be drawing
};

#endif // GLWIDGET_H
