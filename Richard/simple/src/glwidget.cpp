#include "glwidget.h"
#include "glutil.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLWidget::GLWidget(const QGLFormat& format, QWidget *parent) :
    QGLWidget(format, parent), m_scene(this)
{
}

void GLWidget::initializeGL() {
    glewInit();
    glEnable(GL_TEXTURE_2D);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable( GL_POINT_SMOOTH );
    glEnable( GL_MULTISAMPLE_ARB);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Initialise our shaders
    m_shader.init("shaders/gouraud.vs", "shaders/gouraud.fs");


    // Initialise a scene
    m_scene.init();

    // Set the shader light parameters (fixed)
    m_shader.bind();
    glUniform4fv(glGetUniformLocation(m_shader.id(), "u_Light.Position"), 1,
                 glm::value_ptr(glm::vec4(0.2,0.2,-0.2,1.0)));
    glUniform3fv(glGetUniformLocation(m_shader.id(), "u_Light.La"), 1,
                 glm::value_ptr(glm::vec3(0.0,0.0,0.0)));
    glUniform3fv(glGetUniformLocation(m_shader.id(), "u_Light.Ld"), 1,
                 glm::value_ptr(glm::vec3(1.0,1.0,1.0)));
    glUniform3fv(glGetUniformLocation(m_shader.id(), "u_Light.Ls"), 1,
                 glm::value_ptr(glm::vec3(1.0,1.0,1.0)));
    m_shader.unbind();

    _resize(width(), height());

    m_shader.printProperties();
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    
    m_shader.bind();
    m_scene.draw();
    m_shader.unbind();
}

void GLWidget::resizeGL(int w, int h) {
    _resize(w,h);
    updateGL();
}


void GLWidget::_resize(int, int) {
    // Use this function to ensure that the aspect ratio is maintained
    glm::mat4 proj;
    GLfloat r = GLfloat(width()) / GLfloat(height());
    proj = glm::perspective(80.0f, r, 0.5f, 2.1f);

    // Specify the extents of our GL window
    glViewport(0,0, width(), height());
    CheckError("GLWidget::resizeGL(viewport)");

    // Set up the default camera parameters
    GLint proj_id;

    m_shader.bind();
    proj_id = glGetUniformLocation(m_shader.id(), "u_ProjectionMatrix");
    if (proj_id != -1) {
        glUniformMatrix4fv(/*location*/ proj_id,
                       /*# of matrices*/ 1,
                       /*transpose?*/ GL_FALSE,
                       /*The matrix pointer*/ glm::value_ptr(proj));
    } else {
        std::cerr << "u_ProjectionMatrix not found in shader "<<m_shader.id()<<"\n";
    }
    m_shader.unbind();

}

