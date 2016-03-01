#include "scene.h"

/**
  * Create the scene and rig up the sceneDirty signal.
  */
Scene::Scene(QGLWidget *parent) : QObject(parent), m_init(false) {
    // Trigger a GL repaint when we need to
    if (parent != NULL)
        connect(this, SIGNAL(sceneDirty()), parent, SLOT(updateGL()));
}
