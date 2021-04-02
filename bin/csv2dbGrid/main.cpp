#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include "dbMeteoGrid.h"
#include "utilities.h"
#include "import.h"
#include <iostream>
#include <stdio.h>
#include <random>
#include <QRandomGenerator>
#define TEST

void usage()
{
    std::cout << "csv2dbGrid" << std::endl
              << "Usage: csv2dbGrid project.ini" << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    QString appPath = myApp.applicationDirPath() + "/";
    QString settingsFileName, csvFileName;
    Import import;

    if (argc <= 1)
    {
        #ifdef TEST
            QString dataPath;
            if (! searchDataPath(&dataPath)) return -1;

            settingsFileName = dataPath + "PROJECT/testImportCSV/testImportSettings.ini";
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
    /*
    // import ID list.csv
    result = import.loadIDList();
    if (result!=CSV2DBGRID_OK)
    {
        return result;
    }
    */
    // import value from csv file
    QDir csvDir(import.getCsvFilePath());
    QStringList listOfCsv= csvDir.entryList(QStringList() << "*.csv",QDir::Files);;// get a list of file
    for(int i=0; i<listOfCsv.count(); i++)
    {
        import.setCsvFileName(import.getCsvFilePath()+"/"+listOfCsv.at(i));
        if (i==0)
        {
            import.setIsFirstCsv(true);
        }
        else
        {
            import.setIsFirstCsv(false);
        }
        result = import.loadEnsembleValues();
        if (result!=CSV2DBGRID_OK)
        {
            return result;
        }
    }

    return CSV2DBGRID_OK;
}
