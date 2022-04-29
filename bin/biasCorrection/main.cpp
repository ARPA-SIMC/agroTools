#include <QCoreApplication>
#include <QDir>
#include <iostream>
#include "utilities.h"
#include "bias.h"

// uncomment to execute test
#define TEST

void usage()
{
    std::cout << "biasCorrection" << std::endl
              << "Usage: biasCorrection <project.ini> REFERENCE/PROJ" << std::endl;
    std::cout << std::flush;
}

int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    QString settingsFileName;
    Bias bias;

    if (argc <= 2)
    {
        #ifdef TEST
            QString dataPath;
            if (! searchDataPath(&dataPath)) return -1;

            settingsFileName = dataPath + "PROJECT/testHighlanderBias/testHighlanderBiasSettings.ini";
            bias.setIsFutureProjection(false); // REFERENCE
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
        QString referenceProj = argv[2];
        if (referenceProj == "REFERENCE")
        {
            bias.setIsFutureProjection(false);
        }
        else if (referenceProj == "PROJ")
        {
            bias.setIsFutureProjection(true);
        }
        else
        {
            bias.logger.writeError("Wrong file .ini: " + settingsFileName);
            usage();
            return ERROR_MISSINGPARAMETERS;
        }

    }

    bias.initialize();
    bias.setSettingsFileName(settingsFileName);
    bias.logger.writeInfo ("settingsFileName: " + settingsFileName);

    int result = bias.readSettings();
    if (result!=BIASCORRECTION_OK)
    {
        return result;
    }
    bias.matchCells();
    QList<QString> varList = bias.getVarList();
    if (bias.getIsFutureProjection() == false) // reference
    {
        for (int i = 0; i < varList.size(); i++)
        {
            if (varList[i] == "DAILY_TMIN" || varList[i] == "DAILY_TMAX")
            {
                if (bias.getMethod() == "quantileMapping")
                {

                }
                else
                {
                    // TO DO
                }
            }
            else if (varList[i] == "DAILY_PREC")
            {

            }
        }

    }
    else
    {
        // proj
        // TO DO
    }

}
