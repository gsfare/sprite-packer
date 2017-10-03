#-------------------------------------------------
#
# Project created by QtCreator 2014-06-27T09:49:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SpritePacker
TEMPLATE = app


SOURCES += main.cpp \
           mainwindow.cpp \
           mygraphicsview.cpp \
    RectangleBinPack/GuillotineBinPack.cpp \
    RectangleBinPack/MaxRectsBinPack.cpp \
    RectangleBinPack/Rect.cpp \
    RectangleBinPack/ShelfBinPack.cpp \
    RectangleBinPack/ShelfNextFitBinPack.cpp \
    RectangleBinPack/SkylineBinPack.cpp

HEADERS  += mainwindow.h mygraphicsview.h \
    types.h binpack2d.h \
    RectangleBinPack/GuillotineBinPack.h \
    RectangleBinPack/MaxRectsBinPack.h \
    RectangleBinPack/Rect.h \
    RectangleBinPack/ShelfBinPack.h \
    RectangleBinPack/ShelfNextFitBinPack.h \
    RectangleBinPack/SkylineBinPack.h

FORMS    += mainwindow.ui

RESOURCES += \
    Resources.qrc
