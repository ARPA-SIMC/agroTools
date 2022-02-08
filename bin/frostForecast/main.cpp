#include <QCoreApplication>
#include <QDir>
#include <iostream>

// uncomment to execute test
//#define TEST

void usage()
{
    std::cout << "frostForecast" << std::endl
              << "Usage: frostForecast <project.ini> -d: date" << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    QString settingsFileName;
   // Import import;

    if (argc <= 1)
    {
        #ifdef TEST
            QString dataPath;
            if (! searchDataPath(&dataPath)) return -1;

            //settingsFileName = dataPath + "PROJECT/testFrostForecast/testFrostForecastSettings.ini";
        #else
            usage();
            //return ERROR_MISSINGFILE;
        #endif
    }
    else
    {
        settingsFileName = argv[1];
        if (settingsFileName.right(3) != "ini")
        {
            //import.logger.writeError("Wrong file .ini: " + settingsFileName);
            usage();
            //return ERROR_MISSINGFILE;
        }
    }

    // TO DO
}
