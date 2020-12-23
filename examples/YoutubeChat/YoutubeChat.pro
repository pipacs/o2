QT += core gui


include(../../src/src.pri)
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}


CONFIG += c++11


TARGET = YoutubeChat
CONFIG += console

TEMPLATE = app

SOURCES += main.cpp \
    commentmodel.cpp \
    livebroadcastmodel.cpp \
    o2youtube.cpp \
    youtubeapi.cpp \
    youtubecontroller.cpp \
    helper.cpp \
    messagereceiver.cpp

HEADERS += \
    commentmodel.h \
    livebroadcastmodel.h \
    o2youtube.h \
    youtubeapi.h \
    youtubecontroller.h \
    helper.h \
    messagereceiver.h
