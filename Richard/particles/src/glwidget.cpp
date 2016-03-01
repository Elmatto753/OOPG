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
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_POINT_SPRITE);
    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glFrontFace(GL_CCW);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // This can be used to specify the origin of the texture for the fragment shader
    //glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

    // Allows us to set the point size within the vertex program
    glEnable(GL_PROGRAM_POINT_SIZE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Initialise our shaders
    m_particle.init("shaders/particle.vs", "shaders/particle.fs");

    // Initialise our scene
    m_psys.init(1000);

    // resize the window (set up the projection matrix)
    _resize(width(), height());

    // Start the timer
    startTimer(10);
}

/**
  * Use the Qt timer to update the positions of the particles.
  */
void GLWidget::timerEvent(QTimerEvent *) {
    makeCurrent();
    m_psys.update();
    updateGL();
}

/**
  * Paint the scene with the particle shader
  */
void GLWidget::paintGL() {
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_particle.bind();
    m_psys.draw();
    m_particle.unbind();

}

/**
  * This function is called automagically when the window resizes.
  */
void GLWidget::resizeGL(int w,int h) {
    _resize(w, h);
    updateGL();
}

/**
  * Internal resize function which does not trigger an updateGL(). Currently an orthogonal
  * camera is used for simplicity.
  */
void GLWidget::_resize(int, int) {
    // Use this function to ensure that the aspect ratio is maintained
    glm::mat4 proj;
    GLfloat m_cx, m_cy, m_lx, m_ly;
    m_cx = m_cy = 0.5f;
    m_lx = m_ly = 1.0f;
    GLfloat r;
    if (width() > height()) {
        r = GLfloat(width()) / GLfloat(height());
        proj = glm::ortho(m_cx - m_lx*r,
                         m_cx + m_lx*r,
                         m_cy - m_ly,
                         m_cy + m_ly,
                         -1.0f, 1.0f);
    } else {
        r = GLfloat(height()) / GLfloat(width());
        proj = glm::ortho(m_cx - m_lx,
                         m_cx + m_lx,
                         m_cy - m_ly*r,
                         m_cy + m_ly*r,
                         -1.0f, 1.0f);
    }

    // Specify the extents of our GL window
    glViewport(0,0, width(), height());
    CheckError("GLWidget::resizeGL(viewport)");

    // Set up the default camera parameters
    GLint proj_id;
    m_particle.bind();
    proj_id = glGetUniformLocation(m_particle.id(), "u_ProjectionMatrix");
    if (proj_id != -1) {
        glUniformMatrix4fv(/*location*/ proj_id,
                       /*# of matrices*/ 1,
                       /*transpose?*/ GL_FALSE,
                       /*The matrix pointer*/ glm::value_ptr(proj));
    } else {
        std::cerr << "u_ProjectionMatrix not found in shader "<<m_particle.id()<<"\n";
    }
    m_particle.unbind();
}
