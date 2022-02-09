#-----------------------------------------------------------
#
#   FrostForecast
#   This project is part of agroTools distribution
#
#-----------------------------------------------------------

QT += core sql xml network
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

INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis \
                ../../agrolib/utilities ../../agrolib/meteo ../../agrolib/interpolation ../../agrolib/dbMeteoGrid ../../agrolib/dbMeteoPoints

CONFIG(debug, debug|release) {

    LIBS += -L../../agrolib/dbMeteoPoints/debug -ldbMeteoPoints
    LIBS += -L../../agrolib/dbMeteoGrid/debug -ldbMeteoGrid
    LIBS += -L../../agrolib/interpolation/debug -linterpolation
    LIBS += -L../../agrolib/meteo/debug -lmeteo
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions

} else {

    LIBS += -L../../agrolib/dbMeteoPoints/release -ldbMeteoPoints
    LIBS += -L../../agrolib/dbMeteoGrid/release -ldbMeteoGrid
    LIBS += -L../../agrolib/interpolation/release -linterpolation
    LIBS += -L../../agrolib/meteo/release -lmeteo
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../agrolib/gis/release -lgis
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
}

SOURCES += main.cpp \
    frost.cpp

HEADERS += \
    frost.h
