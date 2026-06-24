#-----------------------------------------------------------
#
#   CriteriaOutput Tool
#   Post-processing of CRITERIA-1D output
#
#   This project is part of CRITERIA-1D distribution
#
#-----------------------------------------------------------

QT   -= gui
QT   += sql

CONFIG += console
CONFIG += c++17
TEMPLATE = app

VERSION = 2.0.2
QMAKE_TARGET_COPYRIGHT = "\\251 2026 ARPAE ER - Climate Observatory"

unix:{
    CONFIG(debug, debug|release) {
        TARGET = debug/CriteriaOutput
    } else {
        TARGET = release/CriteriaOutput
    }
}
macx:{
    CONFIG(debug, debug|release) {
        TARGET = debug/CriteriaOutput
    } else {
        TARGET = release/CriteriaOutput
    }
}
win32:{
    TARGET = CriteriaOutput
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += _CRT_SECURE_NO_WARNINGS


SOURCES += \
    main.cpp


INCLUDEPATH +=  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis ../../agrolib/crop  \
                ../../agrolib/utilities ../../agrolib/shapeUtilities  \
                ../../agrolib/shapeHandler ../../agrolib/netcdfHandler \
                ../../agrolib/shapeHandler/shapelib ../../agrolib/criteriaOutput

# comment to compile without GDAL library
CONFIG += GDAL

GDAL:{
    DEFINES += USE_GDAL
    INCLUDEPATH += ../../agrolib/gdalHandler
    include(../../agrolib/gdal.pri)
}

CONFIG(debug, debug|release) {

    LIBS += -L../../agrolib/criteriaOutput/debug -lcriteriaOutput
    LIBS += -L../../agrolib/shapeUtilities/debug -lshapeUtilities
    GDAL:{
        LIBS += -L../../agrolib/gdalHandler/debug -lgdalHandler
    }
    LIBS += -L../../agrolib/shapeHandler/debug -lshapeHandler
    LIBS += -L../../agrolib/utilities/debug -lutilities
    LIBS += -L../../agrolib/gis/debug -lgis
    LIBS += -L../../agrolib/crit3dDate/debug -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/debug -lmathFunctions

} else {

    LIBS += -L../../agrolib/criteriaOutput/release -lcriteriaOutput
    LIBS += -L../../agrolib/netcdfHandler/release -lnetcdfHandler
    LIBS += -L../../agrolib/shapeUtilities/release -lshapeUtilities
    GDAL:{
        LIBS += -L../../agrolib/gdalHandler/release -lgdalHandler
    }
    LIBS += -L../../agrolib/shapeHandler/release -lshapeHandler
    LIBS += -L../../agrolib/utilities/release -lutilities
    LIBS += -L../../agrolib/gis/release -lgis
    LIBS += -L../../agrolib/crit3dDate/release -lcrit3dDate
    LIBS += -L../../agrolib/mathFunctions/release -lmathFunctions
}


win32:{
    LIBS += -L$$(NC4_INSTALL_DIR)/lib -lnetcdf
}
unix:{
    LIBS += -lnetcdf
    LIBS += -lstdc++fs
}
macx:{
    LIBS += -L/usr/local/lib/ -lnetcdf
    LIBS += -lstdc++fs
}
