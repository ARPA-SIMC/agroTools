#include "utilities.h"
#include "import.h"
#include <QCoreApplication>
#include <QDir>
#include <iostream>

// uncomment to execute test
#define TEST

void usage()
{
    std::cout << "csv2dbGrid" << std::endl
              << "Usage: csv2dbGrid <project.ini>" << std::endl;
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

            //settingsFileName = dataPath + "PROJECT/testImportCSV/testImportSettings.ini";
            settingsFileName = dataPath + "PROJECT/testHRESimportCSV/testHRESImportSettings.ini";
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
    if (result!=CSV2DBGRID_OK)
    {
        return result;
    }

    // import value from csv file
    QDir csvDir(import.getCsvFilePath());
    QStringList listOfCsv = csvDir.entryList(QStringList() << "*.csv", QDir::Files); // get a list of file
    QString fileName;
    QList<QString> varList = import.getMeteoVar();
    bool isFirst = false;
    for(int i=0; i<listOfCsv.count(); i++)
    {
        fileName = listOfCsv.at(i);
        bool interest = false;
        for (int j= 0; j<varList.size(); j++)
        {
            // check file of interest
            // LC DA FIXARE fileName NET_RAD_XXXXX.csv contiene sia RAD che NET_RAD
            if(fileName.contains(varList[j]))
            {
                interest = true;
                import.setMeteoVar(varList[j]);
                break;
            }
        }
        if (interest == false)
        {
            continue;
        }

        import.setCsvFileName(import.getCsvFilePath()+"/"+fileName);
        import.logger.writeInfo("read file: " + fileName);

        if (!isFirst)
        {
            import.setIsFirstCsv(true);
            isFirst = true;
        }
        else
        {
            import.setIsFirstCsv(false);
        }
        if (import.getIsDaily())
        {
            if (import.getIsEnsemble())
            {
                result = import.loadEnsembleDailyValues();
                if (result!=CSV2DBGRID_OK)
                {
                    import.logger.writeError("Loading daily data ERROR");
                    return result;
                }
                else
                {
                    result = import.writeEnsembleDailyValues();
                    if (result!=CSV2DBGRID_OK)
                    {
                        import.logger.writeError("Writing daily data ERROR");
                        return result;
                    }
                }
            }
            else
            {
                // TO DO not ensemble
            }
        }
        else
        {
            if (import.getIsEnsemble())
            {
                // TO DO
            }
            else
            {
                result = import.loadMultiTimeValues();
                if (result!=CSV2DBGRID_OK)
                {
                    import.logger.writeError("Loading hourly data ERROR");
                    return result;
                }
                else
                {
                    result = import.writeMultiTimeValues();
                    if (result!=CSV2DBGRID_OK)
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
