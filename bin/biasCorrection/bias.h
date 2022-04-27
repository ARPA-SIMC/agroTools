#ifndef BIAS_H
#define BIAS_H

#include <QString>
#include "logger.h"
#include "dbMeteoGrid.h"
#include "meteo.h"

#define BIASCORRECTION_OK 0
#define ERROR_MISSINGFILE -1

class Bias
{
public:
    Bias();
    Logger logger;
};

#endif // BIAS_H
