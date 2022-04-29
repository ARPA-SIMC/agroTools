#include "bias.h"
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
    method = "";
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

bool Bias::getIsFutureProjection() const
{
    return isFutureProjection;
}

void Bias::setIsFutureProjection(bool value)
{
    isFutureProjection = value;
}

QString Bias::getMethod() const
{
    return method;
}

QList<QString> Bias::getVarList() const
{
    return varList;
}

int Bias::readSettings()
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

    // method
    method = projectSettings->value("method","").toString();
    if (method.isEmpty())
    {
        logger.writeError ("missing method");
        return ERROR_MISSINGPARAMETERS;
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
    if (isFutureProjection)
    {
        if (!QFile::exists(dbClimateName))
        {
            logger.writeError ("Compute reference before the projections");
            refGrid.closeDatabase();
            inputGrid.closeDatabase();
            return ERROR_DBCLIMATE;
        }
    }
    else
    {
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
                        getLatLonFromRowCol(inputGrid.gridStructure().header(), row, col, &lat, &lon);
                        int refRow;
                        int refCol;
                        getMeteoGridRowColFromXY (refGrid.gridStructure().header(), lon, lat, &refRow, &refCol);
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

void Bias::computeMonthlyDistribution(meteoVariable var)
{
    for (int i = 0; i<inputCells.size(); i++)
    {
        int row = inputCells[i].x();
        int col = inputCells[i].y();
        int refRow = referenceCells[i].x();
        int refCol = referenceCells[i].y();
        std::string id;
        inputGrid.meteoGrid()->getMeteoPointActiveId(row, col, &id);  // store id
        if (!inputGrid.gridStructure().isFixedFields())
        {
            inputGrid.loadGridDailyData(&errorString, QString::fromStdString(id), firstDate, lastDate);
        }
        else
        {
            inputGrid.loadGridDailyDataFixedFields(&errorString, QString::fromStdString(id), firstDate, lastDate);
        }
        QDate temp(firstDate.year(),firstDate.month(),1);
        while(temp<=lastDate)
        {
            int month = temp.month();
            temp = temp.addMonths(1);
            // TO DO
        }
    }
}


