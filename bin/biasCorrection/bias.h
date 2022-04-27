#ifndef BIAS_H
#define BIAS_H

#include <QString>
#include "logger.h"
#include "dbMeteoGrid.h"
#include "meteo.h"

#define BIASCORRECTION_OK 0
#define ERROR_MISSINGFILE -1
#define ERROR_MISSINGPARAMETERS -2
#define ERROR_DBGRID -3
#define ERROR_DBOUTPUT -4

class Bias
{
public:
    Bias();
    Logger logger;

    void initialize();
    int readSettings();
    void setSettingsFileName(const QString &value);

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
};

#endif // BIAS_H
