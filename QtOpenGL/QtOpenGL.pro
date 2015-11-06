TARGET=QtOpenGL
CONFIG+=c++11

OBJECTS_DIR=obj
MOC_DIR=moc
QT+= opengl core gui
LIBS+=-lGLU
INCLUDEPATH+=./include

SOURCES+=$$PWD/src/main.cpp \
         $$PWD/src/GLWindow.cpp

HEADERS+=$$PWD/include/GLWindow.h
