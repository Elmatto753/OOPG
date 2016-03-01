#ifndef GLWIDGET_H
#define GLWIDGET_H

#define GLM_FORCE_RADIANS

#include "glinc.h"
#include "shader.h"
#include "testscene.h"
#include "quadscene.h"


#include <QtOpenGL/QGLWidget>

#define FBO_WIDTH 1024
#define FBO_HEIGHT 1024

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    /// Constructor - does not initialise the GL
    explicit GLWidget(const QGLFormat& format, QWidget *parent = 0);

    /// Destructor
    virtual ~GLWidget();
    
    /// Accessor functions
    GLfloat getDepth() {return m_depth;}

protected:
    void initializeGL();        //< Standard function for initialising the GL window
    void resizeGL(int w, int h);//< Standard function for resizing the GL window
    void paintGL();             //< Standard paint function (called a lot)

signals:
    
public slots:
    /// Set the depth at which to focus
    void setDepth(const GLfloat _d);

private:
    GLfloat m_depth;            //< The depth of the
    void _resize(int , int );   //< Hidden function to resize (called from initializeGL and resizeGL)
    Shader m_basicProgram;      //< Shader program for the basic (Gauraud) render pass
    Shader m_dofProgram;        //< Shader program for the depth of field render pass
    TestScene  m_scene;         //< A local container of our scene object
    QuadScene m_quadScene;      //< A simple quad for painting our texture
    GLfloat m_cx, m_cy;         //< The center of the loaded data
    GLfloat m_lx, m_ly;         //< The extents of the data which is loaded in each direction

    /// Id's used for the render buffer object
    GLuint m_fboId, m_rboId, m_fboTextureId, m_fboDepthId, m_fboNormalId;
    
};

#endif // GLWIDGET_H
