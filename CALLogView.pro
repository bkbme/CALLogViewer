#-------------------------------------------------
#
# Project created by QtCreator 2012-05-23T18:48:56
#
#-------------------------------------------------

QT       += core gui network webkit

include(qtextserial/qextserialport.pri)
INCLUDEPATH += qtextserial

TARGET = CALLogView
TEMPLATE = app


SOURCES += main.cpp\
	mainwindow.cpp \
    callogview.cpp \
    logmessage.cpp \
    syslogloader.cpp \
    servicemanager.cpp \
    loganalyzer.cpp \
	femtectester.cpp \
    femtectesterdialog.cpp

HEADERS  += mainwindow.h \
    callogview.h \
    logmessage.h \
    syslogloader.h \
    servicemanager.h \
    loganalyzer.h \
	femtectester.h \
    femtectesterdialog.h

FORMS    += mainwindow.ui \
    femtectesterdialog.ui

RESOURCES += \
    icons.qrc
