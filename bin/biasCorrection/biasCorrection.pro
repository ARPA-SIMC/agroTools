#-----------------------------------------------------------
#
#   BiasCorrection
#   This project is part of agroTools distribution
#
#-----------------------------------------------------------

QT += core sql xml network
QT -= gui

CONFIG += console
TEMPLATE = app

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/BiasCorrection
    } else {
        TARGET = release/BiasCorrection
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/BiasCorrection
    } else {
        TARGET = release/BiasCorrection
    }
}
win32:{
    TARGET = BiasCorrection
}

INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis \
                ../../agrolib/utilities ../../agrolib/meteo ../../agrolib/dbMeteoGrid

CONFIG(debug, debug|release) {

    LIBS += -L../../agrolib/dbMeteoGrid/debug -ldbMeteoGrid
    LIBS += -L../../agrolib/meteo/debug -lmeteo
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions

} else {

    LIBS += -L../../agrolib/dbMeteoGrid/release -ldbMeteoGrid
    LIBS += -L../../agrolib/meteo/release -lmeteo
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../agrolib/gis/release -lgis
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
}

SOURCES += main.cpp \
    bias.cpp

HEADERS += \
    bias.h
