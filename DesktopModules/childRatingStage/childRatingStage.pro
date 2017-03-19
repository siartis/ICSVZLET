
QT       += core gui sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = childRatingStage
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    filehandler.cpp

HEADERS  += mainwindow.h \
    filehandler.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc
