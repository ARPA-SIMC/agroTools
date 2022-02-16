#include <QCoreApplication>
#include <QDir>
#include <iostream>
#include "utilities.h"
#include "frost.h"

// uncomment to execute test
#define TEST

void usage()
{
    std::cout << "frostForecast" << std::endl
              << "Usage: frostForecast <project.ini> [date]" << std::endl;
    std::cout << std::flush;
}

int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    QString settingsFileName;
    QString runDateStr;
    Frost frost;

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
        runDateStr = argv[2];
    }

    // check date
    QDate runDate = QDate::fromString(runDateStr, "yyyy-MM-dd");
    if (! runDate.isValid())
    {
        frost.logger.writeError("Wrong date format. Requested format is: YYYY-MM-DD");
        return ERROR_WRONGDATE;
    }

    frost.initialize();
    frost.setSettingsFileName(settingsFileName);
    frost.setRunDate(runDate);
    frost.logger.writeInfo ("settingsFileName: " + settingsFileName);

    int result = frost.readSettings();
    if (result!=FROSTFORECAST_OK)
    {
        return result;
    }

    result = frost.downloadMeteoPointsData();
    if (result!=FROSTFORECAST_OK)
    {
        return result;
    }

    QList<QString> idList = frost.getIdList();
    for (int i = 0; i< idList.size(); i++)
    {
        result = frost.getForecastData(idList[i]);
        if (result == FROSTFORECAST_OK)
        {
            frost.createCsvFile(idList[i]);
        }
    }
}
