#include "mainwindow.h"
#include "glwidget.h"
#include <QKeyEvent>

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

    // Default depth parameters
    m_currentDepth = m_glwidget->getDepth();
    m_depthInc = 0.1;

    resize(800,600);
}

MainWindow::~MainWindow() {
}

void MainWindow::keyPressEvent(QKeyEvent *e) {
    switch(e->key()) {
    case Qt::Key_Plus:
        if (m_currentDepth < 1.0f) {
            m_currentDepth += m_depthInc;
            m_glwidget->setDepth(m_currentDepth);
        }
        break;
    case Qt::Key_Minus:
        if (m_currentDepth > 0.2f) {
            m_currentDepth -= m_depthInc;
            m_glwidget->setDepth(m_currentDepth);
        }
        break;
    default:
        break;
    }
}
