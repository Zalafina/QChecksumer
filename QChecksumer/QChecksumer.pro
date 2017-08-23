#-------------------------------------------------
#
# Project created by QtCreator 2017-08-17T11:08:26
#
#-------------------------------------------------

QT += core gui
QT += concurrent widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QChecksumer
TEMPLATE = app
RC_ICONS = image/my_icon.ico

CONFIG(debug, debug|release){
    DEFINES += DEBUG_LOGOUT_ON
    message("Debug Build")
}

CONFIG(release, debug|release){
    message("Release Build")
}
#DEFINES += DEBUG_LOGOUT_ON

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32 {
    message("Win32 Platform")
#    CONFIG += static

#    QMAKE_CFLAGS_RELEASE    -= -MD
#    QMAKE_CFLAGS_DEBUG      -= -MDd
#    QMAKE_CXXFLAGS_RELEASE  -= -MD
#    QMAKE_CXXFLAGS_DEBUG    -= -MDd

#    QMAKE_CFLAGS_RELEASE    += -MT
#    QMAKE_CFLAGS_DEBUG      += -MTd
#    QMAKE_CXXFLAGS_RELEASE  += -MT
#    QMAKE_CXXFLAGS_DEBUG    += -MTd
}


SOURCES     += main.cpp\
    checksumermain.cpp \
    checksumer.cpp

HEADERS     += checksumermain.h \
    checksumer.h

FORMS       += checksumermain.ui

RESOURCES   += \
    image.qrc
