#-------------------------------------------------
#
# Project created by QtCreator 2020-07-29T10:34:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtvcp_test
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    emcmodule.cpp \
    hal_glib.cpp \
    qt_action.cpp \
    qt_istat.cpp \
    widgets/mdi_line.cpp \
    widgets/gcode_editor.cpp \
    widgets/virtualkeyboardwidget.cpp \
    widgets/file_manager.cpp \
    widgets/simple_widgets.cpp \
    widgets/origin_offsetview.cpp

HEADERS += \
        mainwindow.h \
    emcmodule.h \
    hal_glib.h \
    qt_action.h \
    qt_istat.h \
    widgets/mdi_line.h \
    widgets/gcode_editor.h \
    widgets/virtualkeyboardwidget.h \
    widgets/file_manager.h \
    widgets/simple_widgets.h \
    widgets/origin_offsetview.h \
    logs.h

FORMS += \
        mainwindow.ui

unix:!macx: LIBS += -L$$PWD/../Qsci/ -lqscintilla2_qt5

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..

RESOURCES += \
    sources.qrc

unix:!macx: LIBS += -L$$PWD/lib/ -llinuxcnc

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

unix:!macx: PRE_TARGETDEPS += $$PWD/lib/liblinuxcnc.a
unix:!macx: LIBS += -L$$PWD/lib/ -llinuxcnchal
unix:!macx: LIBS += -L$$PWD/lib/ -llinuxcncini
unix:!macx: LIBS += -L$$PWD/lib/ -lnml

DEFINES += BOOST_ALL_DYN_LINK

unix:!macx: LIBS += -L$$PWD/../../../../opt/boost_1_67/lib/ -lboost_system
unix:!macx: LIBS += -L$$PWD/../../../../opt/boost_1_67/lib/ -lboost_log
unix:!macx: LIBS += -L$$PWD/../../../../opt/boost_1_67/lib/ -lboost_filesystem
unix:!macx: LIBS += -L$$PWD/../../../../opt/boost_1_67/lib/ -lboost_thread

