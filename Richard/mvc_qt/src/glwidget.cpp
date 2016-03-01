#include "glwidget.h"
#include "glutil.h"



/**
  * Construct the scene. The format parameter forces core OpenGL functionality.
  */
GLWidget::GLWidget(const QGLFormat& format, QWidget *parent) : QGLWidget(format, parent)
{
}

/**
  * Initialise the scene.
  */
void GLWidget::initializeGL() {
    glewInit();
    m_scene.init();

    // Start the timer
    startTimer(10);
}

/**
  * Use the Qt timer to update the positions of the particles.
  */
void GLWidget::timerEvent(QTimerEvent *t) {
    makeCurrent();
    m_scene.update();
    updateGL();
}

/**
  * Paint the scene with the particle shader
  */
void GLWidget::paintGL() {    
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
    m_scene.draw();
}

/**
  * This function is called automagically when the window resizes.
  */
void GLWidget::resizeGL(int w,int h) {
    m_scene.resize(w, h);
    updateGL();
}
