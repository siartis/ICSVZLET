
QT       += core sql

QT       -= gui

TARGET = file
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../../sources/Classes/removetagssql.cpp

HEADERS += \
    ../../sources/Classes/removetagssql.h
