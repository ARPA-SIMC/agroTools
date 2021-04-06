#ifndef IMPORT_H
#define IMPORT_H

#include <QString>
#include <QMultiMap>
#include "logger.h"
#include "dbMeteoGrid.h"

#define CSV2DBGRID_OK 0
#define ERROR_MISSINGFILE -1
#define ERROR_MISSINGPARAMETERS -2
#define ERROR_DBGRID -3
#define ERROR_BAD_REQUEST -4

class Import
{
public:
    Import();
    Logger logger;

    void initialize();
    int readSettings();
    void setSettingsFileName(const QString &value);
    void setCsvFileName(const QString &value);
    //int loadIDList();
    //int loadValues();
    int loadEnsembleValues();
    QString getCsvFilePath() const;
    void setIsFirstCsv(bool value);

private:
    QString settingsFileName;
    QString csvFilePath;
    QString csvFileName;
    QString projectName;
    QString path;
    QString xmlDbGrid;
    QString gridIdList;
    QList<QString> meteoVar;
    bool isDaily;
    bool isEnsemble;
    bool precIsProgressive;
    bool isFirstCsv;
    QList<QString> IDList;
    QMultiMap<QString, float> valuesMap;
    Crit3DMeteoGridDbHandler grid;

};

#endif // IMPORT_H
