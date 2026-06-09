QT       += core gui widgets

TARGET = CNCInterpolation
TEMPLATE = app
CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    canvas.cpp \
    interpolation.cpp

HEADERS += \
    mainwindow.h \
    canvas.h \
    interpolation.h
