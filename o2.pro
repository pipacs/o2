# Install prefix
mac: PREFIX=/Users/hans/devel/libraries/osx
win32: PREFIX=c:/devel/libraries/win64
linux: PREFIX=/home/hans/devel/libraries/linux64

# QT6 Support
equals(QT_MAJOR_VERSION, 6) {
win32: TARGET = o2_qt6
mac: TARGET = o2_qt6
}

# O2 stuff
QT -= gui

TEMPLATE = lib
DEFINES += O2_SHARED_LIB
DEFINES += O2_DLL_EXPORT

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

mac: QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8

PREFIX=$$PREFIX
SRC_PREFIX=$$SRC_PREFIX

LIB_TARGET=lib
CONFIG(debug, release|debug) {
LIB_TARGET=libd
}

target.path = $$PREFIX/$$LIB_TARGET

INSTALLS += target

SOURCES +=

HEADERS +=

#### Headers to install
headers.path = $$PREFIX/include/o2

include(src/src.pri)

