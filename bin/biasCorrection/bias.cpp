#include "bias.h"
#include "utilities.h"
#include "basicMath.h"
#include "gammaFunction.h"
#include <QSettings>
#include <QDir>
#include <QTextStream>
#include <QtSql>

Bias::Bias()
{

}

void Bias::initialize()
{
    path = "";
    projectName = "";
    settingsFileName = "";
    refererenceMeteoGrid = "";
    inputMeteoGrid = "";
    outputMeteoGrid = "";
    varList.clear();
    errorString = "";
    dbClimateName = "";
    inputCells.clear();
    referenceCells.clear();
}

void Bias::setSettingsFileName(const QString &value)
{
    settingsFileName = value;
}

bool Bias::getIsDebias() const
{
    return isDebias;
}

void Bias::setIsDebias(bool value)
{
    isDebias = value;
}


QList<QString> Bias::getVarList() const
{
    return varList;
}

int Bias::readReferenceSettings()
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
    projectSettings->setValue("utm_zone", gisSettings.utmZone);
    projectSettings->setValue("time_zone", gisSettings.timeZone);
    projectSettings->setValue("is_utc", gisSettings.isUTC);
    projectSettings->endGroup();

    // PROJECT
    projectSettings->beginGroup("project");
    projectName = projectSettings->value("name","").toString();

    refererenceMeteoGrid = projectSettings->value("refererenceMeteoGrid","").toString();
    if (refererenceMeteoGrid.isEmpty())
    {
        logger.writeError ("missing xml DB refererenceMeteoGrid");
        return ERROR_MISSINGPARAMETERS;
    }
    if (refererenceMeteoGrid.left(1) == ".")
    {
        refererenceMeteoGrid = path + QDir::cleanPath(refererenceMeteoGrid);
    }

    // open reference grid
    if (! refGrid.parseXMLGrid(refererenceMeteoGrid, &errorString))
    {
        logger.writeError (errorString);
        return ERROR_DBGRID;
    }

    if (! refGrid.openDatabase(&errorString, "refGrid"))
    {
        logger.writeError (errorString);
        return ERROR_DBGRID;
    }

    if (! refGrid.loadCellProperties(&errorString))
    {
        logger.writeError (errorString);
        refGrid.closeDatabase();
        return ERROR_DBGRID;
    }

    inputMeteoGrid = projectSettings->value("inputMeteoGrid","").toString();
    if (inputMeteoGrid.isEmpty())
    {
        logger.writeError ("missing xml DB inputMeteoGrid");
        refGrid.closeDatabase();
        return ERROR_MISSINGPARAMETERS;
    }
    if (inputMeteoGrid.left(1) == ".")
    {
        inputMeteoGrid = path + QDir::cleanPath(inputMeteoGrid);
    }

    // open input grid
    if (! inputGrid.parseXMLGrid(inputMeteoGrid, &errorString))
    {
        logger.writeError (errorString);
        refGrid.closeDatabase();
        return ERROR_DBGRID;
    }

    if (! inputGrid.openDatabase(&errorString, "inputGrid"))
    {
        logger.writeError (errorString);
        refGrid.closeDatabase();
        return ERROR_DBGRID;
    }

    if (! inputGrid.loadCellProperties(&errorString))
    {
        logger.writeError (errorString);
        inputGrid.closeDatabase();
        refGrid.closeDatabase();
        return ERROR_DBGRID;
    }

    // check input and reference area
    double refArea = (refGrid.gridStructure().header().nrRows*refGrid.gridStructure().header().dy)*(refGrid.gridStructure().header().nrCols*refGrid.gridStructure().header().dx);
    double inputArea = (inputGrid.gridStructure().header().nrRows*inputGrid.gridStructure().header().dy)*(inputGrid.gridStructure().header().nrCols*inputGrid.gridStructure().header().dx);
    if (refArea < inputArea)
    {
        logger.writeError ("reference Grid area should be >= input Grid area");
        inputGrid.closeDatabase();
        refGrid.closeDatabase();
        return ERROR_AREA;
    }

    outputMeteoGrid = projectSettings->value("outputMeteoGrid","").toString();

    if (outputMeteoGrid.left(1) == ".")
    {
        outputMeteoGrid = path + QDir::cleanPath(outputMeteoGrid);
    }

    // var
    QString varTemp = projectSettings->value("var","").toString();
    if (varTemp.isEmpty())
    {
        logger.writeError ("missing var");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        varList = varTemp.split(",");
    }

    // firstDate
    QString firstDateStr = projectSettings->value("firstDate","").toString();
    firstDate = QDate::fromString(firstDateStr,"yyyy-MM-dd");
    if (!firstDate.isValid())
    {
        logger.writeError ("missing or invalid first date");
        return ERROR_MISSINGPARAMETERS;
    }

    // lastDate
    QString lastDateStr = projectSettings->value("lastDate","").toString();
    lastDate = QDate::fromString(lastDateStr,"yyyy-MM-dd");
    if (!lastDate.isValid())
    {
        logger.writeError ("missing or invalid last date");
        return ERROR_MISSINGPARAMETERS;
    }

    logger.writeInfo("first computation date: " + firstDate.toString());
    logger.writeInfo("last computation date: " + lastDate.toString());

    // check dates
    if (!refGrid.updateGridDate(&errorString))
    {
        logger.writeError("refGrid updateGridDate: " + errorString);
        return ERROR_DATE;
    }
    logger.writeInfo("refGrid.firstDate(): " + refGrid.firstDate().toString());
    logger.writeInfo("refGrid.lastDate(): " + refGrid.lastDate().toString());
    if (refGrid.firstDate() > firstDate || refGrid.lastDate() < lastDate)
    {
        logger.writeError ("firstDate-lastDate interval not included in reference grid");
        return ERROR_DATE;
    }
    if (!inputGrid.updateGridDate(&errorString))
    {
        logger.writeError("inputGrid updateGridDate: " + errorString);
        return ERROR_DATE;
    }
    logger.writeInfo("inputGrid.firstDate(): " + inputGrid.firstDate().toString());
    logger.writeInfo("inputGrid.lastDate(): " + inputGrid.lastDate().toString());
    if (inputGrid.firstDate() > firstDate || inputGrid.lastDate() < lastDate)
    {
        logger.writeError ("firstDate-lastDate interval not included in input grid");
        return ERROR_DATE;
    }

    // db Climate
    dbClimateName = projectSettings->value("dbClimate","").toString();
    if (dbClimateName.isEmpty())
    {
        logger.writeError ("missing xml DB inputMeteoGrid");
        refGrid.closeDatabase();
        inputGrid.closeDatabase();
        return ERROR_MISSINGPARAMETERS;
    }
    if (dbClimateName.left(1) == ".")
    {
        dbClimateName = path + QDir::cleanPath(dbClimateName);
    }

    // create an empty dbClimate, if exists overwrite.
    QFile dbFile(dbClimateName);
    if (dbFile.exists())
    {
        dbFile.close();
        dbFile.setPermissions(QFile::ReadOther | QFile::WriteOther);
        if (! dbFile.remove())
        {
            logger.writeError ("Remove file failed: " + dbClimateName + "\n" + dbFile.errorString());
            refGrid.closeDatabase();
            inputGrid.closeDatabase();
            return ERROR_DBCLIMATE;
        }
    }
    dbClimate = QSqlDatabase::addDatabase("QSQLITE", QUuid::createUuid().toString());
    dbClimate.setDatabaseName(dbClimateName);
    if (!dbClimate.open())
    {
        logger.writeError ("Problem opening: " + dbClimateName + dbClimate.lastError().text()+"\n");
        refGrid.closeDatabase();
        inputGrid.closeDatabase();
        return ERROR_DBCLIMATE;
    }

    projectSettings->endGroup();


    return BIASCORRECTION_OK;
}

int Bias::readDebiasSettings()
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
    projectSettings->setValue("utm_zone", gisSettings.utmZone);
    projectSettings->setValue("time_zone", gisSettings.timeZone);
    projectSettings->setValue("is_utc", gisSettings.isUTC);
    projectSettings->endGroup();

    // PROJECT
    projectSettings->beginGroup("project");
    projectName = projectSettings->value("name","").toString();

    inputMeteoGrid = projectSettings->value("inputMeteoGrid","").toString();
    if (inputMeteoGrid.isEmpty())
    {
        logger.writeError ("missing xml DB inputMeteoGrid");
        return ERROR_MISSINGPARAMETERS;
    }
    if (inputMeteoGrid.left(1) == ".")
    {
        inputMeteoGrid = path + QDir::cleanPath(inputMeteoGrid);
    }

    // open input grid
    if (! inputGrid.parseXMLGrid(inputMeteoGrid, &errorString))
    {
        logger.writeError (errorString);
        return ERROR_DBGRID;
    }

    if (! inputGrid.openDatabase(&errorString, "inputGrid"))
    {
        logger.writeError (errorString);
        return ERROR_DBGRID;
    }

    if (! inputGrid.loadCellProperties(&errorString))
    {
        logger.writeError (errorString);
        inputGrid.closeDatabase();
        return ERROR_DBGRID;
    }

    outputMeteoGrid = projectSettings->value("outputMeteoGrid","").toString();

    if (outputMeteoGrid.left(1) == ".")
    {
        outputMeteoGrid = path + QDir::cleanPath(outputMeteoGrid);
    }

    if (outputMeteoGrid.isEmpty() || !QFile::exists(outputMeteoGrid))
    {
        // create outputMeteoGrid xml and db

        logger.writeInfo("outputMeteoGrid xml does not exist, copy inputMeteoGrid db structure");
        if (outputMeteoGrid.isEmpty())
        {
            outputMeteoGrid = QDir(QFileInfo(inputMeteoGrid).absolutePath()).filePath(QFileInfo(inputMeteoGrid).baseName()) + "_debias.xml";
        }
        QFile::copy(inputMeteoGrid, outputMeteoGrid);
        // modify db name
        QFile myFile(outputMeteoGrid);
        if (!myFile.open(QIODevice::ReadOnly))
        {
            errorString = "Open XML failed:\n" + outputMeteoGrid + "\n" + myFile.errorString();
            logger.writeError (errorString);
            return ERROR_DBOUTPUT;
        }
        QDomDocument document;
        document.setContent(&myFile);
        QDomElement root = document.documentElement();
        myFile.close();

        // make changes
        QDomNode SettingsNode = root.namedItem("connection");
        QDomNode name = SettingsNode.namedItem("name");
        QString newDbName = inputGrid.db().databaseName()+"_debias";
        name.firstChild().setNodeValue(newDbName);

        //Then open the file for overwrite, write all content, close file.

        if(!myFile.open(QIODevice::WriteOnly  | QIODevice::Text))
        {
            logger.writeError (errorString);
            return ERROR_DBOUTPUT;
        }

        QTextStream output(&myFile);
        output << document.toString();
        myFile.close();

        // new db
        outputGrid.meteoGrid()->setGisSettings(this->gisSettings);
        if (! outputGrid.parseXMLGrid(outputMeteoGrid, &errorString))
        {
            return ERROR_DBOUTPUT;
        }

        if (! outputGrid.newDatabase(&errorString, "outputGrid"))
        {
            return ERROR_DBOUTPUT;
        }

        if (! outputGrid.newCellProperties(&errorString))
        {
            return ERROR_DBOUTPUT;
        }

        Crit3DMeteoGridStructure structure = outputGrid.meteoGrid()->gridStructure();

        if (! outputGrid.writeCellProperties(&errorString, structure.nrRow(), structure.nrCol()))
        {
            return ERROR_DBOUTPUT;
        }

        if (! outputGrid.meteoGrid()->createRasterGrid())
        {
            return ERROR_DBOUTPUT;
        }

    }
    else
    {
        // open output grid
        if (! outputGrid.parseXMLGrid(outputMeteoGrid, &errorString))
        {
            logger.writeError (errorString);
            return ERROR_DBGRID;
        }

        if (outputGrid.openDatabase(&errorString, "outputGrid"))
        {
            // database exists, open it
            if (! outputGrid.loadCellProperties(&errorString))
            {
                logger.writeError (errorString);
                outputGrid.closeDatabase();
                return ERROR_DBGRID;
            }
        }
        else
        {
            // maybe database does not exist, try to create it
            if (! outputGrid.newDatabase(&errorString, "outputGrid"))
            {
                return ERROR_DBOUTPUT;
            }

            if (! outputGrid.newCellProperties(&errorString))
            {
                return ERROR_DBOUTPUT;
            }

            Crit3DMeteoGridStructure structure = outputGrid.meteoGrid()->gridStructure();

            if (! outputGrid.writeCellProperties(&errorString, structure.nrRow(), structure.nrCol()))
            {
                return ERROR_DBOUTPUT;
            }

            if (! outputGrid.meteoGrid()->createRasterGrid())
            {
                return ERROR_DBOUTPUT;
            }
        }
    }
    // var
    QString varTemp = projectSettings->value("var","").toString();
    if (varTemp.isEmpty())
    {
        logger.writeError ("missing var");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        varList = varTemp.split(",");
    }

    // firstDate
    QString firstDateStr = projectSettings->value("firstDate","").toString();
    firstDate = QDate::fromString(firstDateStr,"yyyy-MM-dd");
    if (!firstDate.isValid())
    {
        logger.writeError ("missing or invalid first date");
        return ERROR_MISSINGPARAMETERS;
    }
    // lastDate
    QString lastDateStr = projectSettings->value("lastDate","").toString();
    lastDate = QDate::fromString(lastDateStr,"yyyy-MM-dd");
    if (!lastDate.isValid())
    {
        logger.writeError ("missing or invalid last date");
        return ERROR_MISSINGPARAMETERS;
    }

    if (!inputGrid.updateGridDate(&errorString))
    {
        logger.writeError("inputGrid updateGridDate: " + errorString);
        return ERROR_DATE;
    }
    logger.writeInfo("inputGrid.firstDate(): " + inputGrid.firstDate().toString());
    logger.writeInfo("inputGrid.lastDate(): " + inputGrid.lastDate().toString());
    if (inputGrid.firstDate() > firstDate || inputGrid.lastDate() < lastDate)
    {
        logger.writeError ("firstDate-lastDate interval not included in input grid");
        return ERROR_DATE;
    }

    // db Climate
    dbClimateName = projectSettings->value("dbClimate","").toString();
    if (dbClimateName.isEmpty())
    {
        logger.writeError ("missing xml DB inputMeteoGrid");
        inputGrid.closeDatabase();
        return ERROR_MISSINGPARAMETERS;
    }
    if (dbClimateName.left(1) == ".")
    {
        dbClimateName = path + QDir::cleanPath(dbClimateName);
    }
    if (!QFile::exists(dbClimateName))
    {
        logger.writeError ("Compute reference before debias.");
        inputGrid.closeDatabase();
        return ERROR_DBCLIMATE;
    }
    dbClimate = QSqlDatabase::addDatabase("QSQLITE", QUuid::createUuid().toString());
    dbClimate.setDatabaseName(dbClimateName);
    if (!dbClimate.open())
    {
        logger.writeError ("Problem opening: " + dbClimateName + dbClimate.lastError().text()+"\n");
        inputGrid.closeDatabase();
        return ERROR_DBCLIMATE;
    }

    projectSettings->endGroup();

    return BIASCORRECTION_OK;
}

void Bias::matchCells()
{
    if (inputGrid.gridStructure().header().dx > refGrid.gridStructure().header().dx || inputGrid.gridStructure().header().dy > refGrid.gridStructure().header().dy)
    {
        // TO DO
    }
    else
    {
        for (int row = 0; row<inputGrid.gridStructure().header().nrRows; row++)
        {
            for (int col = 0; col<inputGrid.gridStructure().header().nrCols; col++)
            {
                std::string id;
                if (inputGrid.meteoGrid()->getMeteoPointActiveId(row, col, &id))
                {
                    QPoint inputPoint(row,col);
                    inputCells << inputPoint;
                    if (!inputGrid.gridStructure().isUTM() && !refGrid.gridStructure().isUTM()) // both lat lon
                    {
                        double lat;
                        double lon;
                        inputGrid.meteoGrid()->getLatLonFromId(id,&lat,&lon);
                        int refRow;
                        int refCol;
                        gis::getGridRowColFromXY(refGrid.gridStructure().header(), lon, lat, &refRow, &refCol);
                        QPoint refPoint(refRow,refCol);
                        referenceCells << refPoint;
                    }
                    else if (!inputGrid.gridStructure().isUTM() && refGrid.gridStructure().isUTM())
                    {
                        // TO DO
                    }
                    else if (inputGrid.gridStructure().isUTM() && !refGrid.gridStructure().isUTM())
                    {
                        // TO DO
                    }
                    else if (inputGrid.gridStructure().isUTM() && refGrid.gridStructure().isUTM())
                    {
                        // TO DO
                    }
                }
            }
        }
    }
}

int Bias::computeMonthlyDistribution(QString variable)
{
    meteoVariable var = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, variable.toStdString());
    bool deletePreviousData = true;
    int res = 0;
    for (int i = 0; i<inputCells.size(); i++)
    {
        int row = inputCells[i].x();
        int col = inputCells[i].y();
        int refRow = referenceCells[i].x();
        int refCol = referenceCells[i].y();
        std::string idInput;
        std::string refInput;
        inputGrid.meteoGrid()->getMeteoPointActiveId(row, col, &idInput);  // store id
        refGrid.meteoGrid()->getMeteoPointActiveId(refRow, refCol, &refInput);  // store id
        if (!inputGrid.gridStructure().isFixedFields())
        {
            if (!inputGrid.loadGridDailyData(errorString, QString::fromStdString(idInput), firstDate, lastDate))
            {
                continue;
            }
        }
        else
        {
            if (!inputGrid.loadGridDailyDataFixedFields(errorString, QString::fromStdString(idInput), firstDate, lastDate))
            {
                continue;
            }
        }
        if (!refGrid.gridStructure().isFixedFields())
        {
            refGrid.loadGridDailyData(errorString, QString::fromStdString(refInput), firstDate, lastDate);
        }
        else
        {
            refGrid.loadGridDailyDataFixedFields(errorString, QString::fromStdString(refInput), firstDate, lastDate);
        }
        Crit3DMeteoPoint mpInput = inputGrid.meteoGrid()->meteoPoint(row,col);
        Crit3DMeteoPoint mpRef = refGrid.meteoGrid()->meteoPoint(refRow,refCol);
        std::vector<double> monthlyPar1Input;
        std::vector<double> monthlyPar2Input;
        std::vector<double> monthlyPar3Input;
        std::vector<float> seriesInput;

        std::vector<double> monthlyPar1Ref;
        std::vector<double> monthlyPar2Ref;
        std::vector<double> monthlyPar3Ref;
        std::vector<float> seriesRef;

        QDate startPeriod;
        QDate endPeriod;
        QDate tempDate;
        for (int myMonth = 1; myMonth <= 12; myMonth ++)
        {
            seriesInput.clear();
            seriesRef.clear();
            for (int myYear = firstDate.year(); myYear <= lastDate.year(); myYear++)
            {
                startPeriod.setDate(myYear,myMonth,1);
                endPeriod.setDate(myYear,myMonth,startPeriod.daysInMonth());
                tempDate = startPeriod;
                while (tempDate <= endPeriod)
                {
                    // input Grid
                    float myDailyValue = mpInput.getMeteoPointValueD(getCrit3DDate(tempDate), var, &meteoSettings);
                    if (myDailyValue != NODATA)
                    {
                        if (var == dailyPrecipitation && myDailyValue < meteoSettings.getRainfallThreshold())
                        {
                            myDailyValue = 0;
                        }
                        seriesInput.push_back(myDailyValue);
                    }
                    // ref Grid
                    myDailyValue = mpRef.getMeteoPointValueD(getCrit3DDate(tempDate), var, &meteoSettings);
                    if (myDailyValue != NODATA)
                    {
                        if (var == dailyPrecipitation && myDailyValue < meteoSettings.getRainfallThreshold())
                        {
                            myDailyValue = 0;
                        }
                        seriesRef.push_back(myDailyValue);
                    }
                    tempDate = tempDate.addDays(1);
                }
            }
            if (seriesInput.size() != 0)
            {
                int nrValues = int(seriesInput.size());
                if (var == dailyAirTemperatureMax || var == dailyAirTemperatureMin)
                {
                    monthlyPar2Input.push_back(statistics::standardDeviation(seriesInput, nrValues));
                    monthlyPar1Input.push_back(sorting::percentile(seriesInput, nrValues,50,true));
                    monthlyPar3Input.push_back(NODATA);
                }
                else if (var == dailyPrecipitation)
                {
                    double beta;
                    double alpha;
                    double pZero;
                    generalizedGammaFitting(seriesInput, nrValues, &beta, &alpha,  &pZero);
                    monthlyPar1Input.push_back(beta);
                    monthlyPar2Input.push_back(alpha);
                    monthlyPar3Input.push_back(pZero);
                }
            }
            else
            {
                monthlyPar1Input.push_back(NODATA);
                monthlyPar2Input.push_back(NODATA);
                monthlyPar3Input.push_back(NODATA);
            }

            if (seriesRef.size() != 0)
            {
                int nrValues = int(seriesRef.size());
                if (var == dailyAirTemperatureMax || var == dailyAirTemperatureMin)
                {
                    monthlyPar2Ref.push_back(statistics::standardDeviation(seriesRef, nrValues));
                    monthlyPar1Ref.push_back(sorting::percentile(seriesRef, nrValues,50,true));
                    monthlyPar3Ref.push_back(NODATA);
                }
                else if (var == dailyPrecipitation)
                {
                    double beta;
                    double alpha;
                    double pZero;
                    generalizedGammaFitting(seriesRef, nrValues, &beta, &alpha,  &pZero);
                    monthlyPar1Ref.push_back(beta);
                    monthlyPar2Ref.push_back(alpha);
                    monthlyPar3Ref.push_back(pZero);
                }
            }
            else
            {
                monthlyPar1Ref.push_back(NODATA);
                monthlyPar2Ref.push_back(NODATA);
                monthlyPar3Ref.push_back(NODATA);
            }
        }
        // save values
        res = res + saveDistributionParam(QString::fromStdString(idInput),variable,monthlyPar1Input,monthlyPar2Input,monthlyPar3Input,monthlyPar1Ref,monthlyPar2Ref,monthlyPar3Ref,deletePreviousData);
    }
    if (res == 0)
    {
        return BIASCORRECTION_OK;
    }
    else
    {
        return ERROR_SAVINGPARAM;
    }
}

int Bias::saveDistributionParam(QString idCell, QString variable, std::vector<double> monthlyPar1Input, std::vector<double> monthlyPar2Input, std::vector<double> monthlyPar3Input,
                                std::vector<double> monthlyPar1Ref,std::vector<double> monthlyPar2Ref, std::vector<double> monthlyPar3Ref, bool deletePreviousData)
{
    QString queryStr;
    if (deletePreviousData)
    {
        queryStr = "DROP TABLE IF EXISTS " + idCell;
        dbClimate.exec(queryStr);
    }

    queryStr = QString("CREATE TABLE IF NOT EXISTS `%1`"
                                "(var TEXT(20), month INTEGER, par1 REAL, par2 REAL, par3 REAL, reference_par1 REAL, reference_par2 REAL, reference_par3 REAL, PRIMARY KEY(var, month))").arg(idCell);
    QSqlQuery qry(dbClimate);
    qry.prepare(queryStr);
    if (!qry.exec())
    {
        logger.writeError ("Error in create table: " + idCell + qry.lastError().text()+"\n");
        return ERROR_SAVINGPARAM;
    }

    queryStr = QString(("INSERT OR REPLACE INTO `%1`"
                                " VALUES ")).arg(idCell);

    for (int i = 0; i<monthlyPar1Input.size(); i++)
    {
        queryStr += "('"+variable+"',"+QString::number(i+1)+","+QString::number(monthlyPar1Input[i], 'f', 2)+","+QString::number(monthlyPar2Input[i], 'f', 2)+","+QString::number(monthlyPar3Input[i], 'f', 2)+
                ","+QString::number(monthlyPar1Ref[i], 'f', 2)+","+QString::number(monthlyPar2Ref[i], 'f', 2)+","+QString::number(monthlyPar3Ref[i], 'f', 2)+"),";
    }
    queryStr.chop(1); // remove last ,

    if( !qry.exec(queryStr) )
    {
        logger.writeError ("Error in execute query: " + qry.lastError().text()+"\n");
        return ERROR_SAVINGPARAM;
    }
    else
        return BIASCORRECTION_OK;
}

int Bias::getDistributionParam(QString idCell, QString variable, std::vector<double> &monthlyPar1Input, std::vector<double> &monthlyPar2Input, std::vector<double> &monthlyPar3Input,
                                std::vector<double> &monthlyPar1Ref,std::vector<double> &monthlyPar2Ref, std::vector<double> &monthlyPar3Ref)
{
    QSqlQuery qry(dbClimate);

    QString statement = QString("SELECT * FROM `%1`").arg(idCell);
    qry.prepare( statement + " WHERE var = :var " );
    qry.bindValue(":var", variable);
    if( !qry.exec() )
    {
        logger.writeError ("Error in select table: " + idCell + qry.lastError().text()+"\n");
        return ERROR_GETTINGPARAM;
    }
    else
    {
        int month;
        double par;
        while (qry.next())
        {
            getValue(qry.value("month"), &month);
            if ((month < 1) || (month > 12))
            {
                logger.writeError ("Invalid month: " + QString::number(month) + " idCell: " +"\n");
                return ERROR_GETTINGPARAM;
            }
            month = month - 1;
            getValue(qry.value("par1"), &par);
            monthlyPar1Input[month] = par;
            getValue(qry.value("par2"), &par);
            monthlyPar2Input[month] = par;
            getValue(qry.value("par3"), &par);
            monthlyPar3Input[month] = par;

            getValue(qry.value("reference_par1"), &par);
            monthlyPar1Ref[month] = par;
            getValue(qry.value("reference_par2"), &par);
            monthlyPar2Ref[month] = par;
            getValue(qry.value("reference_par3"), &par);
            monthlyPar3Ref[month] = par;
        }
    }
    return BIASCORRECTION_OK;
}

int Bias::numericalDataReconstruction(QString variable)
{
    meteoVariable var = getKeyMeteoVarMeteoMap(MapDailyMeteoVarToString, variable.toStdString());
    int res = 0;
    std::vector<double> monthlyPar1Input;
    std::vector<double> monthlyPar2Input;
    std::vector<double> monthlyPar3Input;
    std::vector<double> monthlyPar1Ref;
    std::vector<double> monthlyPar2Ref;
    std::vector<double> monthlyPar3Ref;

    for (int i=1; i<=12; i++)
    {
        monthlyPar1Input.push_back(NODATA);
        monthlyPar2Input.push_back(NODATA);
        monthlyPar3Input.push_back(NODATA);
        monthlyPar1Ref.push_back(NODATA);
        monthlyPar2Ref.push_back(NODATA);
        monthlyPar3Ref.push_back(NODATA);
    }
    for (int row = 0; row < inputGrid.gridStructure().header().nrRows; row++)
    {
        for (int col = 0; col < inputGrid.gridStructure().header().nrCols; col++)
        {
            std::string idInput;
            inputGrid.meteoGrid()->getMeteoPointActiveId(row, col, &idInput);  // store id
            if (!inputGrid.gridStructure().isFixedFields())
            {
                if (!inputGrid.loadGridDailyData(errorString, QString::fromStdString(idInput), firstDate, lastDate))
                {
                    continue;
                }
            }
            else
            {
                if (!inputGrid.loadGridDailyDataFixedFields(errorString, QString::fromStdString(idInput), firstDate, lastDate))
                {
                    continue;
                }
            }

            int res = getDistributionParam(QString::fromStdString(idInput), variable, monthlyPar1Input, monthlyPar2Input, monthlyPar3Input,
                                            monthlyPar1Ref, monthlyPar2Ref, monthlyPar3Ref);
            if (res != BIASCORRECTION_OK)
            {
                return res;
            }

            float y;
            QList <float> outputValues;
            for (QDate date = firstDate; date <= lastDate; date = date.addDays(1))
            {
                // input Grid
                float dailyValue = inputGrid.meteoGrid()->meteoPoint(row, col).getMeteoPointValueD(getCrit3DDate(date), var, &meteoSettings);
                unsigned int month = date.month() - 1;   // index start from 0

                y = NODATA;
                if (dailyValue != NODATA)
                {
                    if (var == dailyAirTemperatureMax || var == dailyAirTemperatureMin)
                    {
                        double median = monthlyPar1Input[month];
                        double stdDev = monthlyPar2Input[month];
                        double medianRef = monthlyPar1Ref[month];
                        double stdDevRef = monthlyPar2Ref[month];
                        if (median != NODATA && stdDev != NODATA && medianRef != NODATA && stdDevRef != NODATA)
                        {
                            double n = (dailyValue - median) / stdDev;
                            y = medianRef + stdDevRef * n;
                        }
                    }
                    else if (var == dailyPrecipitation)
                    {
                        double beta = monthlyPar1Input[month];
                        double alpha = monthlyPar2Input[month];
                        double pZero = monthlyPar3Input[month];
                        double betaRef = monthlyPar1Ref[month];
                        double alphaRef = monthlyPar2Ref[month];
                        double pZeroRef = monthlyPar3Ref[month];

                        if (beta != NODATA && alpha != NODATA && pZero != NODATA &&
                            betaRef != NODATA && alphaRef != NODATA && pZeroRef != NODATA)
                        {
                            if (dailyValue <= meteoSettings.getRainfallThreshold())
                            {
                                y = 0;
                            }
                            else
                            {
                                double accuracy = 0.001;
                                double outlierStep = 0.2;
                                float percentileInput = generalizedGammaCDF(dailyValue, beta, alpha, pZero);
                                y = inverseGeneralizedGammaCDF(percentileInput, alphaRef, betaRef, accuracy, pZeroRef, outlierStep);
                            }
                        }
                    }
                }

                outputValues.push_back(y);
            }
            // save values
            res = res + outputGrid.saveListDailyData(&errorString, QString::fromStdString(idInput), firstDate, var, outputValues, false);
        }
    }
    if (res == 0)
    {
        return BIASCORRECTION_OK;
    }
    else
    {
        return ERROR_SAVINGPARAM;
    }
}
