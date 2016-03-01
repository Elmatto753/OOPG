#include "quadscene.h"
#include <iostream>

QuadScene::QuadScene(QGLWidget *parent) : Scene(parent) {
}

QuadScene::~QuadScene() {
    if (m_init) glDeleteBuffers(1, &m_quadVBO);
}

void QuadScene::draw() {
    if (!m_init) return;

    // Retrieve our currently active shader program. If it isn't initialised, exit.
    GLint id; glGetIntegerv(GL_CURRENT_PROGRAM, &id); if (id == 0) return;

    // Check to see whether we have a vertex attribute we can write to
    GLint vertAttribLoc = glGetAttribLocation(id, "a_VertexPosition");

    // At the very least we expect a vertex position pointer
    if (vertAttribLoc == -1) {
        std::cerr << "QuadScene::draw("<<id<<") - a minimum of the \"a_VertexPosition\" attribute is required!";
        return;
    }

    // Bind the vertex array (there is only one for each call to DrawElements / DrawArrays)
    glBindVertexArray(m_quadVA);

    // Bind the vertex array attribute pointer
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glEnableVertexAttribArray(vertAttribLoc);
    glVertexAttribPointer(vertAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Draw the quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Unbind everything
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}




void QuadScene::init() {
    const GLfloat verts[18] = {-1.0f,-1.0f,-0.5f,
                               1.0f,-1.0f, -0.5f,
                               1.0f, 1.0f, -0.5f,
                               1.0f, 1.0f, -0.5f,
                               -1.0f, 1.0f, -0.5f,
                               -1.0f,-1.0f, -0.5f};

    // Create the vertex array which will store all the box array attributes
    glGenVertexArrays(1, &m_quadVA);
    glBindVertexArray(m_quadVA);

    // Create our vertex buffer object
    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, 18*sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_init=true;
}


