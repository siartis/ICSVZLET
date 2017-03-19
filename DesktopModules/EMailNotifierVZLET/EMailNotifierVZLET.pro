
QT       += core network

QT       -= gui

TARGET = EMailNotifierVZLET
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    emailnotificator.cpp \
    smtp.cpp \
    filehandler.cpp

HEADERS += \
    emailnotificator.h \
    smtp.h \
    filehandler.h
