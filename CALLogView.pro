#-------------------------------------------------
#
# Project created by QtCreator 2012-05-23T18:48:56
#
#-------------------------------------------------

QT += core gui network webkit

include(qtextserial/qextserialport.pri)
INCLUDEPATH += qtextserial test log misc

TARGET = CALLogView
TEMPLATE = app


SOURCES += \
	main.cpp\
	mainwindow.cpp \
	servicemanager.cpp \
	log/callogview.cpp \
	log/logmessage.cpp \
	log/syslogparser.cpp \
	test/femtectester.cpp \
	log/calsession.cpp \
	log/calsessionmodel.cpp \
	log/calsessionview.cpp \
	misc/animatedsplitter.cpp \
	log/supportinfoopendialog.cpp \
	misc/settingsdialog.cpp \
	test/testersettingspage.cpp \
	misc/searchwidget.cpp \
	misc/calstatuswidget.cpp \
	misc/aboutdialog.cpp \
	log/logsettingspage.cpp \
	test/femtotester.cpp \
	test/procedurefootswitch.cpp \
	test/abstractmessage.cpp \
	test/ackmessage.cpp \
	test/errormessage.cpp \
	test/initmessage.cpp \
	test/versionmessage.cpp \
	test/footswitchmessage.cpp \
	test/messageparser.cpp \
	test/testerstatuswidget.cpp \
	test/dockingforcemessage.cpp \
	test/servoctrlmessage.cpp \
	test/dockinglimitmessage.cpp \
	test/docksettingspage.cpp \
	test/autodock.cpp \
    test/dockingtaremessage.cpp \
    test/dockingstatemessage.cpp

HEADERS  += \
	mainwindow.h \
	servicemanager.h \
	log/callogview.h \
	log/logmessage.h \
	log/syslogparser.h \
	test/femtectester.h \
	log/calsession.h \
	log/calsessionmodel.h \
	log/calsessionview.h \
	misc/animatedsplitter.h \
	log/supportinfoopendialog.h \
	config.h \
	misc/settingsdialog.h \
	misc/abstractsettingspage.h \
	test/testersettingspage.h \
	misc/searchwidget.h \
	misc/calstatuswidget.h \
	misc/aboutdialog.h \
	log/logsettingspage.h \
	test/femtotester.h \
	test/procedurefootswitch.h \
	test/abstractmessage.h \
	test/ackmessage.h \
	test/errormessage.h \
	test/initmessage.h \
	test/versionmessage.h \
	test/footswitchmessage.h \
	test/messageparser.h \
	test/testerstatuswidget.h \
	test/dockingforcemessage.h \
	test/servoctrlmessage.h \
	test/dockinglimitmessage.h \
	test/docksettingspage.h \
	test/autodock.h \
    test/dockingtaremessage.h \
    test/dockingstatemessage.h

FORMS += \
	mainwindow.ui \
	log/supportinfoopendialog.ui \
	misc/settingsdialog.ui \
	test/testersettingspage.ui \
	misc/searchwidget.ui \
	misc/aboutdialog.ui \
	log/logsettingspage.ui \
	test/docksettingspage.ui

RESOURCES += \
	icons.qrc
