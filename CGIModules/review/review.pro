
QT       += core sql

QT       -= gui

TARGET = review
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../../sources/Classes/removetagssql.cpp

HEADERS += \
    ../../sources/Classes/removetagssql.h
