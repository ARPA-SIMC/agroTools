#include <QCoreApplication>
#include <QDir>
#include <iostream>
#include "utilities.h"
#include "bias.h"

// uncomment to execute test
//#define TEST

void usage()
{
    std::cout << "biasCorrection" << std::endl
              << "Usage: biasCorrection <project.ini>" << std::endl;
    std::cout << std::flush;
}

int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    QString settingsFileName;
    Bias bias;

    if (argc < 2)
    {
        #ifdef TEST
            QString dataPath;
            if (! searchDataPath(&dataPath)) return -1;

            settingsFileName = dataPath + "PROJECT/highlander_bias_test/highlander_bias_test.ini";
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
            bias.logger.writeError("Wrong file .ini: " + settingsFileName);
            usage();
            return ERROR_MISSINGFILE;
        }
    }

}
