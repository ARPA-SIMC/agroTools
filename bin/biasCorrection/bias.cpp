#include "bias.h"
#include <QSettings>
#include <QDir>
#include <QTextStream>

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
}

void Bias::setSettingsFileName(const QString &value)
{
    settingsFileName = value;
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

    if (! refGrid.openDatabase(&errorString))
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

    if (! inputGrid.openDatabase(&errorString))
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
    QFile outputXmlFile(outputMeteoGrid);
    if (outputMeteoGrid.left(1) == ".")
    {
        outputMeteoGrid = path + QDir::cleanPath(outputMeteoGrid);
    }

    if (outputMeteoGrid.isEmpty() || !outputXmlFile.exists())
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

        if (! outputGrid.newDatabase(&errorString))
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

        if (! outputGrid.openDatabase(&errorString))
        {
            logger.writeError (errorString);
            return ERROR_DBGRID;
        }

        if (! outputGrid.loadCellProperties(&errorString))
        {
            logger.writeError (errorString);
            outputGrid.closeDatabase();
            return ERROR_DBGRID;
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
    projectSettings->endGroup();


    return BIASCORRECTION_OK;
}

