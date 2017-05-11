#-------------------------------------------------
#
# Project created by QtCreator 2017-04-23T08:57:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = grab_desktop_and_microphone_3
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    desktopcapture.cpp \
    formatdialog.cpp \
    miccapture.cpp \
    outcapture.cpp \
    utils.cpp \
    cameracaptrue.cpp \
    videowindow.cpp \
    camerathread.cpp \
    encoderthread.cpp \
    micthread.cpp

HEADERS  += mainwindow.h \
    desktopcapture.h \
    formatdialog.h \
    miccapture.h \
    outcapture.h \
    type.h \
    utils.h \
    cameracaptrue.h \
    videowindow.h \
    camerathread.h \
    encoderthread.h \
    micthread.h

#sdl2.0
win32: INCLUDEPATH += $$PWD\sdl2.0\include-win32
win32: LIBS += -L$$PWD\sdl2.0\lib-win32 -lSDL2 -lSDL2main
unix:!macx: LIBS     += -lSDL2

#ffmpeg
DEFINES += __STDC_CONSTANT_MACROS
win32: INCLUDEPATH += $$PWD\ffmpeg\include-win32
win32: LIBS += -L$$PWD/ffmpeg/lib-win32/ -lavcodec -lavfilter -lavformat -lavutil -lswresample -lswscale -lavdevice
unix:!macx: LIBS += -L /usr/local/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswscale -lswresample -lSDL2



RESOURCES += \
    app.qrc

DISTFILES += \
    readme \
    app.rc

RC_ICONS = c32X32.ico



