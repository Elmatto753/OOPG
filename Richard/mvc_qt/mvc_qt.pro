######################################################################
# Automatically generated by qmake (2.01a) Fri Oct 5 15:21:25 2012
######################################################################


QT       += core widgets opengl
CONFIG += debug
TARGET = mvc
TEMPLATE = app
INCLUDEPATH += $(GLM_PATH) ./src ./package ../shared/src
OBJECTS_DIR = obj
MOC_DIR = moc
QMAKE_CXXFLAGS += -std=c++0x
#UI_DIR = ui
LIBS += -lGLEW -lGLU -lglut -lGL 
win32:LIBS -= -lGLEW
win32:LIBS += -lglew32 -L$(GLEW_PATH)\\bin

SOURCES += src/main.cpp\
           src/mainwindow.cpp \
           src/glwidget.cpp \
           ../shared/src/shader.cpp \
           ../shared/src/glutil.cpp \
	    src/mvcdeform.cpp \
    	src/mvc.cpp \
	    package/SOIL.c \
	    package/stb_image_aug.c \
	    package/image_helper.c \
	    package/image_DXT.c
	
HEADERS  += src/mainwindow.h \
            src/glwidget.h \
            ../shared/src/shaderhelper.h \
            ../shared/src/shader.h \
            ../shared/src/glinc.h \
            ../shared/src/scene.h \
            ../shared/src/glutil.h \
	    src/mvcdeform.h \    		
	    src/mvc.h \
	    package/SOIL.h \
	    package/stb_image_aug.h \
	    package/stbi_DDS_aug_c.h \
	    package/stbi_DDS_aug.h \
	    package/image_helper.h \
       	package/image_DXT.h

OTHER_FILES += shaders/basic.vs shaders/basic.fs \
            shaders/mvc.vs shaders/mvc.fs
DISTFILES += shaders/basic.vs shaders/basic.fs \
            shaders/mvc.vs shaders/mvc.fs \
            data/jmacey_cage.node  data/jmacey.ele \
            data/jmacey.jpg  data/jmacey.node
