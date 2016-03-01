#include "testscene.h"
#include <iostream>
#include <QTimerEvent>
#include "glutil.h"

// These are included from the NGL and slow down compilation dramatically because
// they are big.
#include <buddah.h>
#include <dragon.h>
#include <teapot.h>

// Need to do this because not all the ngl primitives are within the namespace
using namespace ngl;

TestScene::TestScene(bool _anim, QGLWidget *parent)
    : Scene(parent), m_isAnimated(_anim) {
    m_buddahZ = -1.5f;
    m_buddahForward = true;
    m_dragonAngle = 45.0f;
    m_teapotAngle = 0.0f;

    // Animation is through a standard Qt timer
    if (m_isAnimated) startTimer(10);


}

TestScene::~TestScene() {
    if (m_init) {
        glDeleteBuffers(1, &m_boxVBO);
        glDeleteBuffers(1, &m_boxTCBO);
        glDeleteBuffers(1, &m_boxEABO);
        glDeleteTextures(1, &m_texID);

        glDeleteBuffers(1, &m_dragonBO);
        glDeleteBuffers(1, &m_buddahBO);
        glDeleteBuffers(1, &m_teapotBO);
    }
}

void TestScene::timerEvent(QTimerEvent *) {
    m_dragonAngle += 0.5f;
    if (m_dragonAngle >= 360.0f) m_dragonAngle -= 360.0f;
    m_teapotAngle += 0.5f;
    if (m_teapotAngle >= 360.0f) m_teapotAngle -= 360.0f;

    if (m_buddahForward) {
        if (m_buddahZ >= -0.8f) {
            m_buddahForward = false;
        } else {
            m_buddahZ += 0.005f;
        }
    } else {
        if (m_buddahZ <= -1.5f) {
            m_buddahForward = true;
        } else {
            m_buddahZ -= 0.005f;
        }
    }
    emit(sceneDirty());
}

/**
  * Note that a shader must be active before you call draw. The locations must be
  * passed in the draw function (these could be passed during init?)
  */
void TestScene::draw() {
    if (!m_init) return;

    // Retrieve our currently active shader program. If it isn't initialised, exit.
    GLint id;
    glGetIntegerv(GL_CURRENT_PROGRAM, &id);
    if (id == 0) return;

    drawBox(id);
    drawObject(id, m_buddahVA, m_buddahBO, buddahSIZE,
               glm::vec3(1.0f,0.5f,0.0f),
               glm::vec3(-0.5f,-1.0f,m_buddahZ),
               0.0f,
               glm::vec3(1.0f,0.0f,0.0f),
               glm::vec3(0.08));
    drawObject(id, m_dragonVA, m_dragonBO, dragonSIZE,
               glm::vec3(1.0f,0.5f,1.0f),
               glm::vec3(0.2f,-0.2f,-1.2f),
               m_dragonAngle,
               glm::vec3(0.0f, 1.0f, 0.0f),
               glm::vec3(0.06));
    drawObject(id, m_teapotVA, m_teapotBO, teapotSIZE,
               glm::vec3(0.0f,0.5f,1.0f),
               glm::vec3(0.2f, -0.5f, -1.0f),
               m_teapotAngle,
               glm::vec3(1.0f, 1.0f, 0.0f),
               glm::vec3(0.5));
}

void TestScene::drawObject(GLuint id, GLuint va, GLuint bo, GLuint size, glm::vec3 diffuseMat, glm::vec3 trans, GLfloat angle, glm::vec3 axis, glm::vec3 scale) {
    // See if the shader program has the a_Position attribute
    GLint vertAttribLoc = glGetAttribLocation(id, "a_VertexPosition");
    GLint normAttribLoc = glGetAttribLocation(id, "a_VertexNormal");
    GLint texAttribLoc = glGetAttribLocation(id, "a_TexCoord");
    GLint texLoc = glGetUniformLocation(id, "u_Texture");
    GLint mvLoc = glGetUniformLocation(id, "u_ModelViewMatrix");
    GLint nmLoc = glGetUniformLocation(id, "u_NormalMatrix");
    GLint matLoc = glGetUniformLocation(id, "u_Material.Ka");

    // At the very least we expect a vertex position pointer
    if (vertAttribLoc == -1) {
        std::cerr << "Scene::drawObject("<<id<<") - a minimum of the \"a_VertexPosition\" attribute is required!";
        return;
    }

    // Bind the vertex array (there is only one for each call to DrawElements / DrawArrays)
    glBindVertexArray(va);

    // The buddah is stored in a single buffer, so we're going to use offsets and stride to
    // find the index of our points.
    glBindBuffer(GL_ARRAY_BUFFER, bo);
    glEnableVertexAttribArray(vertAttribLoc);
    glVertexAttribPointer(vertAttribLoc, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLfloat *)NULL + 5);

    // Set the normal attributes if they exist
    if (normAttribLoc != -1) {
        glEnableVertexAttribArray(normAttribLoc);
        glVertexAttribPointer(normAttribLoc, 3, GL_FLOAT, GL_TRUE, 8*sizeof(GLfloat), (GLfloat *)NULL + 2);
    }
    // Bind the texture array attribute pointer
//    if (texAttribLoc != -1) {
//        glEnableVertexAttribArray(texAttribLoc);
//        glVertexAttribPointer(texAttribLoc, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
//    }

    // Set the modelview matrix
    if (mvLoc != -1) {
        glm::mat4 mv(1.0f);
        mv = glm::translate(mv, trans);
        mv = glm::scale(mv, scale);
        mv = glm::rotate(mv, glm::radians(angle), axis);
        glUniformMatrix4fv(/*location*/mvLoc,
                           /*# of matrices*/1,
                           /*transpose?*/GL_FALSE,
                           /*The matrix pointer*/ glm::value_ptr(mv));
        if (nmLoc != -1) {
            // The normal matrix is the 3x3 (rotational part) of the inverse, transposed mv matrix
            glUniformMatrix3fv(nmLoc, 1, GL_TRUE, glm::value_ptr(glm::mat3(glm::inverse(mv))));
        }
    }


    if (matLoc != -1) {
        glUniform3fv(glGetUniformLocation(id, "u_Material.Ka"), 1,
                     glm::value_ptr(glm::vec3(0.2,0.2,0.2)));
        glUniform3fv(glGetUniformLocation(id, "u_Material.Kd"), 1,
                     glm::value_ptr(diffuseMat));
        glUniform3fv(glGetUniformLocation(id, "u_Material.Ks"), 1,
                     glm::value_ptr(glm::vec3(1.0,1.0,1.0)));
        glUniform1f(glGetUniformLocation(id, "u_Material.Shininess"), 16.0f);
    }

    // Activate our texture 0
    if (texLoc != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texID);
        glUniform1i(texLoc, 0); // The value is zero - the active texture
    }

    // Enable element array and draw
    glDrawArrays(GL_TRIANGLES, 0, size/8);

    // Disable the arrays where necessary
    glDisableVertexAttribArray(vertAttribLoc);
    if (normAttribLoc != -1) glDisableVertexAttribArray(normAttribLoc);
    if (texAttribLoc != -1) glDisableVertexAttribArray(texAttribLoc);
    if (texLoc != -1) glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(0);
}

void TestScene::drawBox(GLuint id) {
    // See if the shader program has the a_Position attribute
    GLint vertAttribLoc = glGetAttribLocation(id, "a_VertexPosition");
    GLint normAttribLoc = glGetAttribLocation(id, "a_VertexNormal");
    GLint texAttribLoc = glGetAttribLocation(id, "a_TexCoord");
    GLint texLoc = glGetUniformLocation(id, "u_Texture");
    GLint mvLoc = glGetUniformLocation(id, "u_ModelViewMatrix");
    GLint nmLoc = glGetUniformLocation(id, "u_NormalMatrix");
    GLint matLoc = glGetUniformLocation(id, "u_Material.Ka");

    // At the very least we expect a vertex position pointer
    if (vertAttribLoc == -1) {
        std::cerr << "Scene::drawBox("<<id<<") - a minimum of the \"a_VertexPosition\" attribute is required!";
        return;
    }

    // Bind the vertex array (there is only one for each call to DrawElements / DrawArrays)
    glBindVertexArray(m_boxVA);

    // Bind the vertex array attribute pointer
    glBindBuffer(GL_ARRAY_BUFFER, m_boxVBO);
    glEnableVertexAttribArray(vertAttribLoc);
    glVertexAttribPointer(vertAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Set the normal attributes if they exist
    if (normAttribLoc != -1) {
        glBindBuffer(GL_ARRAY_BUFFER, m_boxNBO);
        glEnableVertexAttribArray(normAttribLoc);
        glVertexAttribPointer(normAttribLoc, 3, GL_FLOAT, GL_TRUE, 0, 0);
    }

    // Set the modelview matrix
    if (mvLoc != -1) {
        glm::mat4 mv;
        //mv = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        //mv = glm::rotate(glm::mat4(1.0f), 30.0f, glm::vec3(0.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(/*location*/mvLoc,
                           /*# of matrices*/1,
                           /*transpose?*/GL_FALSE,
                           /*The matrix pointer*/ glm::value_ptr(mv));
        if (nmLoc != -1) {
            // The normal matrix is the 3x3 (rotational part) of the inverse, transposed mv matrix
            glUniformMatrix3fv(nmLoc, 1, GL_TRUE, glm::value_ptr(glm::mat3(glm::inverse(mv))));
        }
    }

    // Bind the texture array attribute pointer
    if (texAttribLoc != -1) {
        glEnableVertexAttribArray(texAttribLoc);
        glBindBuffer(GL_ARRAY_BUFFER, m_boxTCBO);
        glVertexAttribPointer(texAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    if (matLoc != -1) {
        glUniform3fv(glGetUniformLocation(id, "u_Material.Ka"), 1,
                     glm::value_ptr(glm::vec3(0.2,0.2,0.2)));
        glUniform3fv(glGetUniformLocation(id, "u_Material.Kd"), 1,
                     glm::value_ptr(glm::vec3(1.0,1.0,1.0)));
        glUniform3fv(glGetUniformLocation(id, "u_Material.Ks"), 1,
                     glm::value_ptr(glm::vec3(1.0,1.0,1.0)));
        glUniform1f(glGetUniformLocation(id, "u_Material.Shininess"), 4.0f);
    }

    // Activate our texture 0
    if (texLoc != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texID);
        glUniform1i(texLoc, 0); // The value is zero - the active texture
    }

    // Enable element array and draw
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_boxEABO);
    glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    // Disable the arrays where necessary
    glDisableVertexAttribArray(vertAttribLoc);
    if (texAttribLoc != -1) glDisableVertexAttribArray(texAttribLoc);
    if (texLoc != -1) glBindTexture(GL_TEXTURE_2D, 0);
}

void TestScene::init() {
    // Create the vertex array which will store all the box array attributes
    glGenVertexArrays(1, &m_boxVA);
    glBindVertexArray(m_boxVA);

    // Create the box with the checkerboard texture
    static const GLfloat boxVerts[20*3] = {-1.0f, -1.0f, -2.0f,
                                           1.0f, -1.0f, -2.0f,
                                           1.0f, 1.0f, -2.0f,
                                           -1.0f, 1.0f, -2.0f,

                                           -1.0f, -1.0f, 0.0f,
                                           1.0f, -1.0f, 0.0f,
                                           1.0f, -1.0f, -2.0f,
                                           -1.0f, -1.0f, -2.0f,

                                           1.0f, -1.0f, 0.0f,
                                           1.0f, 1.0f, 0.0f,
                                           1.0f, 1.0f, -2.0f,
                                           1.0f, -1.0f, -2.0f,

                                           1.0f, 1.0f, 0.0f,
                                           -1.0f, 1.0f, 0.0f,
                                           -1.0f, 1.0f, -2.0f,
                                           1.0f, 1.0f, -2.0f,

                                           -1.0f, 1.0f, 0.0f,
                                           -1.0f, -1.0f, 0.0f,
                                           -1.0f, -1.0f, -2.0f,
                                           -1.0f, 1.0f, -2.0f};
    glGenBuffers(1, &m_boxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_boxVBO);
    glBufferData(GL_ARRAY_BUFFER, 3*20*sizeof(GLfloat), boxVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CheckError("Scene::init(vertices)");

    // Create normals
    static const GLfloat boxNormals[20*3] = {0.0f, 0.0f, 1.0f,
                                             0.0f, 0.0f, 1.0f,
                                             0.0f, 0.0f, 1.0f,
                                             0.0f, 0.0f, 1.0f,
                                             0.0f, 1.0f, 0.0f,
                                             0.0f, 1.0f, 0.0f,
                                             0.0f, 1.0f, 0.0f,
                                             0.0f, 1.0f, 0.0f,
                                             -1.0f, 0.0f, 0.0f,
                                             -1.0f, 0.0f, 0.0f,
                                             -1.0f, 0.0f, 0.0f,
                                             -1.0f, 0.0f, 0.0f,
                                             0.0f, -1.0f, 0.0f,
                                             0.0f, -1.0f, 0.0f,
                                             0.0f, -1.0f, 0.0f,
                                             0.0f, -1.0f, 0.0f,
                                             1.0f, 0.0f, 0.0f,
                                             1.0f, 0.0f, 0.0f,
                                             1.0f, 0.0f, 0.0f,
                                             1.0f, 0.0f, 0.0f};
    glGenBuffers(1, &m_boxNBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_boxNBO);
    glBufferData(GL_ARRAY_BUFFER, 3*20*sizeof(GLfloat), boxNormals, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CheckError("Scene::init(normals)");

    // Create the texture coordinates
    static const GLfloat boxTex[2*20] = {0.0f, 0.0f,
                                         6.0f, 0.0f,
                                         6.0f, 6.0f,
                                         0.0f, 6.0f,
                                         0.0f, 0.0f,
                                         6.0f, 0.0f,
                                         6.0f, 6.0f,
                                         0.0f, 6.0f,
                                         0.0f, 0.0f,
                                         6.0f, 0.0f,
                                         6.0f, 6.0f,
                                         0.0f, 6.0f,
                                         0.0f, 0.0f,
                                         6.0f, 0.0f,
                                         6.0f, 6.0f,
                                         0.0f, 6.0f,
                                         0.0f, 0.0f,
                                         6.0f, 0.0f,
                                         6.0f, 6.0f,
                                         0.0f, 6.0f};
    glGenBuffers(1, &m_boxTCBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_boxTCBO);
    glBufferData(GL_ARRAY_BUFFER, 2*20*sizeof(GLfloat), boxTex, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CheckError("Scene::init(texCoord)");

    // Construct the triangle elements
    static const GLuint boxTris[30] = {0, 1, 2,
                                       2, 3, 0,
                                       4, 5, 6,
                                       6, 7, 4,
                                       8, 9, 10,
                                       10, 11, 8,
                                       12, 13, 14,
                                       14, 15, 12,
                                       16, 17, 18,
                                       18, 19, 16};
    glGenBuffers(1, &m_boxEABO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_boxEABO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*10*sizeof(GLuint), boxTris, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    CheckError("Scene::init(elements)");

    // Create the texture map
    QImage texImage = QImage("../shared/data/checkerboard.jpg");
    QImage texData = QGLWidget::convertToGLFormat(texImage);
    glGenTextures(1, &m_texID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texID);

    // Set up parameters for our texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

//    // Copy the data across
    glTexImage2D(/*target*/ GL_TEXTURE_2D,
                 /*mipmap level*/ 0,
                 /*internal format*/ GL_RGBA,
                 /*texture width*/ texData.width(),
                 /*texture height*/ texData.height(),
                 /*border width*/ 0,
                 /*external formal*/ GL_RGBA,
                 /*type*/ GL_UNSIGNED_BYTE,
                 /*data*/ texData.bits());
    CheckError("Scene::init(texture)");

    // Create the buddah for rendering
    glGenVertexArrays(1, &m_buddahVA);
    glBindVertexArray(m_buddahVA);
    glGenBuffers(1, &m_buddahBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_buddahBO);
    glBufferData(GL_ARRAY_BUFFER, buddahSIZE*sizeof(GLfloat), buddah, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CheckError("Scene::init(buddah)");

    // Create a dragon for rendering
    glGenVertexArrays(1, &m_dragonVA);
    glBindVertexArray(m_dragonVA);
    glGenBuffers(1, &m_dragonBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_dragonBO);
    glBufferData(GL_ARRAY_BUFFER, dragonSIZE*sizeof(GLfloat), dragon, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CheckError("Scene::init(dragon)");

    // Create a dragon for rendering
    glGenVertexArrays(1, &m_teapotVA);
    glBindVertexArray(m_teapotVA);
    glGenBuffers(1, &m_teapotBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_teapotBO);
    glBufferData(GL_ARRAY_BUFFER, teapotSIZE*sizeof(GLfloat), teapot, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CheckError("Scene::init(teapot)");

    m_init = true;
}
