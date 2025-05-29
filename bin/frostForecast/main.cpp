#include <QCoreApplication>
#include <QDir>
#include <iostream>
#include "utilities.h"
#include "frost.h"

// uncomment to execute test
// #define TEST

void usage()
{
    std::cout << "frostForecast" << std::endl
              << "Usage: frostForecast <project.ini> [-calibrate] [-date]" << std::endl;
    std::cout << std::flush;
}

int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    QString settingsFileName;
    QString runDateStr;
    Frost frost;
    bool calibrateModel = false;
    bool outofperiod = false;

    if (argc <= 2)
    {
        #ifdef TEST
            QString dataPath;
            if (! searchDataPath(&dataPath)) return -1;

            settingsFileName = dataPath + "PROJECT/testFrostForecast/testFrostForecastSettings.ini";
            QDate yesterday = QDate::currentDate().addDays(-1);
            runDateStr = yesterday.toString("yyyy-MM-dd");
        #else
            usage();
            return ERROR_MISSINGFILE;
        #endif
    }
    else
    {
        settingsFileName = argv[1];
        if (settingsFileName.right(3) != "ini")
        {
            frost.logger.writeError("Wrong file .ini: " + settingsFileName);
            usage();
            return ERROR_MISSINGFILE;
        }

        if (strcmp(argv[2], "-calibrate") == 0)
            calibrateModel = true;
        else if(strcmp(argv[2], "-outofperiod") == 0)
            outofperiod = true;
        else
            runDateStr = argv[2];
    }

    frost.initialize();
    frost.setSettingsFileName(settingsFileName);

    frost.logger.writeInfo ("settingsFileName: " + settingsFileName);

    int result = frost.readSettings();
    if (result!=FROSTFORECAST_OK) return result;

    int i = 0;

    bool calibrationOk = false;
    if (calibrateModel)
    {
        QList<Crit3DMeteoPoint> pointList = frost.getMeteoPointsList();

        frost.initializeFrostParam();
        if (! frost.setCalibrationLog())
           frost.logger.writeError("Error opening calibration log file");

        for (int i=0; i < pointList.size(); i++)
        {
            if (frost.calibrateModel(i))
            {
                calibrationOk = true;
                frost.logger.writeInfo ("calibration done for point: " + QString::fromStdString(pointList[i].id));
            }
        }

        if (calibrationOk) frost.saveParameters();
    }
    else if (outofperiod)
    {
        result = frost.readParameters();
        if (result != FROSTFORECAST_OK) return result;

        QList<QString> idList = frost.getIdList();

        result = frost.createPointsJsonFile(idList);
        if (result != FROSTFORECAST_OK) return result;
    }
    else
    {
        // check date
        QDate runDate = QDate::fromString(runDateStr, "yyyy-MM-dd");
        if (! runDate.isValid())
        {
            frost.logger.writeError("Wrong date format. Requested format is: YYYY-MM-DD");
            return ERROR_WRONGDATE;
        }

        result = frost.readParameters();
        if (result != FROSTFORECAST_OK) return result;

        QList<QString> idList = frost.getIdList();
        frost.setRunDate(runDate);

        result = frost.downloadMeteoPointsData();
        if (result != FROSTFORECAST_OK) return result;

        QList<QString> idListActive;

        for (i=0; i < idList.size(); i++)
        {
            result = frost.getForecastData(i);
            if (result == FROSTFORECAST_OK)
            {
                frost.createCsvFile(idList[i]);
                idListActive.push_back(idList[i]);
            }
        }

        result = frost.createPointsJsonFile(idListActive);
        if (result != FROSTFORECAST_OK) return result;
    }

    return FROSTFORECAST_OK;
}
