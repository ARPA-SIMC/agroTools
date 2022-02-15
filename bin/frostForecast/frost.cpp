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
    myForecast.clear();
    myForecastMax.clear();
    myForecastMin.clear();
    myObsData.clear();
    myDate.clear();

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
    // get row and col
    int row;
    int col;

    double lat = meteoPointsList[pos].latitude;
    double lon = meteoPointsList[pos].longitude;

    gis::getMeteoGridRowColFromXY(grid.gridStructure().header(), meteoPointsList[pos].point.utm.x, meteoPointsList[pos].point.utm.y, &row, &col);

    TradPoint myRadPoint;
    Crit3DRadiationSettings radSettings;
    gis::Crit3DRasterGrid myDEM;
    radSettings.gisSettings = &gisSettings;
    /*! assign topographic height and coordinates */
    myRadPoint.x = meteoPointsList[pos].point.utm.x;
    myRadPoint.y = meteoPointsList[pos].point.utm.y;
    myRadPoint.height = meteoPointsList[pos].point.z;

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
    std::string idGrid;
    if (grid.meteoGrid()->getIdFromLatLon(lat, lon, &idGrid))
    {
        logger.writeError ("lat: "+QString::number(lat)+" lon: "+QString::number(lon)+" id grid not found");
        return ERROR_DBGRID;
    }
    QDateTime firstDateTime = QDateTime(runDate.addDays(-1), QTime(1,0), Qt::UTC);
    QDateTime lastDateTime = QDateTime(runDate.addDays(2), QTime(0,0), Qt::UTC);
    bool gridAvailable = false;
    if (grid.loadGridHourlyData(&errorString, QString::fromStdString(idGrid), firstDateTime, lastDateTime))
    {
        gridAvailable = true;
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
        float myTSunSet = meteoPointsList[pos].getMeteoPointValueH(getCrit3DDate(newDate), myHour, 0, airTemperature);
        float myRHSunSet = meteoPointsList[pos].getMeteoPointValueH(getCrit3DDate(newDate), myHour, 0, airRelHumidity);

        // myTQuality = Quality.checkValueHourly(Definitions.HOURLY_TAVG, myHourlyData(myDateIndex), myHour, myTSunSet,meteoPoint(myStationIndex).Point.z)
        // NON esiste questa funzione ci sono i check solo per i daily è da inserire?

        int myDateTmpIndex;
        int myTmpHour;

        float myIntecept = intercept[pos].toFloat();
        float myParTss = parTss[pos].toFloat();
        float myParRHss = parRHss[pos].toFloat();
        float mySEintercept = SE_intercept[pos].toFloat();
        float mySEparTss = SE_parTss[pos].toFloat();
        float mySEparRHss = SE_parRHss[pos].toFloat();

        //float myObsData[47];
        //float myForecast[indexSunRise - indexSunSet + 1];
        //float myForecastMin[indexSunRise - indexSunSet + 1];
        //float myForecastMax[indexSunRise - indexSunSet + 1];
        //float myCloudiness[indexSunRise-indexSunSet+1];
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
                if (myDateTmpIndex <= meteoPointsList[pos].nrObsDataDaysH && myDateTmpIndex >= 0)
                {
                    //myTQuality = Quality.checkValueHourly(Definitions.HOURLY_TAVG, myHourlyData(myDateTmpIndex), myTmpHour, myT, meteoPoint(myStationIndex).Point.z)
                    QDate dateTmp = runDate.addDays(myDateTmpIndex);
                    myDate.append(QDateTime(dateTmp,QTime(myTmpHour,0,0)));
                    float myT = meteoPointsList[pos].getMeteoPointValueH(getCrit3DDate(dateTmp), myTmpHour, 0, airTemperature);
                    //If myTQuality < Quality.qualityWrongData Then
                    myObsData[i * 24 + j] = myT;
                    //Else
                    //myObsData(i * 24 + j) = Definitions.NO_DATA
                }
                else
                {
                    myObsData[i * 24 + j] = NODATA;
                }

                // cloudiness forecast data (local time)
                if ((i * 24 + j) >= indexSunSet && (i * 24 + j) <= indexSunRise)
                {
                    radiation::computeRadiationRSunMeteoPoint(&radSettings, myDEM, grid.meteoGrid()->meteoPointPointer(row,col), myRadPoint, row, col, getCrit3DTime(myDate.last()));
                    if (gridAvailable)
                    {
                        float rad = grid.meteoGrid()->meteoPoint(row, col).getMeteoPointValueH(getCrit3DDate(myDate.last().date()), myDate.last().time().hour(), 0, directIrradiance);
                        myCloudiness[i * 24 + j - indexSunSet] = myRadPoint.global/rad;
                    }
                    else
                    {
                        myCloudiness[i * 24 + j - indexSunSet] = NODATA;
                    }
                }
            }
        } // end for

        float myCoeffReuter = coeffReuter(myIntecept, myParTss, myParRHss, myTSunSet, myRHSunSet);
        float myCoeffReuterMin = coeffReuter(myIntecept - 2 * abs(mySEintercept), myParTss - 2 * abs(mySEparTss), myParRHss - 2 * abs(mySEparRHss), myTSunSet, myRHSunSet);
        float myCoeffReuterMax = coeffReuter(myIntecept + 2 * abs(mySEintercept), myParTss + 2 * abs(mySEparTss), myParRHss + 2 * abs(mySEparRHss), myTSunSet, myRHSunSet);

        for (int i = indexSunSet; i<= indexSunRise; i++)
        {
            myForecast[i - indexSunSet] = t_Reuter(myCoeffReuter, i - indexSunSet, myTSunSet);
            myForecastMin[i - indexSunSet] = t_Reuter(myCoeffReuterMin, i - indexSunSet, myTSunSet);
            myForecastMax[i - indexSunSet] = t_Reuter(myCoeffReuterMax, i - indexSunSet, myTSunSet);
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
    float t_Reuter = pow(tIni - d * (deltaTime), 0.5);
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

    QString outputCsvFileName = csvFilePath + id + ".csv";
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

    QString header = "dateTime,TAVG,FORECAST,FORECAST_MIN, FORECAST_MAX, CLOUDINESS";
    QTextStream out(&outputFile);
    out << header << "\n";
    for (int i = 0; i<myForecast.size(); i++)
    {
        out << myDate[i].toString("yyyy-MM-dd hh:mm");
        out << "," << myObsData[i];
        out << "," << myForecast[i];
        out << "," << myForecastMin[i];
        out << "," << myForecastMax[i];
        //out << "," << myCloudiness[i];
        out << "\n";
    }
    outputFile.flush();
    outputFile.close();

    return FROSTFORECAST_OK;

}

QList<QString> Frost::getIdList() const
{
    return idList;
}






