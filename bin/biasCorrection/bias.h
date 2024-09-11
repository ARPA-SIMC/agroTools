#ifndef BIAS_H
#define BIAS_H

#include "logger.h"
#include "dbMeteoGrid.h"
#include "meteo.h"

#include <QPoint>
#include <QString>

#define BIASCORRECTION_OK 0
#define ERROR_MISSINGFILE -1
#define ERROR_MISSINGPARAMETERS -2
#define ERROR_DBGRID -3
#define ERROR_DBOUTPUT -4
#define ERROR_AREA -5
#define ERROR_DATE -6
#define ERROR_DBCLIMATE -7
#define ERROR_SAVINGPARAM -8
#define ERROR_GETTINGPARAM -9
#define ERROR_SAVINGVALUES -10

class Bias
{
public:
    Bias();
    Logger logger;

    void initialize();

    void setSettingsFileName(const QString &value)
    { settingsFileName = value; }

    bool getIsDebias() const
    { return isDebias; }

    void setIsDebias(bool value)
    { isDebias = value; }

    QList<QString> getVarList() const
    { return varList; }

    int readClimateSettings();
    int readDebiasSettings();
    void matchCells();

    int computeMonthlyDistribution(const QString &variable);
    int numericalDataReconstruction(const QString &variable);

    int saveDistributionParam(const QString& idCell, const QString &variable, const std::vector<double> &monthlyPar1Input,
                              const std::vector<double> &monthlyPar2Input, const std::vector<double> &monthlyPar3Input,
                              const std::vector<double> &monthlyPar1Ref, const std::vector<double> &monthlyPar2Ref,
                              const std::vector<double> &monthlyPar3Ref, bool deletePreviousData);

    int getDistributionParam(const QString& idCell, const QString &variable, std::vector<double> &monthlyPar1Input,
                             std::vector<double> &monthlyPar2Input, std::vector<double> &monthlyPar3Input,
                             std::vector<double> &monthlyPar1Ref,std::vector<double> &monthlyPar2Ref,
                             std::vector<double> &monthlyPar3Ref);

private:
    bool isDebias;

    gis::Crit3DGisSettings gisSettings;
    Crit3DMeteoSettings meteoSettings;

    QString settingsFileName;
    QString projectName;
    QString path;
    QString refererenceMeteoGrid;
    QString inputMeteoGrid;
    QString outputMeteoGrid;
    QString errorString;

    QList<QString> varList;

    Crit3DMeteoGridDbHandler refGrid;
    Crit3DMeteoGridDbHandler inputGrid;
    Crit3DMeteoGridDbHandler outputGrid;

    QString dbClimateName;
    QSqlDatabase dbClimate;
    QDate firstDate;
    QDate lastDate;

    QList<QPoint> inputCells;
    QList<QPoint> referenceCells;
};

#endif // BIAS_H
