QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

include(../../src/src.pri)

TARGET = vimeodemo
TEMPLATE = app

SOURCES += main.cpp \
    vimeodemo.cpp

HEADERS += \
    vimeodemo.h
