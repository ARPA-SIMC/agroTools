#ifndef FROST_H
#define FROST_H

#include <QString>
#include "logger.h"
#include "dbMeteoGrid.h"
#include "dbMeteoPointsHandler.h"
#include "meteo.h"

#define FROSTFORECAST_OK 0
#define ERROR_MISSINGFILE -1
#define ERROR_MISSINGPARAMETERS -2
//#define ERROR_DBGRID -3
#define ERROR_DBPOINT -4
#define ERROR_BAD_REQUEST -5
#define ERROR_WRITING_DATA -6
#define ERROR_WRONGDATE -7
#define ERROR_WRONGPARAM -8
#define ERROR_DBPOINTSDOWNLOAD -9
#define ERROR_SUNSET -10
#define ERROR_MISSINGDATA -11
#define ERROR_MPDATANOTAVAILABLE -11
#define ERROR_OUTPUT -13

class Frost
{
public:
    Frost();
    Logger logger;

    void initialize();
    void initializeFrostParam();
    int readSettings();
    int readParameters();
    void saveParameters();
    void setSettingsFileName(const QString &value);
    int downloadMeteoPointsData();
    void setRunDate(const QDate &value);
    int getForecastData(int paramPos);
    float coeffReuter(float a0, float a1, float a2, float t, float RH);
    float t_Reuter(float d, float deltaTime, float tIni);
    int createCsvFile(QString id);
    int createPointsCsvFile(QList <QString> idActive);
    int createPointsJsonFile(QList <QString> idActive);

    QList<QString> getIdList() const;

    bool getRadiativeCoolingHistory(unsigned pos, std::vector<std::vector<float>>& outData, std::vector <std::vector <float>>& sunsetData);
    bool calibrateModel(int idPos);

    QList<Crit3DMeteoPoint> getMeteoPointsList() const;
    void setMeteoPointsList(const QList<Crit3DMeteoPoint> &newMeteoPointsList);

private:
    gis::Crit3DGisSettings gisSettings;
    QString settingsFileName;
    QDate runDate;
    QString csvFilePath;
    QString csvFileName;
    QString projectName;
    QString path;
    //QString xmlDbGrid;
    QString dbMeteoPointsName;
    //Crit3DMeteoGridDbHandler grid;
    Crit3DMeteoPointsDbHandler meteoPointsDbHandler;
    QList<Crit3DMeteoPoint> meteoPointsList;
    QStringList idList;
    QStringList varList;

    std::vector <float> intercept;
    std::vector <float> parTss;
    std::vector <float> parRHss;
    std::vector <float> SE_intercept;
    std::vector <float> SE_parTss;
    std::vector <float> SE_parRHss;

    int indexSunSet;
    int indexSunRise;
    QString errorString;

    //calibration parameters
    int monthIni;
    int monthFin;
    float thresholdTmin;
    float thresholdTrange;

    QList<float> myForecast;
    QList<float> myForecastMin;
    QList<float> myForecastMax;
    QList<float> myObsData;
    QList<QDateTime> myDate;
};

#endif // FROST_H
