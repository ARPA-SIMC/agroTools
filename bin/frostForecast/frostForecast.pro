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
CONFIG += c++17

win32:{
    QMAKE_CXXFLAGS += -openmp -GL
    QMAKE_LFLAGS   += -LTCG
}
unix:{
    QMAKE_CXXFLAGS += -fopenmp #-flto
    QMAKE_LFLAGS += -fopenmp #-flto
}
macx:{
    QMAKE_CXXFLAGS += -fopenmp #-flto
    QMAKE_LFLAGS += -fopenmp #-flto
}

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
                ../../agrolib/utilities ../../agrolib/meteo ../../agrolib/interpolation \
                ../../agrolib/dbMeteoPoints ../../agrolib/solarRadiation

CONFIG(debug, debug|release) {

    LIBS += -L../../agrolib/solarRadiation/debug -lsolarRadiation
    LIBS += -L../../agrolib/dbMeteoPoints/debug -ldbMeteoPoints
    LIBS += -L../../agrolib/interpolation/debug -linterpolation
    LIBS += -L../../agrolib/meteo/debug -lmeteo
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions

} else {

    LIBS += -L../../agrolib/solarRadiation/release -lsolarRadiation
    LIBS += -L../../agrolib/dbMeteoPoints/release -ldbMeteoPoints
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
