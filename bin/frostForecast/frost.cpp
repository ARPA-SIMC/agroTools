#include "frost.h"
#include <QSettings>
#include <QDir>
#include <QTextStream>


Frost::Frost()
{

}

void Frost::initialize()
{
    path = "";
    projectName = "";
    settingsFileName = "";
    csvFileName = "";
    xmlDbGrid = "";
    dbMeteoPointsName = "";
}

void Frost::setSettingsFileName(const QString &value)
{
    settingsFileName = value;
}

int Frost::readSettings()
{
    QSettings* projectSettings;

    // Configuration file
    QFile myFile(settingsFileName);
    if (!myFile.exists())
    {
        logger.writeError("setting file doesn't exist");
        return ERROR_MISSINGFILE;
    }
    else
    {
        settingsFileName = QDir(myFile.fileName()).canonicalPath();
        settingsFileName = QDir().cleanPath(settingsFileName);

        QFileInfo fileInfo(settingsFileName);
        path = fileInfo.path() + "/";
    }
    projectSettings = new QSettings(settingsFileName, QSettings::IniFormat);
    // LOCATION
    projectSettings->beginGroup("location");
    projectSettings->setValue("lat", gisSettings.startLocation.latitude);
    projectSettings->setValue("lon", gisSettings.startLocation.longitude);
    projectSettings->setValue("utm_zone", gisSettings.utmZone);
    projectSettings->setValue("time_zone", gisSettings.timeZone);
    projectSettings->setValue("is_utc", gisSettings.isUTC);
    projectSettings->endGroup();

    // PROJECT
    projectSettings->beginGroup("project");
    projectName = projectSettings->value("name","").toString();

    xmlDbGrid = projectSettings->value("meteoGrid","").toString();
    if (xmlDbGrid.isEmpty())
    {
        logger.writeError ("missing xml DB grid");
        return ERROR_MISSINGPARAMETERS;
    }
    if (xmlDbGrid.left(1) == ".")
    {
        xmlDbGrid = path + QDir::cleanPath(xmlDbGrid);
    }

    // open grid
    QString errorString;
    if (! grid.parseXMLGrid(xmlDbGrid, &errorString))
    {
        logger.writeError (errorString);
        return ERROR_DBGRID;
    }

    if (! grid.openDatabase(&errorString))
    {
        logger.writeError (errorString);
        return ERROR_DBGRID;
    }

    if (! grid.loadCellProperties(&errorString))
    {
        logger.writeError (errorString);
        grid.closeDatabase();
        return ERROR_DBGRID;
    }
    grid.closeDatabase();

    dbMeteoPointsName = projectSettings->value("meteo_points","").toString();
    if (dbMeteoPointsName.isEmpty())
    {
        logger.writeError ("missing meteo points db");
        return ERROR_MISSINGPARAMETERS;
    }
    if (dbMeteoPointsName.left(1) == ".")
    {
        dbMeteoPointsName = path + QDir::cleanPath(dbMeteoPointsName);
    }

    if (!meteoPointsDbHandler.setAndOpenDb(dbMeteoPointsName))
    {
        logger.writeError (errorString);
        return ERROR_DBPOINT;
    }

    csvFilePath = projectSettings->value("csvPath","").toString();
    if (csvFilePath.isEmpty())
    {
        logger.writeError ("missing csvPath");
        return ERROR_MISSINGPARAMETERS;
    }
    if (csvFilePath.left(1) == ".")
    {
        csvFilePath = path + QDir::cleanPath(csvFilePath);
    }

    QString idTemp = projectSettings->value("id","").toString();
    if (idTemp.isEmpty())
    {
        logger.writeError ("missing id station");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        idList = idTemp.split(",");
    }
    projectSettings->endGroup();

    return FROSTFORECAST_OK;
}
