QT       += core gui webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

DEFINES += O0_EXPORT=
include(../../src/src.pri)

TARGET = msgraphdemo
TEMPLATE = app

SOURCES += main.cpp \
    msgraphdemo.cpp \
    webenginepage.cpp \
    webwindow.cpp

HEADERS += \
    msgraphdemo.h \
    webenginepage.h \
    webwindow.h

FORMS += webwindow.ui
