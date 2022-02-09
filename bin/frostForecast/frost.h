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
#define ERROR_DBGRID -3
#define ERROR_DBPOINT -4
#define ERROR_BAD_REQUEST -5
#define ERROR_WRITING_DATA -6
#define ERROR_WRONGDATE -7

class Frost
{
public:
    Frost();
    Logger logger;

    void initialize();
    int readSettings();
    void setSettingsFileName(const QString &value);

private:
    gis::Crit3DGisSettings gisSettings;
    QString settingsFileName;
    QString csvFilePath;
    QString csvFileName;
    QString projectName;
    QString path;
    QString xmlDbGrid;
    QString dbMeteoPointsName;
    Crit3DMeteoGridDbHandler grid;
    Crit3DMeteoPointsDbHandler meteoPointsDbHandler;
    QList<QString> idList;
    QList<QString> intercept;
    QList<QString> parTss;
    QList<QString> parRHss;
    QList<QString> SE_intercept;
    QList<QString> SE_parTss;
    QList<QString> SE_parRHss;
};

#endif // FROST_H
