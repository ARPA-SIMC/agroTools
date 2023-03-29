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
              << "Usage: biasCorrection <project.ini> <REFERENCE|DEBIAS>" << std::endl;
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
            bias.setIsDebias(false); // REFERENCE
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
        QString operation = argv[2];
        operation = operation.toUpper();
        if (operation == "REFERENCE" || operation == "CLIMATE")
        {
            bias.setIsDebias(false);
        }
        else if (operation == "DEBIAS")
        {
            bias.setIsDebias(true);
        }
        else
        {
            bias.logger.writeError("Wrong parameter: " + operation);
            usage();
            return ERROR_MISSINGPARAMETERS;
        }

    }

    bias.initialize();
    bias.setSettingsFileName(settingsFileName);
    bias.logger.writeInfo ("settingsFileName: " + settingsFileName);

    if (! bias.getIsDebias())
    {
        // reference
        int result = bias.readReferenceSettings();
        if (result != BIASCORRECTION_OK)
        {
            return result;
        }

        bias.matchCells();

        QList<QString> varList = bias.getVarList();
        for (int i = 0; i < varList.size(); i++)
        {
            bias.logger.writeInfo ("Compute variable: " + varList[i] + "...");
            result = bias.computeMonthlyDistribution(varList[i]);
            if (result != BIASCORRECTION_OK)
            {
                std::cout << "ERROR nr:" << result << " in compute climate of variable:" << varList[i].toStdString();
                return result;
            }
        }
    }
    else
    {
        // debias
        int result = bias.readDebiasSettings();
        if (result!=BIASCORRECTION_OK)
        {
            return result;
        }
        QList<QString> varList = bias.getVarList();
        for (int i = 0; i < varList.size(); i++)
        {
            bias.logger.writeInfo ("Compute variable: " + varList[i] + "...");
            result = bias.numericalDataReconstruction(varList[i]);
            if (result!=BIASCORRECTION_OK)
            {
                return result;
            }
        }
    }

    return BIASCORRECTION_OK;
}
