#-----------------------------------------------------------
#
#   FrostForecast
#   This project is part of agroTools distribution
#
#-----------------------------------------------------------

QT += core sql
QT -= gui

CONFIG += console
TEMPLATE = app

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/FrostForecast
    } else {
        TARGET = release/FrostForecast
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/FrostForecast
    } else {
        TARGET = release/FrostForecast
    }
}
win32:{
    TARGET = FrostForecast
}


SOURCES += main.cpp
