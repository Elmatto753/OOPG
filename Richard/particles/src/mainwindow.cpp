#include "mainwindow.h"

/**
  * Create a simple mainwindow with the particle system glwidget.
  */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    // Ensure that we are using the latest GL core profile
    QGLFormat format;
    format.setProfile(QGLFormat::CoreProfile);

    // Create our GL widget
    m_glwidget = new GLWidget(format, this);

    // Display our widget as the central feature and show it
    setCentralWidget(m_glwidget);
    m_glwidget->show();
    resize(800,600);
}
