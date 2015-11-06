#include <GL/glu.h>
#include "GLWindow.h"
#include <QGuiApplication>
#include <QKeyEvent>

GLWindow::GLWindow()
{
    setTitle("Simple GL Window");
    m_y=0.0f;

    m_verts.push_back(Vec3(0.0f,1.0f,0.0f));
    m_verts.push_back(Vec3(-1.0f,0.0f,0.0f));
    m_verts.push_back(Vec3(1.0f,0.0f,0.0f));

    m_colours.push_back(Vec3(1.0f,0.0f,0.0f));
    m_colours.push_back(Vec3(0.0f,1.0f,0.0f));
    m_colours.push_back(Vec3(0.0f,0.0f,1.0f));

    Vec3C3 p;
    p.p=Vec3(0.0,1.0,-1.0f);
    p.c=Vec3(1.0,0.0,0.0);
    m_vc.push_back(p);

    p.p=Vec3(-1.0,0.0,-1.0f);
    p.c=Vec3(0.0,1.0,0.0);
    m_vc.push_back(p);

    p.p=Vec3(1.0,0.0,-1.0f);
    p.c=Vec3(0.0,0.0,1.0);
    m_vc.push_back(p);



}

GLWindow::~GLWindow()
{

}

void GLWindow::initializeGL()
{
    glClearColor(0.6,0.6,0.6,1.0);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0f,float(width()/height()),0.1,20.0f);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(2,2,2,0,0,0,0,1,0);
    startTimer(10);
}

void GLWindow::resizeGL(int _w, int _h)
{
    glViewport(0,0,_w,_h);
}

void GLWindow::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
      glRotatef(m_y,0.0,1.0,0.0);

      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3,GL_FLOAT,0,&m_verts[0].x);
      glEnableClientState(GL_COLOR_ARRAY);
      glColorPointer(3,GL_FLOAT,0,&m_colours[0].x);
      glDrawArrays(GL_TRIANGLES,0,m_verts.size());

      glVertexPointer(3,GL_FLOAT,3*sizeof(float),&m_vc[0].p.x);
      glColorPointer(3,GL_FLOAT,3*sizeof(float),&m_vc[0].c.x);
      glDrawArrays(GL_TRIANGLES,0,m_vc.size());

    glPopMatrix();

}

void GLWindow::timerEvent(QTimerEvent *)
{
    m_y+=0.1;
    update();
}

void GLWindow::keyPressEvent(QKeyEvent *_event)
{
    switch(_event->key())
    {
        case Qt::Key_Escape : QGuiApplication::exit(0); break;
        case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
        case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
    }
}
