#-------------------------------------------------
#
# Project created by QtCreator 2012-09-27T10:16:42
#
#-------------------------------------------------

QT       += core opengl widgets
CONFIG += debug
TARGET = dof
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
           ../shared/src/shader.cpp \
           ../shared/src/testscene.cpp \
           ../shared/src/scene.cpp \
           ../shared/src/quadscene.cpp \
           ../shared/src/glutil.cpp

HEADERS  += src/mainwindow.h \
            src/glwidget.h \
            ../shared/src/shaderhelper.h \
            ../shared/src/shader.h \
            ../shared/src/glinc.h \
            ../shared/src/testscene.h \
            ../shared/src/scene.h \
            ../shared/src/quadscene.h

DISTFILES +=  \
    shaders/depth.fs  \
    shaders/depth.vs  \
    shaders/basic.Frag \
    shaders/basic.Vert \
    shaders/dof.Frag \
    shaders/dof.Vert
OTHER_FILES +=  \
    README.txt
