#ifndef BIAS_H
#define BIAS_H

#include <QString>
#include <QPoint>
#include "logger.h"
#include "dbMeteoGrid.h"
#include "meteo.h"

#define BIASCORRECTION_OK 0
#define ERROR_MISSINGFILE -1
#define ERROR_MISSINGPARAMETERS -2
#define ERROR_DBGRID -3
#define ERROR_DBOUTPUT -4
#define ERROR_AREA -5
#define ERROR_DATE -6
#define ERROR_DBCLIMATE -7

class Bias
{
public:
    Bias();
    Logger logger;

    void initialize();
    int readSettings();
    void setSettingsFileName(const QString &value);
    bool getIsFutureProjection() const;
    void setIsFutureProjection(bool value);
    void matchCells();
    QString getMethod() const;
    QList<QString> getVarList() const;

private:
    gis::Crit3DGisSettings gisSettings;
    QString settingsFileName;
    QString projectName;
    QString path;
    QString refererenceMeteoGrid;
    QString inputMeteoGrid;
    QString outputMeteoGrid;
    QList<QString> varList;
    QString method;
    Crit3DMeteoGridDbHandler refGrid;
    Crit3DMeteoGridDbHandler inputGrid;
    Crit3DMeteoGridDbHandler outputGrid;
    QString errorString;
    QDate firstDate;
    QDate lastDate;
    QString dbClimateName;
    QSqlDatabase dbClimate;
    bool isFutureProjection;
    QList<QPoint> inputCells;
    QList<QPoint> referenceCells;
};

#endif // BIAS_H
