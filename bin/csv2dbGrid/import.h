#ifndef IMPORT_H
#define IMPORT_H

#include <QString>
#include <QMultiMap>
#include "logger.h"
#include "dbMeteoGrid.h"
#include "meteo.h"

#define CSV2DBGRID_OK 0
#define ERROR_MISSINGFILE -1
#define ERROR_MISSINGPARAMETERS -2
#define ERROR_DBGRID -3
#define ERROR_BAD_REQUEST -4
#define ERROR_READING_DATA -5
#define ERROR_WRITING_DATA -6
#define ERROR_TIME_INTERPOLATION -6
#define DEFAULT_NR_STORED_DAYS 30

class Import
{
public:
    Import();
    Logger logger;

    void initialize();
    int readSettings();
    void setSettingsFileName(const QString &value);
    void setCsvFileName(const QString &value);

    int loadMultiTimeValues();
    int loadDailyValues();
    int writeMultiTimeValues();
    int writeDailyValues();
    int loadEnsembleDailyValues();
    int writeEnsembleDailyValues();
    int importDailyValuesMultiVariable(QString &errorStr);

    QString getCsvFilePath() const { return csvFilePath; }
    void setIsFirstCsv(bool value);

    QList<QString> getMeteoVarList() const { return meteoVarList; }
    void setMeteoVar(const QString &value);

    bool getIsDaily() const {return isDaily; }
    bool getIsMultiVariable() const {return isMultiVariable; }
    bool getIsEnsemble() const {return isEnsemble; }

    bool getIsDeleteOldData() const {return isDeleteOldData; }
    void setIsDeleteOldData(bool value);

    int getNrStoredDays() const {return nrStoredDays; }
    void setNrStoredDays(int value);

private:
    gis::Crit3DGisSettings gisSettings;
    QString settingsFileName;
    QString csvFilePath;
    QString csvFileName;
    QString csvSeparator;
    QString projectName;
    QString path;
    QString xmlDbGrid;
    QString gridIdList;
    QList<QString> meteoVarList;
    meteoVariable meteoVar;
    bool isDaily;
    bool isEnsemble;
    bool isHourlyStep;
    bool isPrecProgressive;  // TO DO gestire caso particolare isPrecProgressive = true
    bool isMultiVariable;
    bool radConversion;
    bool isFirstCsv;
    bool isDeleteOldData;
    int nrStoredDays;
    QList<QString> IDList;
    QList<int> hoursList;
    QList<int> dayList;
    QMultiMap<QString, float> valuesMap;
    Crit3DMeteoGridDbHandler grid;

};

#endif // IMPORT_H
