#include <QCoreApplication>
#include <QDir>

#include "commonConstants.h"
#include "criteriaOutputProject.h"
#include "utilities.h"
#include <iostream>

// uncomment to compute test
//#define TEST

void usage()
{
    std::cout << "\nUsage:\nCriteriaOutput DTX|CSV|SHAPEFILE|MAPS|NETCDF|AGGREGATION projectName.ini [computationDate]\n" << std::endl;
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
            if (! searchDataPath(&dataPath))
                return -1;

            settingsFileName = "//tomei-smr/SOFTWARE/AGRO/CRITERIA/PROJECT/BOLLAGRO/bollagro.ini";
            //dateComputationStr = "2023-08-15";
            dateComputationStr = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
            myProject.operation = "MAPS";
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

    if (settingsFileName.isEmpty())
    {
        myProject.logger.writeError("Missing file .ini");
        usage();
        return ERROR_WRONGPARAMETER;
    }

    // complete path
    if (settingsFileName.at(0) == ".")
    {
        settingsFileName = appPath + settingsFileName;
        QDir::cleanPath(settingsFileName);
    }

    // initialize
    int myResult = myProject.initializeProject(settingsFileName, myProject.operation, dateComputation, true);
    if (myResult != CRIT1D_OK)
    {
        myProject.logger.writeError(myProject.projectError);
        return myResult;
    }
    myProject.logger.writeInfo("computation date: " + dateComputationStr);

    if (myProject.operation == "DTX")
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

