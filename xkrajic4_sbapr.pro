#-------------------------------------------------
#
# Project created by QtCreator 2014-04-11T21:13:17
#
#-------------------------------------------------

QT += core gui opengl testlib concurrent network

LIBS += -lglut

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = MotionCapture
TEMPLATE = app

CONFIG += link_pkgconfig

PKGCONFIG += opencv gl glu

UI_DIR +=  Gui
UI_HEADERS_DIR += Gui

SOURCES += main.cpp\
    capturecamera.cpp \
    line.cpp \
    markerpoint.cpp \
    modelstructure.cpp \
    openglwindow.cpp \
    animation.cpp \
    frame.cpp \
    Gui/structureeditor.cpp \
    Gui/addproject.cpp \
    Gui/mainwindow.cpp \
    Gui/animplayer.cpp \
    room.cpp \
    capturethread.cpp \
    Gui/camwidget.cpp \
    Gui/addcamera.cpp \
    pointchecker.cpp

HEADERS  += capturecamera.h \
    line.h \
    markerpoint.h \
    modelstructure.h \
    openglwindow.h \
    animation.h \
    frame.h \
    Gui/structureeditor.h \
    Gui/addproject.h \
    Gui/mainwindow.h \
    Gui/animplayer.h \
    room.h \
    capturethread.h \
    Gui/camwidget.h \
    Gui/addcamera.h \
    pointchecker.h

FORMS    += Gui/structureeditor.ui \
    Gui/mainwindow.ui \
    Gui/animplayer.ui \
    Gui/camwidget.ui \
    Gui/addcamera.ui \
    Gui/addproject.ui

QMAKE_CXXFLAGS += -std=c++11 -pedantic -Wall -Wextra

OTHER_FILES += \
    Pictures/main_icon.jpg \
    Pictures/PlayIcon.png \
    Pictures/EditIcon.png \
    Pictures/SaveIcon.png
