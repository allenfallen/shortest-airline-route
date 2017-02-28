QT       += core gui network xml

TARGET = finalData
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    airlinepath.cpp

HEADERS += \
    vertex.h \
    route.h \
    airlinepath.h \
    airline.h
