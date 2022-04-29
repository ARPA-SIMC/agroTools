#include <QCoreApplication>
#include <QDir>

#include "commonConstants.h"
#include "criteriaOutputProject.h"
#include "utilities.h"
#include <iostream>

// uncomment to compute test
// #define TEST

void usage()
{
    std::cout << "\nUsage:\nCriteriaOutput CSV|SHAPEFILE|MAPS|NETCDF|AGGREGATION|PRECOMPUTE_DTX project.ini [date]\n" << std::endl;
    std::cout << std::flush;
}


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    CriteriaOutputProject myProject;

    QString appPath = myApp.applicationDirPath() + "/";
    QString settingsFileName, dateComputationStr;

    if (argc <= 2)
    {
        #ifdef TEST
            QString dataPath;
            if (! searchDataPath(&dataPath)) return -1;

            settingsFileName = dataPath + "PROJECT/C5/C5_monthly.ini";
            dateComputationStr = "2021-07-13"; // QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
            myProject.operation = "NETCDF";
        #else
            usage();
            return ERROR_MISSINGPARAMETERS;
        #endif
    }
    else
    {
        myProject.operation = argv[1];
        myProject.operation = myProject.operation.toUpper();

        settingsFileName = argv[2];
        if (settingsFileName.right(3) != "ini")
        {
            myProject.logger.writeError("Wrong file .ini: " + settingsFileName);
            usage();
            return ERROR_WRONGPARAMETER;
        }

        if (argc > 3)
        {
            dateComputationStr = argv[3];
        }
        else
        {
            dateComputationStr = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
        }
    }

    // check date
    QDate dateComputation = QDate::fromString(dateComputationStr, "yyyy-MM-dd");
    if (! dateComputation.isValid())
    {
        myProject.logger.writeError("Wrong date format. Requested format is: YYYY-MM-DD");
        return ERROR_WRONGDATE;
    }

    // complete path
    if (settingsFileName.left(1) == ".")
    {
        settingsFileName = appPath + settingsFileName;
    }

    // initialize
    int myResult = myProject.initializeProject(settingsFileName, myProject.operation, dateComputation, true);
    if (myResult != CRIT1D_OK)
    {
        myProject.logger.writeError(myProject.projectError);
        return myResult;
    }
    myProject.logger.writeInfo("computation date: " + dateComputationStr);

    if (myProject.operation == "PRECOMPUTE_DTX")
    {
        myResult = myProject.precomputeDtx();
    }
    else if (myProject.operation == "CSV")
    {
        myResult = myProject.createCsvFile();
    }
    else if (myProject.operation == "SHAPE" || myProject.operation == "SHAPEFILE")
    {
        myResult = myProject.createShapeFile();
    }
    else if (myProject.operation == "NETCDF")
    {
        myResult = myProject.createNetcdf();
    }
    else if (myProject.operation == "AGGREGATION")
    {
        myResult = myProject.createAggregationFile();
    }
    else if (myProject.operation == "MAPS")
    {
        #ifdef GDAL
            myResult = myProject.createMaps();
        #else
            myProject.logger.writeError("MAPS are not available (need GDAL library).");
            return ERROR_MISSING_GDAL;
        #endif
    }
    else
    {
        myProject.logger.writeError("Wrong parameter: " + myProject.operation);
        usage();
        return ERROR_WRONGPARAMETER;
    }

    if (myResult == CRIT1D_OK)
    {
        myProject.logger.writeInfo("END");
    }
    else
    {
        myProject.logger.writeError(myProject.projectError);
    }

    return myResult;
}

