#include "utilities.h"
#include "import.h"
#include <QCoreApplication>
#include <QDir>
#include <iostream>

// uncomment to execute test
#define TEST

void usage()
{
    std::cout << "\nUsage: csv2dbGrid <project.ini>" << std::endl << std::endl
              << "Import daily or hourly meteo series (single series or ensemble) into a PRAGA meteo grid." << std::endl
              << "WARNING (for ensemble data): only daily data are supported." << std::endl << std::endl;

}

int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    QString settingsFileName;
    Import import;

    if (argc <= 1)
    {
        #ifdef TEST
            QString dataPath;
            if (! searchDataPath(&dataPath)) return -1;

            settingsFileName = "//praga2-smr/PRAGA_STABLE/DATA/PROJECT/ARCADIA/importDailyArcadiaProj.ini";
        #else
            usage();
            return ERROR_MISSINGFILE;
        #endif
    }
    else
    {
        settingsFileName = argv[1];
        if (settingsFileName.right(3) != "ini")
        {
            import.logger.writeError("Wrong file .ini: " + settingsFileName);
            usage();
            return ERROR_MISSINGFILE;
        }
    }

    import.initialize();
    import.setSettingsFileName(settingsFileName);
    import.logger.writeInfo ("settingsFileName: " + settingsFileName);

    int result = import.readSettings();
    if (result != CSV2DBGRID_OK)
    {
        return result;
    }

    // import value from csv file
    QDir csvDir(import.getCsvFilePath());
    QStringList listOfCsv = csvDir.entryList(QStringList() << "*.csv", QDir::Files);    // get a list of file
    QString fileName;
    QList<QString> varList = import.getMeteoVarList();
    bool isFirst = true;
    for(int i=0; i < listOfCsv.count(); i++)
    {
        fileName = listOfCsv.at(i);

        if (! import.getIsMultiVariable())
        {
            bool isAvbVariable = false;
            for (int j= 0; j < varList.size(); j++)
            {
                // check file of interest
                if(fileName.left(varList[j].size()) == varList[j])
                {
                    isAvbVariable = true;
                    import.setMeteoVar(varList[j]);
                    break;
                }
            }
            if (isAvbVariable == false)
            {
                continue;
            }
        }

        import.setCsvFileName(import.getCsvFilePath() + "/" + fileName);
        import.logger.writeInfo("read file: " + fileName);

        if (isFirst)
        {
            import.setIsFirstCsv(true);
            isFirst = false;
        }
        else
        {
            import.setIsFirstCsv(false);
        }

        if (import.getIsMultiVariable())
        {
            QString errorStr;
            result = import.importDailyValuesMultiVariable(errorStr);
            if (result != CSV2DBGRID_OK)
            {
                import.logger.writeError(errorStr);
            }
            continue;
        }

        if (import.getIsDaily())
        {
            if (import.getIsEnsemble())
            {
                result = import.loadEnsembleDailyValues();
                if (result != CSV2DBGRID_OK)
                {
                    import.logger.writeError("Loading daily data ERROR");
                    return result;
                }
                else
                {
                    result = import.writeEnsembleDailyValues();
                    if (result != CSV2DBGRID_OK)
                    {
                        import.logger.writeError("Writing daily data ERROR");
                        return result;
                    }
                }
            }
            else
            {
                result = import.loadDailyValues();
                if (result != CSV2DBGRID_OK)
                {
                    import.logger.writeError("Loading daily data ERROR");
                    return result;
                }
                else
                {
                    result = import.writeDailyValues();
                    if (result != CSV2DBGRID_OK)
                    {
                        import.logger.writeError("Writing daily data ERROR");
                        return result;
                    }
                }
            }
        }
        else
        {
            if (import.getIsEnsemble())
            {
                import.logger.writeError("Ensembles hourly are not supported.");
                return ERROR_BAD_REQUEST;
            }
            else
            {
                result = import.loadMultiTimeValues();
                if (result != CSV2DBGRID_OK)
                {
                    import.logger.writeError("Loading hourly data ERROR");
                    return result;
                }
                else
                {
                    result = import.writeMultiTimeValues();
                    if (result != CSV2DBGRID_OK)
                    {
                        import.logger.writeError("Writing daily data ERROR");
                        return result;
                    }
                }
            }
        }
    }

    return CSV2DBGRID_OK;
}
