#include "frost.h"
#include "download.h"
#include "commonConstants.h"
#include "utilities.h"
#include "solarRadiation.h"
#include <math.h>
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

int Frost::getForecastData(QString id, int posIdList)
{
    myForecast.clear();
    myForecastMax.clear();
    myForecastMin.clear();
    myObsData.clear();
    myDate.clear();

    int meteoPointListpos = 0;
    bool found = false;
    for (; meteoPointListpos< meteoPointsList.size(); meteoPointListpos++)
    {
        if (meteoPointsList[meteoPointListpos].id == id.toStdString())
        {
            found = true;
            break;
        }
    }
    // get row and col
    int row;
    int col;

    double lat = meteoPointsList[meteoPointListpos].latitude;
    double lon = meteoPointsList[meteoPointListpos].longitude;

    gis::getMeteoGridRowColFromXY(grid.gridStructure().header(), lon, lat, &row, &col);

    TradPoint myRadPoint;
    Crit3DRadiationSettings radSettings;

    radSettings.gisSettings = &gisSettings;
    /*! assign topographic height and coordinates */
    myRadPoint.x = meteoPointsList[meteoPointListpos].point.utm.x;
    myRadPoint.y = meteoPointsList[meteoPointListpos].point.utm.y;
    myRadPoint.height = meteoPointsList[meteoPointListpos].point.z;

    if (!found)
    {
        logger.writeError ("missing id "+id+" into point_properties table");
        return ERROR_DBPOINT;
    }
    // load meteo point observed data
    if (!meteoPointsDbHandler.loadHourlyData(getCrit3DDate(runDate.addDays(-1)), getCrit3DDate(runDate.addDays(2)), &meteoPointsList[meteoPointListpos]))
    {
        logger.writeError ("id: "+id+" meteo point load hourly data error");
        return ERROR_DBPOINT;
    }
    // load meteoGrid forecast data
    std::string idGrid;
    if (!grid.meteoGrid()->getIdFromLatLon(lat, lon, &idGrid))
    {
        logger.writeError ("lat: "+QString::number(lat)+" lon: "+QString::number(lon)+" id grid not found");
        return ERROR_DBGRID;
    }
    QDateTime firstDateTime = QDateTime(runDate.addDays(-1), QTime(1,0), Qt::UTC);
    QDateTime lastDateTime = QDateTime(runDate.addDays(2), QTime(0,0), Qt::UTC);

    if (!grid.loadGridHourlyData(&errorString, QString::fromStdString(idGrid), firstDateTime, lastDateTime))
    {
        logger.writeError ("meteo grid load hourly data error");
    }
    // Sun position
    TsunPosition sunPosition;
    int timeZone = gisSettings.timeZone;
    float temperature = 25;
    float pressure =  1013;
    float radAspect = 0;
    float radSlope = 0;
    if (radiation::computeSunPosition(float(lon), float(lat), timeZone,
        runDate.year(), runDate.month(), runDate.day(), 0, 0, 0,
        temperature, pressure, radAspect, radSlope, &sunPosition))
    {
        int myHourSunSetInteger = round(sunPosition.set/3600);
        int myHourSunRiseInteger = round(sunPosition.rise/3600);

        indexSunSet = myHourSunSetInteger;
        indexSunRise = 24 + myHourSunRiseInteger;

        QDate firstDate = getQDate(meteoPointsList[meteoPointListpos].getMeteoPointHourlyValuesDate(0));
        int myDateIndex = firstDate.daysTo(runDate);
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
        if (myDateIndex > meteoPointsList[meteoPointListpos].nrObsDataDaysH || myDateIndex < 0)
        {
            logger.writeError ("Sunset hour: " + QString::number(myHourSunSetInteger) + " data not available");
            return ERROR_SUNSET;
        }
        QDate newDate = firstDate.addDays(myDateIndex);
        float myTSunSet = meteoPointsList[meteoPointListpos].getMeteoPointValueH(getCrit3DDate(newDate), myHour, 0, airTemperature);
        float myRHSunSet = meteoPointsList[meteoPointListpos].getMeteoPointValueH(getCrit3DDate(newDate), myHour, 0, airRelHumidity);

        if (myTSunSet == NODATA || myRHSunSet == NODATA)
        {
            logger.writeError ("Sunset data not available");
            return ERROR_MISSINGDATA;
        }

        int myDateTmpIndex;
        int myTmpHour;
        QDateTime dateTimeTmp;

        float myIntercept = intercept[posIdList].toFloat();
        float myParTss = parTss[posIdList].toFloat();
        float myParRHss = parRHss[posIdList].toFloat();
        float mySEintercept = SE_intercept[posIdList].toFloat();
        float mySEparTss = SE_parTss[posIdList].toFloat();
        float mySEparRHss = SE_parRHss[posIdList].toFloat();

        for (int i = 0; i <= 1; i++)
        {
            for (int j = 0; j<=23; j++)
            {
                myDateTmpIndex = myDateIndex + i;
                myTmpHour = j - timeZone;
                if (myTmpHour < 0)
                {
                    myDateTmpIndex = myDateTmpIndex - 1;
                    myTmpHour = myTmpHour + 24;
                }
                else if (myTmpHour > 23)
                {
                    myDateTmpIndex = myDateTmpIndex + 1;
                    myTmpHour = myTmpHour - 24;
                }
                // observed values (local time)
                if (myDateTmpIndex <= meteoPointsList[meteoPointListpos].nrObsDataDaysH && myDateTmpIndex >= 0)
                {
                    QDate dateTmp = firstDate.addDays(myDateTmpIndex);
                    dateTimeTmp = QDateTime(dateTmp,QTime(myTmpHour,0,0));
                    myDate.append(QDateTime(runDate.addDays(i),QTime(j,0,0)));
                    float myT = meteoPointsList[meteoPointListpos].getMeteoPointValueH(getCrit3DDate(dateTmp), myTmpHour, 0, airTemperature);
                    if (myT != NODATA)
                    {
                        myObsData.append(myT);
                    }
                    else
                    {
                         myObsData.append(NODATA);
                    }
                }
                else
                {
                     myObsData.append(NODATA);
                }
            }
        } // end for

        float myCoeffReuter = coeffReuter(myIntercept, myParTss, myParRHss, myTSunSet, myRHSunSet);
        float myCoeffReuterMin = coeffReuter(myIntercept - 2 * abs(mySEintercept), myParTss - 2 * abs(mySEparTss), myParRHss - 2 * abs(mySEparRHss), myTSunSet, myRHSunSet);
        float myCoeffReuterMax = coeffReuter(myIntercept + 2 * abs(mySEintercept), myParTss + 2 * abs(mySEparTss), myParRHss + 2 * abs(mySEparRHss), myTSunSet, myRHSunSet);

        for (int i = 0; i<myObsData.size(); i++)
        {
            if (i >= indexSunSet && i<= indexSunRise)
            {
                myForecast.append(t_Reuter(myCoeffReuter, i - indexSunSet, myTSunSet));
                myForecastMax.append(t_Reuter(myCoeffReuterMin, i - indexSunSet, myTSunSet));
                myForecastMin.append(t_Reuter(myCoeffReuterMax, i - indexSunSet, myTSunSet));
            }
            else
            {
                myForecast.append(NODATA);
                myForecastMin.append(NODATA);
                myForecastMax.append(NODATA);
            }
        }


    } // end if radiation
    else
    {
        logger.writeError ("Error computing sunset hour");
        return ERROR_SUNSET;
    }
    return FROSTFORECAST_OK;
}

float Frost::coeffReuter(float a0, float a1, float a2, float t, float RH)
{
    float coeffReuter = a0 + a1 * t + a2 * RH;
    return coeffReuter;
}

float Frost::t_Reuter(float d, float deltaTime, float tIni)
{
    float t_Reuter = tIni - d * pow((deltaTime), 0.5);
    return t_Reuter;
}

int Frost::createCsvFile(QString id)
{
    logger.writeInfo("Create CSV");
    // check output csv directory
    if (! QDir(csvFilePath).exists())
    {
        QDir().mkdir(csvFilePath);
    }

    QString outputCsvFileName = csvFilePath + "/"+id + ".csv";
    QFile outputFile(outputCsvFileName);

    if (!outputFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        logger.writeError ("Open failure: " + outputCsvFileName);
        return ERROR_CSVFILE;
    }
    else
    {
        logger.writeInfo("Output file: " + outputCsvFileName);
    }

    QString header = "dateTime,TAVG,FORECAST,FORECAST_MIN, FORECAST_MAX";
    QTextStream out(&outputFile);
    out << header << "\n";
    for (int i = 0; i<myObsData.size(); i++)
    {
        if (i>12 && i<38) // csv file from h.12 to h.12 day after
        {
            out << myDate[i].toString("yyyy-MM-dd hh:mm");
            if (myObsData[i] !=NODATA)
            {
                out << "," << myObsData[i];
            }
            else
            {
                out << "," << "";
            }
            if (myForecast[i] != NODATA)
            {
                out << "," << myForecast[i];
            }
            else
            {
                out << "," << "";
            }
            if (myForecastMin[i] != NODATA)
            {
                out << "," << myForecastMin[i];
            }
            else
            {
                out << "," << "";
            }
            if (myForecastMax[i] != NODATA)
            {
                out << "," << myForecastMax[i];
            }
            else
            {
                out << "," << "";
            }
            out << "\n";
        }
    }
    outputFile.flush();
    outputFile.close();

    return FROSTFORECAST_OK;

}

QList<QString> Frost::getIdList() const
{
    return idList;
}
