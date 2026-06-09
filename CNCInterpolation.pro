QT       += core gui widgets

TARGET = CNCInterpolation
TEMPLATE = app
CONFIG += c++17
DEFINES += _USE_MATH_DEFINES

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    canvas.cpp \
    interpolation.cpp

HEADERS += \
    mainwindow.h \
    canvas.h \
    interpolation.h
