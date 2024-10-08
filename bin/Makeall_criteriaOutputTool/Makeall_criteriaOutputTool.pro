TEMPLATE = subdirs

# comment to compile without GDAL library
CONFIG += GDAL

GDAL {
     SUBDIRS =  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis  \
                ../../agrolib/utilities  ../../agrolib/shapeHandler \
                ../../agrolib/shapeUtilities ../../agrolib/gdalHandler ../../agrolib/netcdfHandler \
                ../../agrolib/criteriaOutput ../criteriaOutputTool
     }
else {
     SUBDIRS =  ../../agrolib/crit3dDate ../../agrolib/mathFunctions ../../agrolib/gis  \
                ../../agrolib/utilities  ../../agrolib/shapeHandler \
                ../../agrolib/shapeUtilities ../../agrolib/netcdfHandler \
                ../../agrolib/criteriaOutput ../criteriaOutputTool
     }

CONFIG += ordered 

