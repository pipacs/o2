QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

include(../../src/src.pri)

TARGET = youtubedemo
TEMPLATE = app

SOURCES += main.cpp \
    ytdemo.cpp

HEADERS += \
    ytdemo.h
