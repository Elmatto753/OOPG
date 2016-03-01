#ifndef SCENE_H
#define SCENE_H

#include "glinc.h"
#include <QObject>
#include <QtOpenGL/QGLWidget>

class Scene : public QObject
{
    Q_OBJECT
public:
    /// Constructor
    explicit Scene(QGLWidget *parent = 0);

    /// Virtual destructor
    virtual ~Scene() {}

    /// Draw. Note that attributes and uniforms of the current shader will be queried, so the shader must be bound.
    virtual void draw() {};

    /// Initialise the scene
    virtual void init() {};

signals:
    /// Emit this when you want to redraw the scene
    void sceneDirty();

protected:
    /// Keep track as to whether this object has been initialised
    bool m_init;
};

#endif // SCENE_H
