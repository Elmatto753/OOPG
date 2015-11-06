#include <QtGui/QGuiApplication>
#include <QSurfaceFormat>

#include <iostream>
#include "GLWindow.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc,argv);

    QSurfaceFormat format;
    format.setMajorVersion(4);
    format.setMinorVersion(5);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);

    QSurfaceFormat::setDefaultFormat(format);

    GLWindow window;
    window.resize(1024,720);
    window.show();

    return app.exec();
}
