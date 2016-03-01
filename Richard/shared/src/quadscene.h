#ifndef QUADSCENE_H
#define QUADSCENE_H

#include "scene.h"

class QuadScene : public Scene
{
    Q_OBJECT
public:
    /// Constructor
    explicit QuadScene(QGLWidget *parent = 0);

    /// Destructor
    ~QuadScene();

    /// Draw. Note that attributes and uniforms of the current shader will be queried, so the shader must be bound.
    virtual void draw();

    /// Initialise the scene
    virtual void init();

signals:
    /// Emit this when you want to redraw the scene
    void sceneDirty();
    
private:
    /// The buffer of the buddah object
    GLuint m_quadVA, m_quadVBO;
};

#endif // QUADSCENE_H
