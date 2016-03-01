#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class GLWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    /// A couple of helpful values to track the current camera depth
    float m_currentDepth, m_depthInc;

    /// A local pointer to the widget where the magic happens
    GLWidget *m_glwidget;

protected:
    /// Overloaded function to handle keyboard input
    void keyPressEvent(QKeyEvent *e);
};

#endif // MAINWINDOW_H
