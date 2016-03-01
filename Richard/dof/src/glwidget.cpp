#include "glwidget.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "glutil.h"

GLWidget::GLWidget(const QGLFormat& format, QWidget *parent)
    : QGLWidget(format, parent), m_scene(true, this) {
    m_cx = m_cy = 0.0f;
    m_lx = m_ly = 1.0f;
    m_depth = 0.2f;
}

GLWidget::~GLWidget() {
}

void GLWidget::initializeGL() {
    glewExperimental= GL_TRUE;
    glewInit();
    glEnable(GL_TEXTURE_2D);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable( GL_POINT_SMOOTH );
    glEnable( GL_MULTISAMPLE_ARB);
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

    // Create the shader
    m_basicProgram.init("shaders/basic.Vert", "shaders/basic.Frag");
    m_dofProgram.init("shaders/dof.Vert", "shaders/dof.Frag");

    // Set the shader light parameters (fixed)
    m_basicProgram.bind();
    glUniform4fv(glGetUniformLocation(m_basicProgram.id(), "u_Light.Position"), 1,
                 glm::value_ptr(glm::vec4(0.2,0.2,0.2,1.0)));
    glUniform3fv(glGetUniformLocation(m_basicProgram.id(), "u_Light.La"), 1,
                 glm::value_ptr(glm::vec3(0.2,0.2,0.2)));
    glUniform3fv(glGetUniformLocation(m_basicProgram.id(), "u_Light.Ld"), 1,
                 glm::value_ptr(glm::vec3(1.0,1.0,1.0)));
    glUniform3fv(glGetUniformLocation(m_basicProgram.id(), "u_Light.Ls"), 1,
                 glm::value_ptr(glm::vec3(1.0,1.0,1.0)));
    m_basicProgram.unbind();

    // Create the frame buffer
    glGenFramebuffers(1, &m_fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    // Generate a texture to write the FBO color component result to
    // DO THIS!
    glGenTextures(1, &m_fboTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
    glTexImage2D(GL_TEXTURE_2D,
                0,
                GL_RGB,
                FBO_WIDTH,
                FBO_HEIGHT,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_fboTextureId, 0);

    // The depth buffer is rendered to a texture buffer too
    // DO THIS!
    glGenTextures(1, &m_fboDepthId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_fboDepthId);
    glTexImage2D(GL_TEXTURE_2D,
                0,
                GL_DEPTH_COMPONENT,
                FBO_WIDTH,
                FBO_HEIGHT,
                0,
                GL_DEPTH_COMPONENT,
                GL_UNSIGNED_BYTE,
                NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_fboDepthId, 0);



    glGenTextures(1, &m_fboNormalId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_fboNormalId);
    glTexImage2D(GL_TEXTURE_2D,
                0,
                GL_RGB,
                FBO_WIDTH,
                FBO_HEIGHT,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_fboNormalId, 0);

    // Set the fragment shader output targets (DEPTH_ATTACHMENT is done automagically)
    GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(1, drawBufs);

    // Check it is ready to rock and roll
    CheckFrameBuffer();

    // Unbind the framebuffer to revert to default render pipeline
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create the scene
    m_scene.init();
    m_quadScene.init();

    // Set up the viewport etc.
    _resize(width(), height());
}

void GLWidget::_resize(int, int) {
    // Use this function to ensure that the aspect ratio is maintained        
    glm::mat4 proj;
    GLfloat r = GLfloat(width()) / GLfloat(height());
    proj = glm::perspective(glm::radians(80.0f), r, 0.5f, 2.1f);

    // Specify the extents of our GL window
    glViewport(0,0, width(), height());
    CheckError("GLWidget::resizeGL(viewport)");

    // Set up the default camera parameters
    GLint proj_id;
    m_basicProgram.bind();
    proj_id = glGetUniformLocation(m_basicProgram.id(), "u_ProjectionMatrix");
    if (proj_id != -1) {
        glUniformMatrix4fv(/*location*/ proj_id,
                       /*# of matrices*/ 1,
                       /*transpose?*/ GL_FALSE,
                       /*The matrix pointer*/ glm::value_ptr(proj));
    } else {
        std::cerr << "u_ProjectionMatrix not found in shader "<<m_basicProgram.id()<<"\n";
    }
    m_basicProgram.unbind();

    m_dofProgram.bind();
    // Need to store the window size for this one
    glUniform2f(glGetUniformLocation(m_dofProgram.id(), "u_WindowSize"),
                 GLfloat(width()), GLfloat(height()));
    m_dofProgram.unbind();
}

void GLWidget::resizeGL(int w, int h) {
    _resize(w,h);
    updateGL();
}

void GLWidget::paintGL() {
    // **** The stuff below is the two pass render process! Uncomment when you're ready to try it out ****
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,FBO_WIDTH,FBO_HEIGHT);
    m_basicProgram.bind();
    m_scene.draw();
    m_basicProgram.unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,width(),height());

    // Now bind our rendered image which should be in the frame buffer for the next render pass
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fboTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_fboDepthId);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_fboNormalId);

    m_dofProgram.bind();
    glUniform1i(glGetUniformLocation(m_dofProgram.id(), "u_RenderTexture"), 1);
    glUniform1i(glGetUniformLocation(m_dofProgram.id(), "u_DepthTexture"), 2);
    glUniform1i(glGetUniformLocation(m_dofProgram.id(), "u_NormalTexture"), 3);
    glUniform1f(glGetUniformLocation(m_dofProgram.id(), "u_Depth"), m_depth);
    glUniform1f(glGetUniformLocation(m_dofProgram.id(), "u_BlurRadius"), 0.008f);
    m_quadScene.draw();
    m_dofProgram.unbind();
    glBindTexture(GL_TEXTURE_2D, 0);

    // **** Comment this out when you're ready ****
//    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glViewport(0,0,width(),height());
//    m_basicProgram.bind();
//    m_scene.draw();
//    m_basicProgram.unbind();
}

/**
  * Set the depth to be in focus. Note that only values between 0 and 1 are accepted
  */
void GLWidget::setDepth(const GLfloat _d) {
    if ((_d >= 0.2f) && (_d <= 1.0f)) {
        m_depth = _d;
    }
    updateGL();
}
