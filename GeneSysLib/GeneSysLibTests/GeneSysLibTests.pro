TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET="GeneSysLibTests"


SOURCES += \
    Test_Device.cpp \
    main.cpp

macx: LIBS += -lboost_unit_test_framework

DEPENDPATH += $$PWD/../
INCLUDEPATH += $$PWD/../
INCLUDEPATH += $$PWD/../../../../rtmidi-2.0.1
DEPENDPATH += $$PWD/../../../../rtmidi-2.0.1
