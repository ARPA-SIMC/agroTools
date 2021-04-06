#include "import.h"
#include <QSettings>
#include <QDir>
#include <QTextStream>

Import::Import()
{

}

void Import::initialize()
{
    path = "";
    projectName = "";
    settingsFileName = "";
    csvFileName = "";
    xmlDbGrid = "";
    meteoVar.clear();
    isDaily = false;;
    isEnsemble = false;
}

int Import::readSettings()
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

    // PROJECT
    projectSettings->beginGroup("project");
    projectName = projectSettings->value("name","").toString();

    xmlDbGrid = projectSettings->value("meteoGrid","").toString();
    if (xmlDbGrid.isEmpty())
    {
        logger.writeError ("missing xml DB grid");
        return ERROR_MISSINGPARAMETERS;
    }
    if (xmlDbGrid.left(1) == ".")
    {
        xmlDbGrid = path + QDir::cleanPath(xmlDbGrid);
    }

    // open grid
    QString errorString;
    if (! grid.parseXMLGrid(xmlDbGrid, &errorString))
    {
        logger.writeError (errorString);
        return ERROR_DBGRID;
    }

    if (! grid.openDatabase(&errorString))
    {
        logger.writeError (errorString);
        return ERROR_DBGRID;
    }

    if (! grid.loadCellProperties(&errorString))
    {
        logger.writeError (errorString);
        grid.closeDatabase();
        return ERROR_DBGRID;
    }
    grid.closeDatabase();

    csvFilePath = projectSettings->value("inputData","").toString();
    if (csvFilePath.isEmpty())
    {
        logger.writeError ("missing input data path");
        return ERROR_MISSINGPARAMETERS;
    }
    if (csvFilePath.left(1) == ".")
    {
        csvFilePath = path + QDir::cleanPath(csvFilePath);
    }

    QString tempVar = projectSettings->value("var","").toString();
    if (tempVar.isEmpty())
    {
        logger.writeError ("missing meteo variables");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        meteoVar = tempVar.split(",");
    }

    if (projectSettings->value("isDaily","").toString().isEmpty())
    {
        logger.writeError ("missing isDaily");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        isDaily = projectSettings->value("isDaily","").toBool();
    }

    if (projectSettings->value("isEnsemble","").toString().isEmpty())
    {
        logger.writeError ("missing isEnsemble");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        isEnsemble = projectSettings->value("isEnsemble","").toBool();
    }

    if (projectSettings->value("precipitationIsProgressive","").toString().isEmpty())
    {
        logger.writeError ("missing precipitationIsProgressive");
        return ERROR_MISSINGPARAMETERS;
    }
    else
    {
        precIsProgressive = projectSettings->value("precipitationIsProgressive","").toBool();
    }

    projectSettings->endGroup();

    return CSV2DBGRID_OK;
}

/*
int Import::loadValues()
{
    QFile myFile(csvFileName);
    int posValue = 2; // csv header Lat Lon Val MODIFICARE NEL CASO ENSEMBLE CON TUTTI I VALORI NELLO STESSO CSV
    int idListIndex = 0;
    QString valueStr;
    if ( !myFile.open(QFile::ReadOnly | QFile::Text) )
    {
        logger.writeError ("csvFileName file does not exist");
        return ERROR_MISSINGFILE;
    }
    else
    {
        QTextStream in(&myFile);
        //skip header
        QString line = in.readLine();
        while (!in.atEnd())
        {
            line = in.readLine();
            QStringList items = line.split(" ");
            items.removeAll({});
            if (IDList[idListIndex] != "-9999")
            {
                valueStr = items[posValue].toFloat();
                valuesMap.insert(IDList[idListIndex], items[posValue].toFloat());
            }
            idListIndex ++;
        }
    }
    myFile.close();
    return CSV2DBGRID_OK;
}
*/
int Import::loadEnsembleValues()
{
    if (!isEnsemble)
    {
        logger.writeError ("is not ensemble");
        return ERROR_BAD_REQUEST;
    }
    QFile myFile(csvFileName);
    int posLat = 0;
    int posLon = 1;
    int posValue = 2;
    int posPerturbationNumber = 3;
    int idListIndex = 0;
    int prev_perturbation = 0;
    int nFields = 4;
    std::string id;
    bool ok;

    if ( !myFile.open(QFile::ReadOnly | QFile::Text) )
    {
        logger.writeError ("csvFileName file does not exist");
        return ERROR_MISSINGFILE;
    }
    else
    {
        QTextStream in(&myFile);
        //skip header
        QString line = in.readLine();
        while (!in.atEnd())
        {
            line = in.readLine();
            QStringList items = line.split(" ");
            items.removeAll({});
            if (items.size()<nFields)
            {
                logger.writeError ("is not ensemble, missing field required");
                return ERROR_BAD_REQUEST;
            }
            int perturbation = items[posPerturbationNumber].toInt(&ok,10);
            if(!ok)
            {
                // repeated header lat lon value perturbation number
                continue;
            }
            if (perturbation != prev_perturbation)
            {
                // new ensemble, reset idListIndex
                prev_perturbation = perturbation;
                idListIndex = 0;
            }
            if (isFirstCsv && perturbation==0)
            {
                // make IDList
                double lat = items[posLat].toDouble();
                double lon = items[posLon].toDouble();
                if (!grid.meteoGrid()->getIdFromLatLon(lat,lon,&id))
                {
                    return ERROR_DBGRID;
                }
                IDList<<QString::fromStdString(id);
            }
            if (IDList[idListIndex] != "-9999")
            {
                valuesMap.insert(IDList[idListIndex], items[posValue].toFloat());
            }
            idListIndex ++;
        }
    }
    myFile.close();
    return CSV2DBGRID_OK;
}

int Import::writeEnsembleValues()
{

}

QString Import::getCsvFilePath() const
{
    return csvFilePath;
}

void Import::setIsFirstCsv(bool value)
{
    isFirstCsv = value;
}

QList<QString> Import::getMeteoVar() const
{
    return meteoVar;
}

void Import::setSettingsFileName(const QString &value)
{
    settingsFileName = value;
}

void Import::setCsvFileName(const QString &value)
{
    csvFileName = value;
}
