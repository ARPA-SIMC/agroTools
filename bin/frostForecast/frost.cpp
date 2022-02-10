#include "frost.h"
#include "download.h"
#include "commonConstants.h"
#include "utilities.h"
#include "solarRadiation.h"
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
    errorString = "";
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

    if (!meteoPointsDbHandler.loadVariableProperties())
    {
        logger.writeError (errorString);
        return ERROR_DBPOINT;
    }

    if (!meteoPointsDbHandler.getPropertiesFromDb(meteoPointsList, gisSettings, errorString))
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

    QString varTemp = projectSettings->value("var","").toString();
    if (varTemp.isEmpty())
    {
        logger.writeError ("missing var");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        varList = varTemp.split(",");
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

void Frost::setRunDate(const QDate &value)
{
    runDate = value;
}

int Frost::downloadMeteoPointsData()
{
    Download* myDownload = new Download(meteoPointsDbHandler.getDbName());

    QList<int> arkIdVarList;

    for (int i = 0; i<varList.size(); i++)
    {
        int id = meteoPointsDbHandler.getArkIdFromVar(varList[i]);
        if (id == NODATA)
        {
            return ERROR_DBPOINTSDOWNLOAD;
        }
        arkIdVarList.append(id);
    }

    QMultiMap<QString, QString> mapDatasetId;
    for( int i=0; i < idList.size(); i++ )
    {
        QString dataset = meteoPointsDbHandler.getDatasetFromId(idList[i]);
        mapDatasetId.insert(dataset,idList[i]);
    }

    QList<QString> keys = mapDatasetId.uniqueKeys();
    for (int i = 0; i < keys.size(); i++)
    {
        if (!myDownload->downloadHourlyData(runDate.addDays(-1), runDate.addDays(2), keys[i], mapDatasetId.values(keys[i]), arkIdVarList) )
        {
            return ERROR_DBPOINTSDOWNLOAD;
        }
    }
    return FROSTFORECAST_OK;
}

int Frost::getForecastData(QString id)
{
    int pos = 0;
    bool found = false;
    for (; pos< meteoPointsList.size(); pos++)
    {
        if (meteoPointsList[pos].id == id.toStdString())
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        logger.writeError ("missing id "+id+" into point_properties table");
        return ERROR_DBPOINT;
    }
    // load meteo point observed data
    if (!meteoPointsDbHandler.loadHourlyData(getCrit3DDate(runDate.addDays(-1)), getCrit3DDate(runDate.addDays(2)), &meteoPointsList[pos]))
    {
        logger.writeError ("id: "+id+" meteo point load hourly data error");
        return ERROR_DBPOINT;
    }
    // load meteoGrid forecast data
    QDateTime firstDateTime = QDateTime(runDate.addDays(-1), QTime(1,0), Qt::UTC);
    QDateTime lastDateTime = QDateTime(runDate.addDays(2), QTime(0,0), Qt::UTC);
    if (grid.loadGridHourlyData(&errorString, id, firstDateTime, lastDateTime))
    {
        logger.writeError ("id: "+id+" meteo grid load hourly data error");
        return ERROR_DBGRID;
    }
    // Sun position
    TsunPosition sunPosition;
    int timeZone = gisSettings.timeZone;
    float temperature = 25;
    float pressure =  1013;
    float radAspect = 0;
    float radSlope = 0;
    if (radiation::computeSunPosition(float(meteoPointsList[pos].longitude), float(meteoPointsList[pos].latitude), timeZone,
        runDate.year(), runDate.month(), runDate.day(), 0, 0, 0,
        temperature, pressure, radAspect, radSlope, &sunPosition))
    {
        int myHourSunSetInteger = sunPosition.set;
        int myHourSunRiseInteger = sunPosition.rise;

        int indexSunSet = myHourSunSetInteger;
        int indexSunRise = 24 + myHourSunRiseInteger;

        QDate fistDate = getQDate(meteoPointsList[pos].getMeteoPointHourlyValuesDate(0));
        int myDateIndex = fistDate.daysTo(runDate);
        int myHour = myHourSunSetInteger - timeZone;

        if (myHour < 0)
        {
            myDateIndex = myDateIndex - 1;
            myHour = myHourSunSetInteger + 24;
        }
        else if (myHour > 23)
        {
            myDateIndex = myDateIndex + 1;
            myHour = myHourSunSetInteger - 24;
        }
        if (myDateIndex > meteoPointsList[pos].nrObsDataDaysH || myDateIndex < 0)
        {
            logger.writeError ("Sunset hour: " + QString::number(myHourSunSetInteger) + " data not available");
            return ERROR_SUNSET;
        }
        QDate newDate = runDate.addDays(myDateIndex);
        float tavg = meteoPointsList[pos].getMeteoPointValueH(getCrit3DDate(newDate), myHour, 0, airTemperature);
        float rhavg = meteoPointsList[pos].getMeteoPointValueH(getCrit3DDate(newDate), myHour, 0, airRelHumidity);

        // myTQuality = Quality.checkValueHourly(Definitions.HOURLY_TAVG, myHourlyData(myDateIndex), myHour, myTSunSet,meteoPoint(myStationIndex).Point.z)
        // NON esiste questa funzione ci sono i check solo per i daily è da inserire?

        // TO DO

    }
}

