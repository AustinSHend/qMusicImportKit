#-------------------------------------------------
#
# Project created by QtCreator 2019-07-29T09:54:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qMusicImportKit
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += \
       c++11 \
       file_copies


SOURCES += \
        aboutwindow.cpp \
        helper.cpp \
        main.cpp \
        mainwindow.cpp \
        settingswindow.cpp

HEADERS += \
        aboutwindow.h \
        helper.h \
        mainwindow.h \
        settingswindow.h

FORMS += \
        aboutwindow.ui \
        mainwindow.ui \
        settingswindow.ui

# Split intermediate files
OBJECTS_DIR=generated_files
MOC_DIR=generated_files
RCC_DIR=generated_files

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Linux include libraries
unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += taglib

# Windows include libraries
win32: LIBS += -L'C:/Program Files (x86)/taglib/lib/' -ltag
win32: INCLUDEPATH += 'C:/Program Files (x86)/taglib/include/taglib'
win32: DEPENDPATH += 'C:/Program Files (x86)/taglib/include/taglib'

RESOURCES += \
    data.qrc
