#include "import.h"
#include "radiationDefinitions.h"
#include "radiationSettings.h"
#include "solarRadiation.h"
#include <QSettings>
#include <QDir>
#include <QTextStream>

#include <qdebug.h>

Import::Import()
{

}

void Import::initialize()
{
    path = "";
    projectName = "";
    settingsFileName = "";
    csvFileName = "";
    xmlDbGrid = "";
    meteoVarList.clear();
    isDaily = false;;
    isEnsemble = false;
    isPrecProgressive = false;
    radConversion = false;
}

int Import::readSettings()
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

    QString tempVar = projectSettings->value("var","").toString();
    if (tempVar.isEmpty())
    {
        logger.writeError ("missing meteo variables");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        meteoVarList = tempVar.split(",");
    }

    if (projectSettings->value("isDaily","").toString().isEmpty())
    {
        logger.writeError ("missing isDaily");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        isDaily = projectSettings->value("isDaily","").toBool();
    }

    if (projectSettings->value("isEnsemble","").toString().isEmpty())
    {
        logger.writeError ("missing isEnsemble");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        isEnsemble = projectSettings->value("isEnsemble","").toBool();
    }

    if (projectSettings->value("isPrecipitationProgressive","").toString().isEmpty())
    {
        logger.writeError ("missing isPrecipitationProgressive");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        isPrecProgressive = projectSettings->value("isPrecipitationProgressive","").toBool();
    }

    if (projectSettings->value("radConversion","").toString().isEmpty())
    {
        radConversion = false;
    }
    else
    {
        radConversion = projectSettings->value("radConversion","").toBool();
    }

    projectSettings->endGroup();

    return CSV2DBGRID_OK;
}

int Import::loadMultiTimeValues()
{
    hoursList.clear();
    valuesMap.clear();
    if (isEnsemble)
    {
        logger.writeError ("is ensemble");
        return ERROR_BAD_REQUEST;
    }
    QFile myFile(csvFileName);
    int posLat = 0;
    int posLon = 1;
    int posValue = 2;
    int posHourlyStep = 3;
    int idListIndex = 0;
    int prev_hour = 0;
    int nFields = 4;
    std::string id;
    bool ok;

    if ( !myFile.open(QFile::ReadOnly | QFile::Text) )
    {
        logger.writeError ("csvFileName file does not exist");
        return ERROR_MISSINGFILE;
    }
    else
    {
        QTextStream in(&myFile);
        //skip header
        QString line = in.readLine();
        while (!in.atEnd())
        {
            line = in.readLine();
            QStringList items = line.split(" ");
            items.removeAll({});
            if (items.size()<nFields)
            {
                logger.writeError ("missing field required");
                return ERROR_BAD_REQUEST;
            }
            int hour = items[posHourlyStep].toInt(&ok,10);
            if(!ok)
            {
                // repeated header lat lon value perturbation number
                continue;
            }
            if (!hoursList.contains(hour))
            {
                // save hourly step
                hoursList<<hour;
            }
            if (hour != prev_hour)
            {
                // new hourly-set, reset idListIndex
                prev_hour = hour;
                idListIndex = 0;
            }
            if (isFirstCsv && hour==hoursList[0])
            {
                // make IDList
                double lat = items[posLat].toDouble();
                double lon = items[posLon].toDouble();
                if (!grid.meteoGrid()->getIdFromLatLon(lat,lon,&id))
                {
                    return ERROR_DBGRID;
                }
                if (!grid.meteoGrid()->isActiveMeteoPointFromId(id))
                {
                    id = "-9999";
                }
                IDList<<QString::fromStdString(id);
            }
            if (IDList[idListIndex] != "-9999")
            {
                valuesMap.insert(IDList[idListIndex], items[posValue].toFloat());
            }
            idListIndex ++;
        }
    }
    myFile.close();
    return CSV2DBGRID_OK;
}

int Import::writeMultiTimeValues()
{
    QString errorString;
    if (! grid.openDatabase(&errorString))
    {
        logger.writeError (errorString);
        return ERROR_DBGRID;
    }
    if (meteoVar == noMeteoVar)
    {
        logger.writeError ("invalid MeteoVar");
        return ERROR_BAD_REQUEST;
    }

    QStringList nameParts = QFileInfo(csvFileName).baseName().split("_");
    QString dateStr = nameParts[nameParts.size()-1];
    QDateTime dateTime = QDateTime::fromString(dateStr,"yyyyMMddhh");
    QDate date = dateTime.date();

    QList<float> valueList;
    QList<float> interpolatedValueList;
    QString key;

    QString varname = QString::fromStdString(getMeteoVarName(meteoVar));
    logger.writeInfo("write data: " + varname + "  " + date.toString("yyyy-MM-dd"));

    for (int i=0; i<IDList.size(); i++)
    {
        interpolatedValueList.clear();
        key = IDList[i];
        if ( key != "-9999")
        {
            valueList = valuesMap.values(key);
            if(meteoVar == globalIrradiance || meteoVar == netIrradiance)
            {
                if (radConversion)
                {
                    // Conversion J/m2 to Watt/m2
                    for (int n=1; n<valueList.size(); n++)
                    {
                        if (n==0)
                        {
                            valueList[valueList.size()-1-n] = valueList[valueList.size()-1-n] / (3600);
                        }
                        else
                        {
                            valueList[valueList.size()-1-n] = valueList[valueList.size()-1-n] / (3600*(hoursList[n]-hoursList[n-1]));
                        }
                    }
                }
            }
            if(meteoVar == precipitation)
            {
                // time interpolation (linear)
                for(int j=0; j<hoursList.size(); j++)
                {
                    int myHour = hoursList[j];
                    float myValue = valueList[valueList.size()-1-j]; //revers order

                    if (j==0)
                    {
                        // first data
                        interpolatedValueList << myValue;
                    }
                    else
                    {
                        int myPrevHour = hoursList[j-1];
                        int nHours = myHour - myPrevHour;
                        for (int h = (myPrevHour + 1); h <= myHour; h++)
                        {
                            interpolatedValueList << myValue / nHours;
                        }
                    }
                }
            }
            else
            {
                // time interpolation (linear)
                if(meteoVar != globalIrradiance)
                {
                    for(int j=0; j<hoursList.size(); j++)
                    {
                        int myHour = hoursList[j];
                        float myValue = valueList[valueList.size()-1-j]; //revers order

                        if (j==0)
                        {
                            // first data
                            interpolatedValueList << myValue;
                        }
                        else
                        {
                            int myPrevHour = hoursList[j-1];
                            float myPrevValue = valueList[valueList.size()-j]; //revers order
                            int nHours = myHour - myPrevHour;
                            for (int h = (myPrevHour + 1); h < myHour; h++)
                            {
                                interpolatedValueList << (myPrevValue + ((myValue - myPrevValue) / nHours) * (h - myPrevHour));
                            }
                            interpolatedValueList << myValue;
                        }
                    }
                }
                else
                {
                    // time interpolation (linear) check rad potent
                    for(int j=0; j<hoursList.size(); j++)
                    {
                        /* * */
                        double x;
                        double y;
                        double z;
                        double lat; double lon;
                        grid.meteoGrid()->getXYZFromId(key.toStdString(), &x, &y, &z);
                        grid.meteoGrid()->getLatLonFromId(key.toStdString(), &lat, &lon);
                        TsunPosition mySunPosition;
                        TradPoint myRadPoint;
                        Crit3DRadiationSettings radSettings;
                        gis::Crit3DRasterGrid myDEM;
                        gis::Crit3DPoint myPoint;
                        radSettings.gisSettings = &gisSettings;
                        /*! assign topographic height and coordinates */
                        myRadPoint.x = x;
                        myRadPoint.y = y;
                        myRadPoint.height = z;
                        myRadPoint.lat = lat;
                        myRadPoint.lon = lon;
                        myPoint.utm.x = x;
                        myPoint.utm.y = y;
                        myPoint.z = z;
                        /*! suppose radiometers are horizontal */
                        myRadPoint.aspect = 0.;
                        myRadPoint.slope = 0.;

                        float myLinke = radiation::readLinke(&radSettings, myPoint);
                        float myAlbedo = radiation::readAlbedo(&radSettings, myPoint);
                        float myClearSkyTransmissivity = radSettings.getClearSky();
                        Crit3DTime myTime;

                        int myHour = hoursList[j];
                        float myValue = valueList[valueList.size()-1-j]; //revers order

                        if (j==0)
                        {
                            // first data
                            QDateTime tmp = dateTime.addSecs(myHour*3600);
                            myTime.date.day = tmp.date().day();
                            myTime.date.month = tmp.date().month();
                            myTime.date.year = tmp.date().year();
                            myTime.time = (tmp.time().hour()*3600-1800);
                            radiation::computeRadiationPointRsun(&radSettings, TEMPERATURE_DEFAULT, PRESSURE_SEALEVEL, myTime, myLinke, myAlbedo,
                                                            myClearSkyTransmissivity, myClearSkyTransmissivity, &mySunPosition, &myRadPoint, myDEM);
                            // global rad check
                            if (myRadPoint.global == 0)
                            {
                                interpolatedValueList << 0;
                            }
                            else
                            {
                                interpolatedValueList << myValue;
                            }
                        }
                        else
                        {
                            int myPrevHour = hoursList[j-1];
                            float myPrevValue = valueList[valueList.size()-j]; //revers order
                            int nHours = myHour - myPrevHour;

                            // interpolation data
                            for (int h = (myPrevHour + 1); h < myHour; h++)
                            {
                                QDateTime tmp = dateTime.addSecs(h*3600);
                                myTime.date.day = tmp.date().day();
                                myTime.date.month = tmp.date().month();
                                myTime.date.year = tmp.date().year();
                                myTime.time = (tmp.time().hour()*3600-1800);

                                radiation::computeRadiationPointRsun(&radSettings, TEMPERATURE_DEFAULT, PRESSURE_SEALEVEL, myTime, myLinke, myAlbedo,
                                                                myClearSkyTransmissivity, myClearSkyTransmissivity, &mySunPosition, &myRadPoint, myDEM);
                                // global rad check
                                if (myRadPoint.global == 0)
                                {
                                    interpolatedValueList << 0;
                                }
                                else
                                {
                                    interpolatedValueList << (myPrevValue + ((myValue - myPrevValue) / nHours) * (h - myPrevHour));
                                }
                            }
                            QDateTime tmp = dateTime.addSecs(myHour*3600);
                            myTime.date.day = tmp.date().day();
                            myTime.date.month = tmp.date().month();
                            myTime.date.year = tmp.date().year();
                            myTime.time = (tmp.time().hour()*3600-1800);

                            radiation::computeRadiationPointRsun(&radSettings, TEMPERATURE_DEFAULT, PRESSURE_SEALEVEL, myTime, myLinke, myAlbedo,
                                                            myClearSkyTransmissivity, myClearSkyTransmissivity, &mySunPosition, &myRadPoint, myDEM);

                            if (myRadPoint.global == 0)
                            {
                                interpolatedValueList << 0;
                            }
                            else
                            {
                                interpolatedValueList << myValue;
                            }
                        }
                    }
                }
            }
            if (!grid.saveListHourlyData(&errorString, key, date, meteoVar, interpolatedValueList))
            {
                return ERROR_WRITING_DATA;
            }
        }
    }
    grid.closeDatabase();
    return CSV2DBGRID_OK;
    // TO DO check che gli ID compaiano una sola volta per cella
}

int Import::loadEnsembleDailyValues()
{
    valuesMap.clear();
    if (!isEnsemble)
    {
        logger.writeError ("is not ensemble");
        return ERROR_BAD_REQUEST;
    }
    QFile myFile(csvFileName);
    int posLat = 0;
    int posLon = 1;
    int posValue = 2;
    int posPerturbationNumber = 3;
    int idListIndex = 0;
    int prev_perturbation = 0;
    int nFields = 4;
    std::string id;
    bool ok;

    if ( !myFile.open(QFile::ReadOnly | QFile::Text) )
    {
        logger.writeError ("csvFileName file does not exist");
        return ERROR_MISSINGFILE;
    }
    else
    {
        QTextStream in(&myFile);
        //skip header
        QString line = in.readLine();
        while (!in.atEnd())
        {
            line = in.readLine();
            QStringList items = line.split(" ");
            items.removeAll({});
            if (items.size()<nFields)
            {
                logger.writeError ("is not ensemble, missing field required");
                return ERROR_BAD_REQUEST;
            }
            int perturbation = items[posPerturbationNumber].toInt(&ok,10);
            if(!ok)
            {
                // repeated header lat lon value perturbation number
                continue;
            }
            if (perturbation != prev_perturbation)
            {
                // new ensemble, reset idListIndex
                prev_perturbation = perturbation;
                idListIndex = 0;
            }
            if (isFirstCsv && perturbation==0)
            {
                // make IDList
                double lat = items[posLat].toDouble();
                double lon = items[posLon].toDouble();
                if (!grid.meteoGrid()->getIdFromLatLon(lat,lon,&id))
                {
                    return ERROR_DBGRID;
                }
                if (!grid.meteoGrid()->isActiveMeteoPointFromId(id))
                {
                    id = "-9999";
                }
                IDList<<QString::fromStdString(id);
            }
            if (IDList[idListIndex] != "-9999")
            {
                valuesMap.insert(IDList[idListIndex], items[posValue].toFloat());
            }
            idListIndex ++;
        }
    }
    myFile.close();
    return CSV2DBGRID_OK;
}

int Import::writeEnsembleDailyValues()
{
    QString errorString;
    if (! grid.openDatabase(&errorString))
    {
        logger.writeError (errorString);
        return ERROR_DBGRID;
    }
    if (meteoVar == noMeteoVar)
    {
        logger.writeError ("invalid MeteoVar");
        return ERROR_BAD_REQUEST;
    }

    QStringList nameParts = QFileInfo(csvFileName).baseName().split("_");
    int nDay = nameParts[nameParts.size()-1].toInt();
    nDay = nDay/24-1;
    QString dateStr = nameParts[nameParts.size()-2];
    QDate date = QDateTime::fromString(dateStr,"yyyyMMddhh").date();
    date = date.addDays(nDay);
    QList<float> valueList;
    QString key;

    QString varname = QString::fromStdString(getMeteoVarName(meteoVar));
    logger.writeInfo("write data: " + varname + "  " + date.toString("yyyy-MM-dd"));

    for (int i=0; i<IDList.size(); i++)
    {
        key = IDList[i];
        if ( key != "-9999")
        {
            valueList = valuesMap.values(key);
            if (!grid.saveListDailyDataEnsemble(&errorString, key, date, meteoVar, valueList))
            {
                return ERROR_WRITING_DATA;
            }
        }
    }
    grid.closeDatabase();
    return CSV2DBGRID_OK;
    // TO DO check che gli ID compaiano una sola volta per cella
}

QString Import::getCsvFilePath() const
{
    return csvFilePath;
}

void Import::setIsFirstCsv(bool value)
{
    isFirstCsv = value;
}

QList<QString> Import::getMeteoVar() const
{
    return meteoVarList;
}

void Import::setMeteoVar(const QString &value)
{
    if (isDaily)
    {
        meteoVar = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, value.toStdString());
    }
    else
    {
        meteoVar = getKeyMeteoVarMeteoMap(MapHourlyMeteoVarToString, value.toStdString());
    }
}

bool Import::getIsDaily() const
{
    return isDaily;
}

bool Import::getIsEnsemble() const
{
    return isEnsemble;
}

void Import::setSettingsFileName(const QString &value)
{
    settingsFileName = value;
}

void Import::setCsvFileName(const QString &value)
{
    csvFileName = value;
}
