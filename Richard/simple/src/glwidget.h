#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "glinc.h"
#include "shader.h"
#include "scene.h"
#include "mytestscene.h"
#include <QtOpenGL/QGLWidget>

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(const QGLFormat& format, QWidget *parent = 0);
    
protected:
    void initializeGL();        //< Standard function for initialising the GL window
    void resizeGL(int w, int h);//< Standard function for resizing the GL window
    void paintGL();             //< Standard paint function (called a lot)

signals:
    
public slots:

private:
    void _resize(int , int );   //< Hidden function to resize (called from initializeGL and resizeGL)
    MyTestScene  m_scene;         //< A local container of our scene object

    /// Our three shaders
    Shader m_shader;
};

#endif // GLWIDGET_H
