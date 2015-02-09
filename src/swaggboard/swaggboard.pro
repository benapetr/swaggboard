#-------------------------------------------------
#
# Project created by QtCreator 2015-02-08T14:38:24
#
#-------------------------------------------------

QT       += core gui xml
lessThan(QT_MAJOR_VERSION, 5): QT += phonon
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

TARGET = swaggboard
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    shortcuthelper.cpp \
    musicfinder.cpp \
    information.cpp \
    options.cpp

HEADERS  += mainwindow.hpp \
    shortcuthelper.hpp \
    musicfinder.hpp \
    information.hpp \
    options.hpp

FORMS    += mainwindow.ui \
    musicfinder.ui \
    information.ui \
    options.ui

LIBS += -ldsound -lole32
