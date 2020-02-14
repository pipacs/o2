QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

include(../../src/src.pri)

TARGET = msgraphdemo
TEMPLATE = app

SOURCES += main.cpp \
    msgraphdemo.cpp

HEADERS += \
    msgraphdemo.h
