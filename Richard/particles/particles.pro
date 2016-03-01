######################################################################
# Automatically generated by qmake (2.01a) Fri Oct 5 15:21:25 2012
######################################################################


QT       += core gui opengl
CONFIG += debug
TARGET = bin/particles
TEMPLATE = app
INCLUDEPATH += $(GLM_PATH) $(NGL)/src ../shared/src
OBJECTS_DIR = obj
MOC_DIR = moc
#UI_DIR = ui
LIBS += -lGLEW -lGLU
win32:LIBS -= -lGLEW
win32:LIBS += -lglew32 -L$(GLEW_PATH)\\bin

SOURCES += src/main.cpp\
           src/mainwindow.cpp \
           src/glwidget.cpp \
           src/particlesystem.cpp \
           ../shared/src/shader.cpp \
           ../shared/src/glutil.cpp

HEADERS  += src/mainwindow.h \
            src/glwidget.h \
            src/particlesystem.h \
            ../shared/src/shaderhelper.h \
            ../shared/src/shader.h \
            ../shared/src/glinc.h \
            ../shared/src/scene.h \
            ../shared/src/glutil.h

DISTFILES += shaders/particle.fs \
             shaders/particle.vs \
             data/sprite.jpg

OTHER_FILES += shaders/particle.fs \
               shaders/particle.vs \
               data/sprite.jpg \
               README.txt


# Directories
