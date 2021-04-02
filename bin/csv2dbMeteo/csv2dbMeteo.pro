#-----------------------------------------------------------
#
#   Csv2dbMeteo
#   This project is part of CRITERIA1D distribution
#
#-----------------------------------------------------------

QT += core sql
QT -= gui

CONFIG += console
TEMPLATE = app

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/Csv2dbMeteo
    } else {
        TARGET = release/Csv2dbMeteo
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/Csv2dbMeteo
    } else {
        TARGET = release/Csv2dbMeteo
    }
}
win32:{
    TARGET = Csv2dbMeteo
}


SOURCES += main.cpp
