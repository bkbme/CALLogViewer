#-------------------------------------------------
#
# Project created by QtCreator 2012-05-23T18:48:56
#
#-------------------------------------------------

QT       += core gui network webkit

TARGET = CALLogView
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    callogview.cpp \
    logmessage.cpp \
    syslogloader.cpp \
    servicemanager.cpp

HEADERS  += mainwindow.h \
    callogview.h \
    logmessage.h \
    syslogloader.h \
    servicemanager.h

FORMS    += mainwindow.ui

RESOURCES += \
    LogViewResources.qrc
