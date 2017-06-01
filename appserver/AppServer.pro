#-------------------------------------------------
#
# Project created by QtCreator 2012-03-12T14:35:05
#
#-------------------------------------------------

QT       += core
QT       += network
QT       += sql
QT       += widgets

QT       += gui

#TARGET = ../../../appserver5/AppServer8
TARGET = AppServer8
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    urldecode.cpp \
    httpsplint.cpp \
    HttpRead.cpp \
    httpserver.cpp \
    httprequest.cpp \
    api.cpp \
    option.cpp \
    httpthread.cpp \
    httpsession.cpp \
    TB.cpp \
    httpadmin.cpp \
    Authorization.cpp

HEADERS += \
    ../INCLUDE/table.h \
    ../INCLUDE/parser.h \
    httpserver.h \
    httprequest.h \
    cookie.h \
    option.h \
    httpthread.h \
    httpsession.h \
    TB.h \
    httpadmin.h

RESOURCES +=

OTHER_FILES += \
    7.txt \
    88.txt \
    55.txt \
    fff.js \
    777.qml \
    777.js

INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib

#QTPLUGIN +=qsqlodbc
#QTPLUGIN +=c:/qt/plugins/sqldrivers/libqsqlodbcd4

#QTPLUGIN +=libqsqlodbcd4
#LIBS+= c:/qt/plugins/sqldrivers/libqsqlodbcd4d.a



