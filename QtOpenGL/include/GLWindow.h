#ifndef GLWINDOW_H__
#define GLWINDOW_H__

#include <QOpenGLWindow>
#include <vector>

struct Vec3
{
  float x;
  float y;
  float z;
  Vec3(float _x, float _y, float _z)
  {
    x=_x; y=_y; z=_z;
  }
  Vec3(){}
};

struct Vec3C3
{
  Vec3 p;
  Vec3 c;
};



class GLWindow : public QOpenGLWindow
{
public :
    GLWindow();
    ~GLWindow();
    void initializeGL();
    void resizeGL(int _w, int _h);
    void paintGL();
private :
    float m_y;
    void timerEvent(QTimerEvent *);
    void keyPressEvent(QKeyEvent *_event);
    std::vector <Vec3> m_verts;
    std::vector <Vec3> m_colours;
    std::vector <Vec3C3> m_vc;

};

#endif
