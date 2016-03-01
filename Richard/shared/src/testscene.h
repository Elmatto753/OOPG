#ifndef TESTSCENE_H
#define TESTSCENE_H

#include "scene.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
  * This is our big juicy scene containing teapots, buddahs and dragons for
  * your delectation. Note that there is a lot of internal wizardry to set up
  * the buffer objects. The chequered box is created from scratch in different
  * buffers, so this is a good place to see how it is done. In drawObject() you
  * will find an example of binding different vertex attribute pointers to the
  * same buffer object using stride and offset.
  *
  * Note that a shader must be bound before you can call draw().
  * \author Richard Southern
  * \date 3/10/2012
  */
class TestScene : public Scene {
    Q_OBJECT
public:
    /// Construct the scene
    TestScene(bool /*isAnimated?*/, QGLWidget */*parent*/);

    /// Delete the elements added to this scene
    ~TestScene();

    /// Draw. Note that attributes and uniforms of the current shader will be queried, so the shader must be bound.
    void draw();

    /// Initialise the scene
    void init();

protected:
    /// Overloaded function to handle keyboard input
    void timerEvent(QTimerEvent *e);

protected:
    /// Draw only the box with the chequer pattern
    void drawBox(GLuint /*shader id*/);

    /// Draw an object at the specified position with the specified scale
    void drawObject(GLuint /*shader id*/,
                    GLuint /*vertex array*/,
                    GLuint /*buffer object*/,
                    GLuint /*size of buffer*/,
                    glm::vec3 /*diffuse material*/ = glm::vec3(1.0f),
                    glm::vec3 /*translation vector*/ = glm::vec3(0.0f),
                    GLfloat /*rotation angle*/ = 0.0f,
                    glm::vec3 /*rotation axis*/ = glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec3 /*scale vector*/ = glm::vec3(1.0f) );

private:
    /// Locally stored indices of the buffer objects and arrays
    GLuint m_boxVA, m_texID, m_boxVBO, m_boxNBO, m_boxTCBO, m_boxEABO;

    /// The buffer of the buddah object
    GLuint m_buddahVA, m_buddahBO;

    /// The buffer of the dragon object
    GLuint m_dragonVA, m_dragonBO;

    /// The buffer of the teapot object
    GLuint m_teapotVA, m_teapotBO;

    /// Whether this scene is animated
    bool m_isAnimated;

    /// Animation parameters
    GLfloat m_buddahZ, m_dragonAngle, m_teapotAngle;
    bool m_buddahForward;
};

#endif // TESTSCENE_H
