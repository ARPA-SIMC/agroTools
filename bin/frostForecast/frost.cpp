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
    idList.clear();
    intercept.clear();
    parTss.clear();
    parRHss.clear();
    SE_intercept.clear();
    SE_parTss.clear();
    SE_parRHss.clear();
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

    // reuter_param
    projectSettings->beginGroup("reuter_param");
    QString interceptTemp = projectSettings->value("intercept","").toString();
    if (interceptTemp.isEmpty())
    {
        logger.writeError ("missing reuter param intercept");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        intercept = interceptTemp.split(",");
    }
    QString parTssTemp = projectSettings->value("parTss","").toString();
    if (parTssTemp.isEmpty())
    {
        logger.writeError ("missing reuter param parTss");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        parTss = parTssTemp.split(",");
    }
    QString parRHssTemp = projectSettings->value("parRHss","").toString();
    if (parRHssTemp.isEmpty())
    {
        logger.writeError ("missing reuter param parRHss");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        parRHss = parRHssTemp.split(",");
    }
    QString SEinterceptTemp = projectSettings->value("SE_intercept","").toString();
    if (SEinterceptTemp.isEmpty())
    {
        logger.writeError ("missing reuter param SE_intercept");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        SE_intercept = SEinterceptTemp.split(",");
    }
    QString SEparTssTemp = projectSettings->value("SE_parTss","").toString();
    if (SEparTssTemp.isEmpty())
    {
        logger.writeError ("missing reuter param SE_parTss");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        SE_parTss = SEparTssTemp.split(",");
    }
    QString SEparRHssTemp = projectSettings->value("SE_parRHss","").toString();
    if (SEparRHssTemp.isEmpty())
    {
        logger.writeError ("missing reuter param SE_parRHss");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        SE_parRHss = SEparRHssTemp.split(",");
    }

    return FROSTFORECAST_OK;
}
