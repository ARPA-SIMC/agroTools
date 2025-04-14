#include "frost.h"
#include "basicMath.h"
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
    dbMeteoPointsName = "";
    errorString = "";

    initializeFrostParam();

    monthIni = 1;
    monthFin = 12;
    thresholdTmin = 0;
    thresholdTrange = 10;
    historyDateEnd = QDate::currentDate().addDays(-1);
    historyDateStart = historyDateStart.addDays(-365);
    minPercentage = 0.8f;
    logCalibrationName = "";
}

void Frost::initializeFrostParam()
{
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

int Frost::readParameters()
{
    QSettings* projectSettings = new QSettings(settingsFileName, QSettings::IniFormat);

    projectSettings->beginGroup("project");
    idList = projectSettings->value("id").toStringList();
    if (idList.isEmpty())
    {
        logger.writeError ("missing id station");
        return ERROR_MISSINGPARAMETERS;
    }
    projectSettings->endGroup();

    QStringList myList;

    // reuter_param
    projectSettings->beginGroup("reuter_param");

    myList = projectSettings->value("intercept").toStringList();
    if (myList.isEmpty())
    {
        logger.writeError ("missing reuter param intercept");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        intercept = StringListToFloat(myList);
    }

    myList = projectSettings->value("parTss").toStringList();
    if (myList.isEmpty())
    {
        logger.writeError ("missing reuter param parTss");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        parTss = StringListToFloat(myList);
    }

    myList = projectSettings->value("parRHss").toStringList();
    if (myList.isEmpty())
    {
        logger.writeError ("missing reuter param parRHss");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        parRHss = StringListToFloat(myList);
    }

    myList = projectSettings->value("SE_intercept").toStringList();
    if (myList.isEmpty())
    {
        logger.writeError ("missing reuter param SE_intercept");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        SE_intercept = StringListToFloat(myList);
    }

    myList = projectSettings->value("SE_parTss").toStringList();
    if (myList.isEmpty())
    {
        logger.writeError ("missing reuter param SE_parTss");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        SE_parTss = StringListToFloat(myList);
    }

    myList = projectSettings->value("SE_parRHss").toStringList();
    if (myList.isEmpty())
    {
        logger.writeError ("missing reuter param SE_parRHss");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        SE_parRHss = StringListToFloat(myList);
    }

    projectSettings->endGroup();

    return FROSTFORECAST_OK;
}

void Frost::saveParameters()
{
    QSettings* projectSettings = new QSettings(settingsFileName, QSettings::IniFormat);

    projectSettings->beginGroup("project");
    projectSettings->setValue("id", idList);
    projectSettings->endGroup();

    projectSettings->beginGroup("reuter_param");
    projectSettings->setValue("intercept", FloatVectorToStringList(intercept));
    projectSettings->setValue("parTss", FloatVectorToStringList(parTss));
    projectSettings->setValue("parRHss", FloatVectorToStringList(parRHss));
    projectSettings->setValue("SE_intercept", FloatVectorToStringList(SE_intercept));
    projectSettings->setValue("SE_parTss", FloatVectorToStringList(SE_parTss));
    projectSettings->setValue("SE_parRHss", FloatVectorToStringList(SE_parRHss));
    projectSettings->endGroup();

    projectSettings->sync();

}

int Frost::readSettings()
{
    QSettings* projectSettings;

    // Configuration file
    QFile myFile(settingsFileName);
    if (!myFile.exists())
    {
        logger.writeError("setting file does not exist");
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
        logger.writeError (meteoPointsDbHandler.getErrorString());
        return ERROR_DBPOINT;
    }

    if (!meteoPointsDbHandler.loadVariableProperties())
    {
        logger.writeError (meteoPointsDbHandler.getErrorString());
        return ERROR_DBPOINT;
    }

    if (!meteoPointsDbHandler.getPropertiesFromDb(meteoPointsList, gisSettings, errorString))
    {
        logger.writeError (meteoPointsDbHandler.getErrorString());
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

    varList = projectSettings->value("var").toStringList();
    if (varList.isEmpty())
    {
        logger.writeError ("missing var");
        return ERROR_MISSINGPARAMETERS;
    }

    projectSettings->endGroup();

    // calibration
    projectSettings->beginGroup("calibration");
    monthIni = projectSettings->value("monthIni").toInt();
    monthFin = projectSettings->value("monthFin").toInt();
    thresholdTmin = projectSettings->value("thresholdTmin").toFloat();
    thresholdTrange = projectSettings->value("thresholdTrange").toFloat();

    if (projectSettings->contains("history_date_start") && !projectSettings->value("history_date_start").toString().isEmpty())
        historyDateStart = projectSettings->value("history_date_start").toDate();

    if (projectSettings->contains("history_date_end") && !projectSettings->value("history_date_end").toString().isEmpty())
        historyDateEnd = projectSettings->value("history_date_end").toDate();

    if (projectSettings->contains("logCalibrationFile") && !projectSettings->value("logCalibrationFile").toString().isEmpty())
        logCalibrationName = projectSettings->value("logCalibrationFile").toString();

    if (projectSettings->contains("minPercentage") && !projectSettings->value("minPercentage").toString().isEmpty())
        minPercentage = projectSettings->value("minPercentage").toFloat() / 100;

    projectSettings->endGroup();

    return FROSTFORECAST_OK;
}

void Frost::setRunDate(const QDate &value)
{
    runDate = value;
}

int Frost::downloadMeteoPointsData()
{
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

    Download* myDownload = new Download(meteoPointsDbHandler.getDbName());

    QMultiMap<QString, QString> mapDatasetId;
    for( int i=0; i < idList.size(); i++ )
    {
        QString dataset = meteoPointsDbHandler.getDatasetFromId(idList[i]);
        mapDatasetId.insert(dataset,idList[i]);
    }

    QList<QString> keys = mapDatasetId.uniqueKeys();
    for (int i = 0; i < keys.size(); i++)
    {
        QString errorString;
        if (! myDownload->downloadHourlyData(runDate.addDays(-1), runDate.addDays(2), keys[i],
                                            mapDatasetId.values(keys[i]), arkIdVarList, errorString) )
        {
            logger.writeError(errorString);
            return ERROR_DBPOINTSDOWNLOAD;
        }
    }

    delete myDownload;

    return FROSTFORECAST_OK;
}


int Frost::getForecastData(int paramPos)
{
    myForecast.clear();
    myForecastMax.clear();
    myForecastMin.clear();
    myObsData.clear();
    myDateTime.clear();

    int meteoPointListpos = 0;
    bool found = false;
    for (; meteoPointListpos< meteoPointsList.size(); meteoPointListpos++)
    {
        if (meteoPointsList[meteoPointListpos].id == idList[paramPos].toStdString())
        {
            found = true;
            break;
        }
    }

    if (! found)
    {
        logger.writeError ("missing id " + QString::fromStdString(meteoPointsList[meteoPointListpos].id) +" into point_properties table");
        return ERROR_DBPOINT;
    }

    Crit3DMeteoPoint* point = &meteoPointsList[meteoPointListpos];

    // get row and col
    //int row;
    //int col;

    double lat = point->latitude;
    double lon = point->longitude;

    //gis::getGridRowColFromXY(grid.gridStructure().header(), lon, lat, &row, &col);

    //TradPoint myRadPoint;
    Crit3DRadiationSettings radSettings;

    //radSettings.gisSettings = &gisSettings;
    /*! assign topographic height and coordinates */
    //myRadPoint.x = meteoPointsList[meteoPointListpos].point.utm.x;
    //myRadPoint.y = meteoPointsList[meteoPointListpos].point.utm.y;
    //myRadPoint.height = meteoPointsList[meteoPointListpos].point.z;

    // load meteo point observed data
    if (! meteoPointsDbHandler.loadHourlyData(getCrit3DDate(runDate.addDays(-1)), getCrit3DDate(runDate.addDays(2)), *point))
    {
        logger.writeError ("id: " + QString::fromStdString(point->id) + " meteo point load hourly data error");
        return ERROR_DBPOINT;
    }
    // load meteoGrid forecast data
    /*
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
    */
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

        QDate firstDate = getQDate(point->getMeteoPointHourlyValuesDate(0));
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
        if (myDateIndex > point->nrObsDataDaysH || myDateIndex < 0)
        {
            logger.writeError ("Sunset hour: " + QString::number(myHourSunSetInteger) + " data not available");
            return ERROR_SUNSET;
        }
        QDate newDate = firstDate.addDays(myDateIndex);
        float myTSunSet = point->getMeteoPointValueH(getCrit3DDate(newDate), myHour, 0, airTemperature);
        float myRHSunSet = point->getMeteoPointValueH(getCrit3DDate(newDate), myHour, 0, airRelHumidity);

        if (myTSunSet == NODATA || myRHSunSet == NODATA)
        {
            logger.writeError ("Sunset data not available");
            return ERROR_MISSINGDATA;
        }

        int myDateTmpIndex;
        int myTmpHour;
        QDateTime dateTimeTmp;

        float myIntercept = intercept[paramPos];
        float myParTss = parTss[paramPos];
        float myParRHss = parRHss[paramPos];
        float mySEintercept = SE_intercept[paramPos];
        float mySEparTss = SE_parTss[paramPos];
        float mySEparRHss = SE_parRHss[paramPos];

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
                if (myDateTmpIndex <= point->nrObsDataDaysH && myDateTmpIndex >= 0)
                {
                    QDate dateTmp = firstDate.addDays(myDateTmpIndex);
                    dateTimeTmp = QDateTime(dateTmp,QTime(myTmpHour,0,0));
                    myDateTime.append(QDateTime(runDate.addDays(i),QTime(j,0,0),Qt::UTC));

                    float myT = point->getMeteoPointValueH(getCrit3DDate(dateTmp), myTmpHour, 0, airTemperature);
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
        return ERROR_OUTPUT;
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
            out << myDateTime[i].toString("yyyy-MM-dd hh:mm");
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

int Frost::createPointsCsvFile(QList <QString> idActive)
{
    logger.writeInfo("Create point CSV");
    // check output csv directory
    if (! QDir(csvFilePath).exists())
    {
        QDir().mkdir(csvFilePath);
    }

    QString outputCsvFileName = csvFilePath + "/" + "stations.csv";
    QFile outputFile(outputCsvFileName);

    if (!outputFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        logger.writeError ("Open failure: " + outputCsvFileName);
        return ERROR_OUTPUT;
    }
    else
    {
        logger.writeInfo("Output file: " + outputCsvFileName);
    }

    QString header = "code,name,lat,lon,z";

    QTextStream out(&outputFile);
    out << header << "\n";

    int pointPos;
    for (const auto& id : idActive)
    {
        pointPos = 0;
        for (; pointPos < meteoPointsList.size(); pointPos++)
        {
            if (meteoPointsList[pointPos].id == id.toStdString())
            {
                out << QString::fromStdString(meteoPointsList[pointPos].id) << ",";
                out << QString::fromStdString(meteoPointsList[pointPos].name) << ",";
                out << meteoPointsList[pointPos].latitude << ",";
                out << meteoPointsList[pointPos].longitude << ",";
                out << meteoPointsList[pointPos].point.z << "\n";

                break;
            }
        }
    }

    outputFile.flush();
    outputFile.close();

    return FROSTFORECAST_OK;

}

int Frost::createPointsJsonFile(QList <QString> idActive)
{
    logger.writeInfo("Creating point json...");
    // check output json directory
    if (! QDir(csvFilePath).exists())
    {
        QDir().mkdir(csvFilePath);
    }

    std::vector <QString> fieldNames = {"code","name","lat","lon","z"};
    std::vector <QString> datatype = {"string","string","float","float","float"};
    std::vector <std::vector <QString>> values;
    QString outputJson = csvFilePath + "/" + "stations.json";

    int pointPos;
    for (const auto& id : idActive)
    {
        pointPos = 0;
        for (; pointPos < meteoPointsList.size(); pointPos++)
        {
            if (meteoPointsList[pointPos].id == id.toStdString())
            {
                std::vector <QString> myValues;

                myValues.push_back(QString::fromStdString(meteoPointsList[pointPos].id));
                myValues.push_back(QString::fromStdString(meteoPointsList[pointPos].name));
                myValues.push_back(QString::number(meteoPointsList[pointPos].latitude));
                myValues.push_back(QString::number(meteoPointsList[pointPos].longitude));
                myValues.push_back(QString::number(meteoPointsList[pointPos].point.z));

                values.push_back(myValues);

                break;
            }
        }
    }

    if (writeJson("point", fieldNames, datatype, values, outputJson))
        return FROSTFORECAST_OK;
    else
        return ERROR_OUTPUT;

}


QList<QString> Frost::getIdList() const
{
    return idList;
}

void fitCoolingCoefficient(std::vector <std::vector <float>>& hourly_series, int nrIterations, float tolerance, float minValue, float maxValue, std::vector <float>& coeff)
{
    float myMin, myMax;
    float mySum1, mySum2;
    float myFirstError = 0;
    float mySecondError = 0;;
    int iteration;
    float Tsunset;
    int time;

    for(std::vector <float> hourlyT : hourly_series)
    {

        myMin = minValue;
        myMax = maxValue;
        iteration = 0;
        do {

            mySum1 = 0;
            mySum2 = 0;
            Tsunset = hourlyT[0];
            time = 0;
            for (float T : hourlyT)
            {
                if (! isEqual(T, NODATA))
                {
                    mySum1 += pow((Tsunset - myMin * sqrt(time) - T), 2);
                    mySum2 += pow((Tsunset - myMax * sqrt(time) - T), 2);
                }
                time++;
            }

            myFirstError = sqrt(mySum1);
            mySecondError = sqrt(mySum2);

            if (myFirstError < mySecondError)
                myMax = (myMin + myMax) / 2;
            else
                myMin = (myMin + myMax) / 2;

            iteration++;

        } while (fabs(myFirstError - mySecondError) > tolerance && iteration <= nrIterations);

        coeff.push_back((myMin + myMax) / 2);
    }
}

bool Frost::getRadiativeCoolingHistory(unsigned pos, std::vector<std::vector<float>>& outData, std::vector <std::vector <float>>& sunsetData, std::vector <QDate>& frostDates)
{
    Crit3DMeteoPoint* point = &meteoPointsList[pos];

    // load meteo point observed data
    if (!meteoPointsDbHandler.loadHourlyData(getCrit3DDate(historyDateStart), getCrit3DDate(historyDateEnd), *point))
    {
        logger.writeError ("id: " + QString::fromStdString(point->id) +" meteo point load hourly data error");
        return false;
    }

    TObsDataH* hourlyData = point->getObsDataH();

    const unsigned MIN_DATA = 5;
    Crit3DDate today;
    TsunPosition sunPosition;
    float temperature = 25;
    float pressure =  1013;
    float radAspect = 0;
    float radSlope = 0;
    int hourSunSetLocal;
    int hourSunRiseLocal;
    int hourSunRiseUtc;
    int dateIndexSunRiseUtc;
    QDate dateTmp;
    int h, d;
    std::vector <float> hourlyT;
    float T;
    float minT, maxT;
    bool dataSunsetPresent;
    std::vector <float> ssData;
    float RH_SS;
    bool isSunRise, isSunset;
    int nrValidData;

    //TODO use historyDateStart and historyDateEnd
    for (int i = 0; i < point->nrObsDataDaysH; i++)
    {
        today = hourlyData[i].date;

        if (today.month <= monthFin && today.month >= monthIni)
        {
            if (radiation::computeSunPosition(float(point->longitude), float(point->latitude), gisSettings.timeZone,
                                              today.year, today.month, today.day, 0, 0, 0,
                                              temperature, pressure, radAspect, radSlope, &sunPosition))
            {
                hourSunSetLocal = round(sunPosition.set/3600);
                hourSunRiseLocal = round(sunPosition.rise/3600);
                hourSunRiseUtc = hourSunRiseLocal - gisSettings.timeZone;
                dateIndexSunRiseUtc = i;

                if (hourSunRiseUtc < 1)
                {
                    hourSunRiseUtc += 24;
                    dateIndexSunRiseUtc++;
                }
                else if (hourSunRiseUtc > 24)
                {
                    hourSunRiseUtc -= 24;
                    dateIndexSunRiseUtc--;
                }

                h = hourSunSetLocal - gisSettings.timeZone;
                d = i-1;

                isSunRise = false;
                isSunset = true;
                minT = NODATA;
                maxT = NODATA;
                dataSunsetPresent = false;
                hourlyT.clear();
                nrValidData = 0;

                while (! isSunRise)
                {
                    if (h < 1)
                    {
                         d--;
                         h += 24;
                    }
                    else if (h > 24)
                    {
                         d++;
                         h -= 24;
                    }

                    T = NODATA;
                    if (d <= point->nrObsDataDaysH && d >= 0)
                    {
                        dateTmp = historyDateStart.addDays(d);
                        T = point->getMeteoPointValueH(getCrit3DDate(dateTmp), h, 0, airTemperature);
                    }
                    hourlyT.push_back(T);

                    // sunset vector
                    if (isSunset)
                    {
                        ssData.clear();
                        RH_SS = point->getMeteoPointValueH(getCrit3DDate(dateTmp), h, 0, airRelHumidity);
                        if (! isEqual(T, NODATA) && ! isEqual(RH_SS, NODATA))
                        {
                            ssData.push_back(T);
                            ssData.push_back(RH_SS);
                            dataSunsetPresent = true;
                        }
                    }

                    if (! isEqual(T, NODATA))
                    {
                        nrValidData++;

                        if (isEqual(minT, NODATA))
                            minT = T;
                        else
                            minT = MINVALUE(minT, T);

                        if (isEqual(maxT, NODATA))
                            maxT = T;
                        else
                            maxT = MAXVALUE(maxT, T);
                    }

                    if (h == hourSunRiseUtc && d == dateIndexSunRiseUtc)
                    {
                        isSunRise = true;
                        if (nrValidData/hourlyT.size() >= minPercentage && dataSunsetPresent && minT < thresholdTmin && (maxT - minT >= thresholdTrange))
                        {
                            outData.push_back(hourlyT);
                            sunsetData.push_back(ssData);
                            frostDates.push_back(getQDate(today));
                        }
                    }
                    else
                        h++;

                    isSunset = false;
                }
            }
        }
    }

    return (outData.size() > MIN_DATA);
}

bool Frost::calibrateModel(int idPos)
{
    std::vector <std::vector <float>> outData;
    std::vector <std::vector <float>> sunsetData;
    std::vector <float> outCoeff;
    std::vector <float> weights;
    float regrConst, R2, stdError, regrConstStdError;
    std::vector <float> regrCoeff, regrCoeffStdError;
    std::vector <QDate> frostDates;

    if (getRadiativeCoolingHistory(idPos, outData, sunsetData, frostDates))
    {
        if (logCalibrationName != "")
        {
            QString text = QString::fromStdString(meteoPointsList[idPos].id) + ",";
            QTextStream out(calibrationLog);

            for (int i=0; i<frostDates.size(); i++)
            {
                text += frostDates[i].toString("yyyy-MM-dd");
                if (i < frostDates.size()-1) text += ",";
            }

            out << text + "\n";
        }

        fitCoolingCoefficient(outData, 10000, float(0.001), float(0), float(10), outCoeff);

        for (int j=0; j < sunsetData.size(); j++)
            weights.push_back(1);

        statistics::weightedMultiRegressionLinearWithStats(sunsetData, outCoeff, weights, &regrConst, regrCoeff, false, true, &R2, &stdError, &regrConstStdError, regrCoeffStdError);

        idList.push_back(QString::fromStdString(meteoPointsList[idPos].id));

        intercept.push_back(regrConst);
        parTss.push_back(regrCoeff[0]);
        parRHss.push_back(regrCoeff[1]);
        SE_intercept.push_back(regrConstStdError);
        SE_parTss.push_back(regrCoeffStdError[0]);
        SE_parRHss.push_back(regrCoeffStdError[1]);

        return true;
    }

    return false;
}

QList<Crit3DMeteoPoint> Frost::getMeteoPointsList() const
{
    return meteoPointsList;
}

void Frost::setMeteoPointsList(const QList<Crit3DMeteoPoint> &newMeteoPointsList)
{
    meteoPointsList = newMeteoPointsList;
}

bool Frost::setCalibrationLog()
{
    if (! QDir(path + "log").exists())
    {
        QDir().mkdir(path + "log");
    }

    if (!logCalibrationName.isEmpty())
    {
        calibrationLog = new QFile;

        QString logFileName = path + "log/" + logCalibrationName + ".txt";
        calibrationLog->setFileName(logFileName);
        return calibrationLog->open(QIODevice::WriteOnly | QIODevice::Text);
    }
    else
    {
        return false;
    }
}
