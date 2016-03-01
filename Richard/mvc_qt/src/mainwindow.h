#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "glwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    
signals:
    
public slots:

protected:
    GLWidget *m_glwidget;

protected:
    /// Overloaded function to handle keyboard input
    void keyPressEvent(QKeyEvent *) {}
    
};

#endif // MAINWINDOW_H
