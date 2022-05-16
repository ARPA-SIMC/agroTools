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
#define ERROR_SAVINGPARAM -8
#define ERROR_GETTINGPARAM -9

class Bias
{
public:
    Bias();
    Logger logger;

    void initialize();
    int readReferenceSettings();
    int readDebiasSettings();
    void setSettingsFileName(const QString &value);
    bool getIsFutureProjection() const;
    void setIsFutureProjection(bool value);
    void matchCells();
    QString getMethod() const;
    QList<QString> getVarList() const;
    int computeMonthlyDistribution(QString variable);
    int numericalDataReconstruction(QString variable);
    int saveDistributionParam(QString idCell, QString variable, std::vector<double> monthlyPar1Input, std::vector<double> monthlyPar2Input, std::vector<double> monthlyPar3Input,
                              std::vector<double> monthlyPar1Ref,std::vector<double> monthlyPar2Ref, std::vector<double> monthlyPar3Ref, bool deletePreviousData);
    int getDistributionParam(QString idCell, QString variable, std::vector<double> &monthlyPar1Input, std::vector<double> &monthlyPar2Input, std::vector<double> &monthlyPar3Input,
                                    std::vector<double> &monthlyPar1Ref,std::vector<double> &monthlyPar2Ref, std::vector<double> &monthlyPar3Ref);

private:
    gis::Crit3DGisSettings gisSettings;
    Crit3DMeteoSettings meteoSettings;
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
