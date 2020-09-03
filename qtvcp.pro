QT += core
QT -= gui

CONFIG += c++11

TARGET = qtvcp
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    emcmodule.cpp \
    hal_glib.cpp \
    slot_test.cpp \
    qt_istat.cpp \
    qt_action.cpp \
    widgets/file_manager.cpp \
    widgets/gcode_editor.cpp \
    widgets/mdi_line.cpp \
    widgets/origin_offsetview.cpp \
    widgets/simple_widgets.cpp \
    widgets/virtualkeyboardwidget.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    emcmodule.h \
    hal_glib.h \
    slot_test.h \
    qt_istat.h \
    core.h \
    qt_action.h \
    widgets/file_manager.h \
    widgets/gcode_editor.h \
    widgets/mdi_line.h \
    widgets/origin_offsetview.h \
    widgets/simple_widgets.h \
    widgets/virtualkeyboardwidget.h \
    logs.h

unix:!macx: LIBS += -L$$PWD/../linuxcnc/lib/ -llinuxcnc

INCLUDEPATH += $$PWD/../linuxcnc/include
DEPENDPATH += $$PWD/../linuxcnc/include

unix:!macx: PRE_TARGETDEPS += $$PWD/../linuxcnc/lib/liblinuxcnc.a
unix:!macx: LIBS += -L$$PWD/../linuxcnc/lib/ -llinuxcncini
unix:!macx: LIBS += -L$$PWD/../linuxcnc/lib/ -lnml
unix:!macx: LIBS += -L$$PWD/../linuxcnc/lib/ -llinuxcnchal

unix:!macx: LIBS += -L$$PWD/../Qsci/ -lqscintilla2_qt5

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..

RESOURCES += \
    sources.qrc

DEFINES += BOOST_ALL_DYN_LINK

unix:!macx: LIBS += -L$$PWD/../../../../opt/boost_1_67/lib/ -lboost_system
unix:!macx: LIBS += -L$$PWD/../../../../opt/boost_1_67/lib/ -lboost_log
unix:!macx: LIBS += -L$$PWD/../../../../opt/boost_1_67/lib/ -lboost_filesystem
unix:!macx: LIBS += -L$$PWD/../../../../opt/boost_1_67/lib/ -lboost_thread
