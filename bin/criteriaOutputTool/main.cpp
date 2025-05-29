#include <QCoreApplication>
#include <QDir>

#include "commonConstants.h"
#include "utilities.h"
#include "criteriaOutputProject.h"
#include <iostream>

// uncomment to compute test
// #define TEST


void version()
{
    std::cout << "CriteriaOutput " << CRITERIA_VERSION << std::endl;
    std::cout << "Post processing of the agro-hydrological outputs of CRITERIA1D-GEO" << std::endl;
}

void usage()
{
    std::cout << std::endl << "Usage:" << std::endl
              << "CriteriaOutput <CSV|SHAPEFILE|MAPS|NETCDF|AGGREGATION|DTX> <projectName.ini> [computationDate]" << std::endl
              << "computationDate must be in YYYY-MM-DD format, default date is today." << std::endl << std::endl;
    std::cout << std::flush;
}


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    version();

    CriteriaOutputProject myProject;

    QString appPath = myApp.applicationDirPath() + "/";
    QString settingsFileName, dateComputationStr, operationStr;

    if (argc <= 2)
    {
        #ifdef TEST
            QString dataPath;
            if (! searchDataPath(&dataPath))
                return -1;

            settingsFileName = "//Icolt-smr/criteria1D/PROJECTS/icolt2025_JJA/seasonalIrriForecast_AL.ini";
            //settingsFileName = "C:/SOFTWARE/AGRO/CRITERIA/PROJECT/BOLLAGRO/test.ini";
            dateComputationStr = QDateTime::currentDateTime().date().toString("yyyy-MM-dd");
            operationStr = "AGGREGATION";
        #else
            usage();
            return ERROR_MISSINGPARAMETERS;
        #endif
    }
    else
    {
        operationStr = argv[1];
        operationStr = operationStr.toUpper();

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
    if (settingsFileName.at(0) == '.')
    {
        settingsFileName = appPath + settingsFileName;
        QDir::cleanPath(settingsFileName);
    }

    // initialize
    int myResult = myProject.initializeProject(settingsFileName, operationStr, dateComputation, true);
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
        bool isReorder = true;
        myResult = myProject.createAggregationFile(isReorder);
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
        myProject.logger.writeError("Wrong operation: " + myProject.operation);
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
        myProject.logger.writeInfo("END");
    }

    return myResult;
}

