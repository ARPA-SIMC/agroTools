/*!
    \copyright 2016 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by ARPAE Emilia-Romagna

    CRITERIA3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CRITERIA3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA3D.  If not, see <http://www.gnu.org/licenses/>.

    contacts:
    fausto.tomei@gmail.com
    ftomei@arpae.it
*/


#include <math.h>
#include <iomanip>
#include <sstream>

#include "commonConstants.h"
#include "basicMath.h"
#include "meteoPoint.h"
#include "quality.h"
#include "crit3dDate.h"
#include "meteo.h"


Crit3DMeteoPoint::Crit3DMeteoPoint()
{
    this->clear();
}

void Crit3DMeteoPoint::clear()
{
    this->dataset = "";
    this->municipality = "";
    this->state = "";
    this->region = "";
    this->province = "";

    this->name = "";
    this->id = "";
    this->isUTC = true;
    this->isForecast = false;

    this->aggregationPointsMaxNr = 0;

    this->latitude = NODATA;
    this->longitude = NODATA;
    this->area = NODATA;
    this->latInt = NODATA;
    this->lonInt = NODATA;
    this->isInsideDem = false;

    this->nrObsDataDaysH = 0;
    this->nrObsDataDaysD = 0;
    this->nrObsDataDaysM = 0;
    this->hourlyFraction = 1;

    this->_obsDataH = nullptr;

    this->currentValue = NODATA;
    this->residual = NODATA;

    this->elaboration = NODATA;
    this->anomaly = NODATA;
    this->anomalyPercentage = NODATA;
    this->climate = NODATA;

    this->active = false;
    this->selected = false;
    this->marked = false;

    this->quality = quality::missing_data;

    proxyValues.clear();
    lapseRateCode = primary;
    topographicDistance = nullptr;
}


void Crit3DMeteoPoint::setLapseRateCode(const std::string &lapseRateCode)
{
    if (lapseRateCode == "primary")
    {
        this->lapseRateCode = primary;
    }
    else if (lapseRateCode == "secondary")
    {
        this->lapseRateCode = secondary;
    }
    else if (lapseRateCode == "supplemental")
    {
        this->lapseRateCode = supplemental;
    }
}


void Crit3DMeteoPoint::initializeObsDataH(int myHourlyFraction, int numberOfDays, const Crit3DDate& firstDate)
{
    this->cleanObsDataH();

    nrObsDataDaysH = numberOfDays;
    hourlyFraction = myHourlyFraction;
    quality = quality::missing_data;
    residual = NODATA;

    unsigned int nrDailyValues = unsigned(hourlyFraction * 24);
    _obsDataH = new TObsDataH[unsigned(numberOfDays)];

    Crit3DDate myDate = firstDate;
    for (unsigned int i = 0; i < unsigned(numberOfDays); i++)
    {
        _obsDataH[i].date = myDate;
        _obsDataH[i].tAir = new float[nrDailyValues];
        _obsDataH[i].prec = new float[nrDailyValues];
        _obsDataH[i].rhAir = new float[nrDailyValues];
        _obsDataH[i].tDew = new float[nrDailyValues];
        _obsDataH[i].irradiance = new float[nrDailyValues];
        _obsDataH[i].netIrradiance = new float[nrDailyValues];
        _obsDataH[i].et0 = new float[nrDailyValues];
        _obsDataH[i].windVecX = new float[nrDailyValues];
        _obsDataH[i].windVecY = new float[nrDailyValues];
        _obsDataH[i].windVecInt = new float[nrDailyValues];
        _obsDataH[i].windVecDir = new float[nrDailyValues];
        _obsDataH[i].windScalInt = new float[nrDailyValues];
        _obsDataH[i].leafW = new int[nrDailyValues];
        _obsDataH[i].transmissivity = new float[nrDailyValues];

        for (unsigned int j = 0; j < nrDailyValues; j++)
        {
            _obsDataH[i].tAir[j] = NODATA;
            _obsDataH[i].prec[j] = NODATA;
            _obsDataH[i].rhAir[j] = NODATA;
            _obsDataH[i].tDew[j] = NODATA;
            _obsDataH[i].irradiance[j] = NODATA;
            _obsDataH[i].netIrradiance[j] = NODATA;
            _obsDataH[i].et0[j] = NODATA;
            _obsDataH[i].windVecX[j] = NODATA;
            _obsDataH[i].windVecY[j] = NODATA;
            _obsDataH[i].windVecInt[j] = NODATA;
            _obsDataH[i].windVecDir[j] = NODATA;
            _obsDataH[i].windScalInt[j] = NODATA;
            _obsDataH[i].leafW[j] = NODATA;
            _obsDataH[i].transmissivity[j] = NODATA;
        }
        ++myDate;
    }
}

void Crit3DMeteoPoint::initializeObsDataHFromMp(int myHourlyFraction, int numberOfDays, const Crit3DDate& firstDate, Crit3DMeteoPoint &meteoPoint)
{
    hourlyFraction = myHourlyFraction;
    unsigned int nrDailyValues = unsigned(hourlyFraction * 24);
    Crit3DDate myDate = firstDate;
    TObsDataH *data = meteoPoint.getObsDataH();

    for (unsigned int i = 0; i < unsigned(numberOfDays); i++)
    {
        if (i < unsigned(nrObsDataDaysH))
        {
            _obsDataH[i].date = myDate;
            for (unsigned int j = 0; j < nrDailyValues; j++)
            {
                _obsDataH[i].tAir[j] = data[i].tAir[j];
                _obsDataH[i].prec[j] = data[i].prec[j];
                _obsDataH[i].rhAir[j] = data[i].rhAir[j];
                _obsDataH[i].tDew[j] = data[i].tDew[j];
                _obsDataH[i].irradiance[j] = data[i].irradiance[j];
                _obsDataH[i].netIrradiance[j] =data[i].netIrradiance[j];
                _obsDataH[i].et0[j] = data[i].et0[j];
                _obsDataH[i].windVecX[j] = data[i].windVecX[j];
                _obsDataH[i].windVecY[j] = data[i].windVecY[j];
                _obsDataH[i].windVecInt[j] = data[i].windVecInt[j];
                _obsDataH[i].windVecDir[j] = data[i].windVecDir[j];
                _obsDataH[i].windScalInt[j] = data[i].windScalInt[j];
                _obsDataH[i].leafW[j] = data[i].leafW[j];
                _obsDataH[i].transmissivity[j] = data[i].transmissivity[j];
            }
            ++myDate;
        }
    }
}


void Crit3DMeteoPoint::initializeObsDataD(unsigned int numberOfDays, const Crit3DDate& firstDate)
{
    obsDataD.clear();
    obsDataD.resize(numberOfDays);
    nrObsDataDaysD = int(numberOfDays);

    quality = quality::missing_data;
    residual = NODATA;

    Crit3DDate myDate = firstDate;
    for (unsigned int i = 0; i < numberOfDays; i++)
    {
        obsDataD[i].date = myDate;
        obsDataD[i].tMax = NODATA;
        obsDataD[i].tMin = NODATA;
        obsDataD[i].tAvg = NODATA;
        obsDataD[i].prec = NODATA;
        obsDataD[i].rhMax = NODATA;
        obsDataD[i].rhMin = NODATA;
        obsDataD[i].rhAvg = NODATA;
        obsDataD[i].globRad = NODATA;
        obsDataD[i].et0_hs = NODATA;
        obsDataD[i].et0_pm = NODATA;
        obsDataD[i].dd_heating = NODATA;
        obsDataD[i].dd_cooling = NODATA;
        obsDataD[i].windVecIntAvg = NODATA;
        obsDataD[i].windVecIntMax = NODATA;
        obsDataD[i].windVecDirPrev = NODATA;
        obsDataD[i].windScalIntAvg = NODATA;
        obsDataD[i].windScalIntMax = NODATA;
        obsDataD[i].leafW = NODATA;
        obsDataD[i].waterTable = NODATA;
        ++myDate;
    }
}


void Crit3DMeteoPoint::initializeObsDataDFromMp(unsigned int numberOfDays, const Crit3DDate& firstDate, Crit3DMeteoPoint &meteoPoint)
{
    Crit3DDate myDate = firstDate;
    for (unsigned int i = 0; i < numberOfDays; i++)
    {
        obsDataD[i].date = myDate;
        obsDataD[i].tMax = meteoPoint.obsDataD[i].tMax;
        obsDataD[i].tMin = meteoPoint.obsDataD[i].tMin;
        obsDataD[i].tAvg = meteoPoint.obsDataD[i].tAvg;
        obsDataD[i].prec = meteoPoint.obsDataD[i].prec;
        obsDataD[i].rhMax = meteoPoint.obsDataD[i].rhMax;
        obsDataD[i].rhMin = meteoPoint.obsDataD[i].rhMin;
        obsDataD[i].rhAvg = meteoPoint.obsDataD[i].rhAvg;
        obsDataD[i].globRad = meteoPoint.obsDataD[i].globRad;
        obsDataD[i].et0_hs = meteoPoint.obsDataD[i].et0_hs;
        obsDataD[i].et0_pm = meteoPoint.obsDataD[i].et0_pm;
        obsDataD[i].dd_heating = meteoPoint.obsDataD[i].dd_heating;
        obsDataD[i].dd_cooling = meteoPoint.obsDataD[i].dd_cooling;
        obsDataD[i].windVecIntAvg = meteoPoint.obsDataD[i].windVecIntAvg;
        obsDataD[i].windVecIntMax = meteoPoint.obsDataD[i].windVecIntMax;
        obsDataD[i].windVecDirPrev = meteoPoint.obsDataD[i].windVecDirPrev;
        obsDataD[i].windScalIntAvg = meteoPoint.obsDataD[i].windScalIntAvg;
        obsDataD[i].windScalIntMax = meteoPoint.obsDataD[i].windScalIntMax;
        obsDataD[i].leafW = meteoPoint.obsDataD[i].leafW;
        obsDataD[i].waterTable = meteoPoint.obsDataD[i].waterTable;
        ++myDate;
    }
}


void Crit3DMeteoPoint::initializeObsDataM(unsigned int numberOfMonths, unsigned int month, int year)
{
    obsDataM.clear();
    obsDataM.resize(numberOfMonths);
    nrObsDataDaysM = numberOfMonths;

    quality = quality::missing_data;
    residual = NODATA;
    int addYear = 0;

    for (unsigned int i = month; i < month+numberOfMonths; i++)
    {
        if (i <= 12)
        {
            obsDataM[i-month]._month = i;   // obsDataM start from 0
            obsDataM[i-month]._year = year;
        }
        else
        {
            if (i%12 == 0)
            {
                obsDataM[i-month]._month = 12;
            }
            else
            {
                obsDataM[i-month]._month = i%12;
            }
            if (obsDataM[i-month]._month == 1)
            {
                // new year
                addYear++;
            }
            obsDataM[i-month]._year = year + addYear;
        }

        obsDataM[i-month].tMax = NODATA;
        obsDataM[i-month].tMin = NODATA;
        obsDataM[i-month].tAvg = NODATA;
        obsDataM[i-month].prec = NODATA;
        obsDataM[i-month].et0_hs = NODATA;
        obsDataM[i-month].globRad = NODATA;
        obsDataM[i-month].bic = NODATA;
    }
}


void Crit3DMeteoPoint::emptyVarObsDataH(meteoVariable myVar, const Crit3DDate& myDate)
{
    if (! isDateLoadedH(myDate)) return;

    int nrDayValues = hourlyFraction * 24;
    int i = _obsDataH[0].date.daysTo(myDate);
    residual = NODATA;

    if (i >= 0 && i < nrObsDataDaysH)
        if (_obsDataH[i].date == myDate)
            for (int j = 0; j < nrDayValues; j++)
            {
                if (myVar == airTemperature)
                    _obsDataH[i].tAir[j] = NODATA;
                else if (myVar == precipitation)
                    _obsDataH[i].prec[j] = NODATA;
                else if (myVar == airRelHumidity)
                    _obsDataH[i].rhAir[j] = NODATA;
                else if (myVar == airDewTemperature)
                    _obsDataH[i].tDew[j] = NODATA;
                else if (myVar == globalIrradiance)
                    _obsDataH[i].irradiance[j] = NODATA;
                else if (myVar == netIrradiance)
                    _obsDataH[i].netIrradiance[j] = NODATA;
                else if (myVar == windScalarIntensity)
                    _obsDataH[i].windScalInt[j] = NODATA;
                else if (myVar == windVectorX)
                    _obsDataH[i].windVecX[j] = NODATA;
                else if (myVar == windVectorY)
                    _obsDataH[i].windVecY[j] = NODATA;
                else if (myVar == windVectorIntensity)
                    _obsDataH[i].windVecInt[j] = NODATA;
                else if (myVar == windVectorDirection)
                    _obsDataH[i].windVecDir[j] = NODATA;
                else if (myVar == leafWetness)
                    _obsDataH[i].leafW[j] = NODATA;
                else if (myVar == atmTransmissivity)
                    _obsDataH[i].transmissivity[j] = NODATA;
				else if (myVar == referenceEvapotranspiration)
                    _obsDataH[i].et0[j] = NODATA;
			}
}

void Crit3DMeteoPoint::emptyVarObsDataH(meteoVariable myVar, const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (! isDateIntervalLoadedH(date1, date2)) return;

    int nrDayValues = hourlyFraction * 24;
    int indexIni = _obsDataH[0].date.daysTo(date1);
    int indexFin = _obsDataH[0].date.daysTo(date2);
    residual = NODATA;

    for (int i = indexIni; i <= indexFin; i++)
        for (int j = 0; j < nrDayValues; j++)
        {
            if (myVar == airTemperature)
                _obsDataH[i].tAir[j] = NODATA;
            else if (myVar == precipitation)
                _obsDataH[i].prec[j] = NODATA;
            else if (myVar == airRelHumidity)
                _obsDataH[i].rhAir[j] = NODATA;
            else if (myVar == airDewTemperature)
                _obsDataH[i].tDew[j] = NODATA;
            else if (myVar == globalIrradiance)
                _obsDataH[i].irradiance[j] = NODATA;
            else if (myVar == netIrradiance)
                _obsDataH[i].netIrradiance[j] = NODATA;
            else if (myVar == windScalarIntensity)
                _obsDataH[i].windScalInt[j] = NODATA;
            else if (myVar == windVectorX)
                _obsDataH[i].windVecX[j] = NODATA;
            else if (myVar == windVectorY)
                _obsDataH[i].windVecY[j] = NODATA;
            else if (myVar == windVectorIntensity)
                _obsDataH[i].windVecInt[j] = NODATA;
            else if (myVar == windVectorDirection)
                _obsDataH[i].windVecDir[j] = NODATA;
            else if (myVar == leafWetness)
                _obsDataH[i].leafW[j] = NODATA;
            else if (myVar == atmTransmissivity)
                _obsDataH[i].transmissivity[j] = NODATA;
			else if (myVar == referenceEvapotranspiration)
                _obsDataH[i].et0[j] = NODATA;
        }
}

void Crit3DMeteoPoint::emptyObsDataH(const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (! isDateIntervalLoadedH(date1, date2)) return;

    int nrDayValues = hourlyFraction * 24;
    int indexIni = _obsDataH[0].date.daysTo(date1);
    int indexFin = _obsDataH[0].date.daysTo(date2);

    for (int i = indexIni; i <= indexFin; i++)
        for (int j = 0; j < nrDayValues; j++)
        {
            _obsDataH[i].tAir[j] = NODATA;
            _obsDataH[i].prec[j] = NODATA;
            _obsDataH[i].rhAir[j] = NODATA;
            _obsDataH[i].tDew[j] = NODATA;
            _obsDataH[i].irradiance[j] = NODATA;
            _obsDataH[i].netIrradiance[j] = NODATA;
            _obsDataH[i].windScalInt[j] = NODATA;
            _obsDataH[i].windVecX[j] = NODATA;
            _obsDataH[i].windVecY[j] = NODATA;
            _obsDataH[i].windVecInt[j] = NODATA;
            _obsDataH[i].windVecDir[j] = NODATA;
            _obsDataH[i].leafW[j] = NODATA;
            _obsDataH[i].transmissivity[j] = NODATA;
            _obsDataH[i].et0[j] = NODATA;
        }
}

void Crit3DMeteoPoint::emptyVarObsDataD(meteoVariable myVar, const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (! isDateIntervalLoadedH(date1, date2)) return;

    int indexIni = obsDataD[0].date.daysTo(date1);
    int indexFin = obsDataD[0].date.daysTo(date2);
    residual = NODATA;

    for (unsigned int i = indexIni; i <= unsigned(indexFin); i++)
        if (myVar == dailyAirTemperatureMax)
            obsDataD[i].tMax = NODATA;
        else if (myVar == dailyAirTemperatureMin)
            obsDataD[i].tMin = NODATA;
        else if (myVar == dailyAirTemperatureAvg)
            obsDataD[i].tAvg = NODATA;
        else if (myVar == dailyPrecipitation)
            obsDataD[i].prec = NODATA;
        else if (myVar == dailyAirRelHumidityMax)
            obsDataD[i].rhMax = NODATA;
        else if (myVar == dailyAirRelHumidityMin)
            obsDataD[i].rhMin = NODATA;
        else if (myVar == dailyAirRelHumidityAvg)
            obsDataD[i].rhAvg = NODATA;
        else if (myVar == dailyGlobalRadiation)
            obsDataD[i].globRad = NODATA;
        else if (myVar == dailyWindScalarIntensityAvg)
            obsDataD[i].windScalIntAvg = NODATA;
        else if (myVar == dailyWindScalarIntensityMax)
            obsDataD[i].windScalIntMax = NODATA;
        else if (myVar == dailyWindVectorIntensityAvg)
            obsDataD[i].windVecIntAvg = NODATA;
        else if (myVar == dailyWindVectorIntensityMax)
            obsDataD[i].windVecIntMax = NODATA;
        else if (myVar == dailyWindVectorDirectionPrevailing)
            obsDataD[i].windVecDirPrev = NODATA;
        else if (myVar == dailyReferenceEvapotranspirationHS)
            obsDataD[i].et0_hs = NODATA;
        else if (myVar == dailyReferenceEvapotranspirationPM)
            obsDataD[i].et0_pm = NODATA;
        else if (myVar == dailyLeafWetness)
            obsDataD[i].leafW = NODATA;
        else if (myVar == dailyHeatingDegreeDays)
            obsDataD[i].dd_heating = NODATA;
        else if (myVar == dailyCoolingDegreeDays)
            obsDataD[i].dd_cooling = NODATA;
}

void Crit3DMeteoPoint::emptyObsDataD(const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (! isDateIntervalLoadedH(date1, date2)) return;

    int indexIni = _obsDataH[0].date.daysTo(date1);
    int indexFin = _obsDataH[0].date.daysTo(date2);

    for (unsigned int i = indexIni; i <= unsigned(indexFin); i++)
    {
        obsDataD[i].tMax = NODATA;
        obsDataD[i].tMin = NODATA;
        obsDataD[i].tAvg = NODATA;
        obsDataD[i].prec = NODATA;
        obsDataD[i].rhMax = NODATA;
        obsDataD[i].rhMin = NODATA;
        obsDataD[i].rhAvg = NODATA;
        obsDataD[i].globRad = NODATA;
        obsDataD[i].windScalIntAvg = NODATA;
        obsDataD[i].windScalIntMax = NODATA;
        obsDataD[i].windVecIntAvg = NODATA;
        obsDataD[i].windVecIntMax = NODATA;
        obsDataD[i].windVecDirPrev = NODATA;
        obsDataD[i].et0_hs = NODATA;
        obsDataD[i].et0_pm = NODATA;
        obsDataD[i].dd_heating = NODATA;
        obsDataD[i].dd_cooling = NODATA;
        obsDataD[i].leafW = NODATA;
    }
}

void Crit3DMeteoPoint::emptyObsDataM(const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (! isDateIntervalLoadedM(date1, date2)) return;

    int indexIni = (date1.year - obsDataM[0]._year)*12 + date1.month-obsDataM[0]._month;
    int indexFin = (date2.year - obsDataM[0]._year)*12 + date2.month-obsDataM[0]._month;

    for (unsigned int i = indexIni; i <= unsigned(indexFin); i++)
    {
        obsDataM[i].tMax = NODATA;
        obsDataM[i].tMin = NODATA;
        obsDataM[i].tAvg = NODATA;
        obsDataM[i].prec = NODATA;
        obsDataM[i].et0_hs = NODATA;
        obsDataM[i].globRad = NODATA;
        obsDataM[i].bic = NODATA;
    }
}

bool Crit3DMeteoPoint::isDateLoadedH(const Crit3DDate& myDate)
{
    if (nrObsDataDaysH == 0)
        return false;
    else if (myDate < _obsDataH[0].date || myDate > _obsDataH[nrObsDataDaysH - 1].date)
        return false;
    else
        return true;
}

bool Crit3DMeteoPoint::isDateTimeLoadedH(const Crit3DTime& myDateTime)
{
    if (nrObsDataDaysH == 0)
        return false;
    else if (myDateTime < Crit3DTime(_obsDataH[0].date,1) || myDateTime >= Crit3DTime(_obsDataH[nrObsDataDaysH - 1].date,1))
        return false;
    else
        return true;
}

bool Crit3DMeteoPoint::isDateIntervalLoadedH(const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (nrObsDataDaysH == 0)
        return false;
    else if (date1 > date2)
        return false;
    else if (date1 < _obsDataH[0].date || date2 > _obsDataH[nrObsDataDaysH - 1].date)
        return false;
    else
        return true;
}

bool Crit3DMeteoPoint::isDateLoadedD(const Crit3DDate& myDate)
{
    if (nrObsDataDaysD == 0)
        return false;
    else if (myDate < obsDataD[0].date || myDate > obsDataD[unsigned(nrObsDataDaysD-1)].date)
        return false;
    else
        return true;
}

bool Crit3DMeteoPoint::isDateLoadedM(const Crit3DDate& myDate)
{
    if (nrObsDataDaysM == 0)
        return false;
    else if ( (myDate.year < obsDataM[0]._year ) || (myDate.year > obsDataM[unsigned(nrObsDataDaysM-1)]._year))
        return false;
    else if (myDate.year == obsDataM[0]._year && myDate.month < obsDataM[0]._month)
        return false;
    else if (myDate.year == obsDataM[unsigned(nrObsDataDaysM-1)]._year && myDate.month > obsDataM[unsigned(nrObsDataDaysM-1)]._month)
        return false;
    else
        return true;
}

bool Crit3DMeteoPoint::isDateIntervalLoadedD(const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (nrObsDataDaysD == 0)
        return false;
    else if (date1 > date2)
        return false;
    else if (date1 < obsDataD[0].date || date2 > obsDataD[unsigned(nrObsDataDaysD-1)].date)
        return false;
    else
        return (true);
}

bool Crit3DMeteoPoint::isDateIntervalLoadedM(const Crit3DDate& date1, const Crit3DDate& date2)
{
    if (nrObsDataDaysM == 0)
        return false;
    else if (date1 > date2)
        return false;
    else if ( (date1.year < obsDataM[0]._year ) || (date2.year > obsDataM[unsigned(nrObsDataDaysM-1)]._year))
        return false;
    else if (date1.year == obsDataM[0]._year && date1.month < obsDataM[0]._month)
        return false;
    else if (date2.year == obsDataM[unsigned(nrObsDataDaysM-1)]._year && date2.month > obsDataM[unsigned(nrObsDataDaysM-1)]._month)
        return false;
    else
        return true;
}

bool Crit3DMeteoPoint::isDateIntervalLoadedH(const Crit3DTime& timeIni, const Crit3DTime& timeFin)
{
    if (nrObsDataDaysH == 0)
        return false;
    else if (timeIni > timeFin)
        return false;
    else if (_obsDataH == nullptr)
        return false;
    else if (timeIni.date < _obsDataH[0].date || timeFin.date > (_obsDataH[0].date.addDays(nrObsDataDaysH - 1)))
        return (false);
    else
        return (true);
}

float Crit3DMeteoPoint::obsDataConsistencyH(meteoVariable myVar, const Crit3DTime& timeIni, const Crit3DTime& timeFin)
{
    if (nrObsDataDaysH == 0)
        return 0.0;
    else if (timeIni > timeFin)
        return 0.0;
    else if (_obsDataH == nullptr)
        return 0.0;
    else if (timeFin.date < _obsDataH[0].date || timeIni.date > (_obsDataH[0].date.addDays(nrObsDataDaysH - 1)))
        return 0.0;
    else
    {
        Crit3DTime myTime = timeIni;
        float myValue;
        int deltaSeconds = 3600 / hourlyFraction;
        int counter=0, counterAll=0;
        while (myTime <= timeFin)
        {
            myValue = getMeteoPointValueH(myTime.date, myTime.getHour(), myTime.getMinutes(), myVar);
            if (int(myValue) != int(NODATA))
                counter++;

            counterAll++;
            myTime = myTime.addSeconds(deltaSeconds);
        }
        return (float(counter)/float(counterAll));
    }

}

void Crit3DMeteoPoint::cleanObsDataH()
{
    quality = quality::missing_data;

    if (nrObsDataDaysH > 0)
    {
        for (int i = 0; i < nrObsDataDaysH; i++)
        {
            delete [] _obsDataH[i].tAir;
            delete [] _obsDataH[i].prec;
            delete [] _obsDataH[i].rhAir;
            delete [] _obsDataH[i].tDew;
            delete [] _obsDataH[i].irradiance;
            delete [] _obsDataH[i].netIrradiance;
            delete [] _obsDataH[i].windScalInt;
            delete [] _obsDataH[i].windVecX;
            delete [] _obsDataH[i].windVecY;
            delete [] _obsDataH[i].windVecInt;
            delete [] _obsDataH[i].windVecDir;
            delete [] _obsDataH[i].leafW;
            delete [] _obsDataH[i].transmissivity;
        }
        delete [] _obsDataH;
    }

    nrObsDataDaysH = 0;
}


void Crit3DMeteoPoint::cleanAllData()
{
    cleanObsDataH();

    obsDataD.clear();
    obsDataM.clear();

    quality = quality::missing_data;
}


bool Crit3DMeteoPoint::setMeteoPointValueH(const Crit3DDate& myDate, int myHour, int myMinutes, meteoVariable myVar, float myValue)
{
    // check
    if (myVar == noMeteoVar || _obsDataH == nullptr)
    {
        return false;
    }

    // day index
    int i = _obsDataH[0].date.daysTo(myDate);

    // check if out of range (accept +1 date exceed)
    if (i < 0 || i > nrObsDataDaysH) return false;

    // sub hourly index
    int subH = int(ceil(float(myMinutes) / float(60 / hourlyFraction)));

    // if +1 date exceed accept only hour 00:00
    if (i == nrObsDataDaysH && (myHour != 0 || subH != 0)) return false;

    // hour 0 becomes hour 24 of the previous day
    if (myHour == 0 && subH == 0)
    {
        myHour = 24;
        i--;
        if (i < 0) return false;
    }

    // (sub)hour index
    int j = hourlyFraction * myHour + subH - 1;

    if (j < 0 || j >= hourlyFraction * 24) return false;

    if (myVar == airTemperature)
        _obsDataH[i].tAir[j] = myValue;
    else if (myVar == precipitation)
        _obsDataH[i].prec[j] = myValue;
    else if (myVar == airRelHumidity)
        _obsDataH[i].rhAir[j] = myValue;
    else if (myVar == airDewTemperature)
        _obsDataH[i].tDew[j] = myValue;
    else if (myVar == globalIrradiance)
        _obsDataH[i].irradiance[j] = myValue;
    else if (myVar == netIrradiance)
        _obsDataH[i].netIrradiance[j] = myValue;
    else if (myVar == referenceEvapotranspiration)
        _obsDataH[i].et0[j] = myValue;
    else if (myVar == windScalarIntensity)
        _obsDataH[i].windScalInt[j] = myValue;
    else if (myVar == windVectorX)
    {
        _obsDataH[i].windVecX[j] = myValue;
        float intensity = NODATA, direction = NODATA;
        computeWindPolar(_obsDataH[i].windVecX[j], _obsDataH[i].windVecY[j], &intensity, &direction);
        _obsDataH[i].windVecInt[j] = intensity;
        _obsDataH[i].windVecDir[j] = direction;
    }
    else if (myVar == windVectorY)
    {
        _obsDataH[i].windVecY[j] = myValue;
        float intensity = NODATA, direction = NODATA;
        computeWindPolar(_obsDataH[i].windVecX[j], _obsDataH[i].windVecY[j], &intensity, &direction);
        _obsDataH[i].windVecInt[j] = intensity;
        _obsDataH[i].windVecDir[j] = direction;
    }
    else if (myVar == windVectorIntensity)
    {
        _obsDataH[i].windVecInt[j] = myValue;
        float u = NODATA, v = NODATA;
        computeWindCartesian(_obsDataH[i].windVecInt[j], _obsDataH[i].windVecDir[j], &u, &v);
        _obsDataH[i].windVecX[j] = u;
        _obsDataH[i].windVecY[j] = v;
    }
    else if (myVar == windVectorDirection)
    {
        _obsDataH[i].windVecDir[j] = myValue;
        float u = NODATA, v = NODATA;
        computeWindCartesian(_obsDataH[i].windVecInt[j], _obsDataH[i].windVecDir[j], &u, &v);
        _obsDataH[i].windVecX[j] = u;
        _obsDataH[i].windVecY[j] = v;
    }
    else if (myVar == leafWetness)
        _obsDataH[i].leafW[j] = int(myValue);
    else if (myVar == atmTransmissivity)
        _obsDataH[i].transmissivity[j] = myValue;
    else
        return false;

    return true;
}

bool Crit3DMeteoPoint::setMeteoPointValueD(const Crit3DDate& myDate, meteoVariable myVar, float myValue)
{
    long index = obsDataD[0].date.daysTo(myDate);
    if ((index < 0) || (index >= nrObsDataDaysD)) return false;

    unsigned i = unsigned(index);

    if (myVar == dailyAirTemperatureMax)
        obsDataD[i].tMax = myValue;
    else if (myVar == dailyAirTemperatureMin)
        obsDataD[i].tMin = myValue;
    else if (myVar == dailyAirTemperatureAvg)
        obsDataD[i].tAvg = myValue;
    else if (myVar == dailyPrecipitation)
        obsDataD[i].prec = myValue;
    else if (myVar == dailyAirRelHumidityMax)
        obsDataD[i].rhMax = myValue;
    else if (myVar == dailyAirRelHumidityMin)
        obsDataD[i].rhMin = myValue;
    else if (myVar == dailyAirRelHumidityAvg)
        obsDataD[i].rhAvg = myValue;
    else if (myVar == dailyGlobalRadiation)
        obsDataD[i].globRad = myValue;
    else if (myVar == dailyReferenceEvapotranspirationHS)
         obsDataD[i].et0_hs = myValue;
    else if (myVar == dailyReferenceEvapotranspirationPM)
         obsDataD[i].et0_pm = myValue;
    else if (myVar == dailyHeatingDegreeDays)
         obsDataD[i].dd_heating = myValue;
    else if (myVar == dailyCoolingDegreeDays)
         obsDataD[i].dd_cooling = myValue;
    else if (myVar == dailyWindScalarIntensityAvg)
        obsDataD[i].windScalIntAvg = myValue;
    else if (myVar == dailyWindScalarIntensityMax)
        obsDataD[i].windScalIntMax = myValue;
    else if (myVar == dailyWindVectorIntensityAvg)
        obsDataD[i].windVecIntAvg = myValue;
    else if (myVar == dailyWindVectorIntensityMax)
        obsDataD[i].windVecIntMax = myValue;
    else if (myVar == dailyWindVectorDirectionPrevailing)
        obsDataD[i].windVecDirPrev = myValue;
    else if (myVar == dailyLeafWetness)
        obsDataD[i].leafW = myValue;					
    else if (myVar == dailyWaterTableDepth)
        obsDataD[i].waterTable = myValue;
    else
        return false;

    return true;
}

bool Crit3DMeteoPoint::setMeteoPointValueM(const Crit3DDate &myDate, meteoVariable myVar, float myValue)
{
    //check
    if (myVar == noMeteoVar) return false;
    if (nrObsDataDaysM == 0) return false;

    int index;
    if (myDate.year == obsDataM[0]._year)
    {
        // same year of first data
        index = myDate.month-obsDataM[0]._month;
    }
    else if (myDate.year == obsDataM[0]._year+1)
    {
        // second year
        index = 12-obsDataM[0]._month + myDate.month;
    }
    else
    {
        // other years
        index = (myDate.year - obsDataM[0]._year -1)*12+(12-obsDataM[0]._month) + myDate.month;
    }
    if ((index < 0) || (index >= nrObsDataDaysM)) return false;

    unsigned i = unsigned(index);

    if (myVar == monthlyAirTemperatureMax)
        obsDataM[i].tMax = myValue;
    else if (myVar == monthlyAirTemperatureMin)
        obsDataM[i].tMin = myValue;
    else if (myVar == monthlyAirTemperatureAvg)
        obsDataM[i].tAvg = myValue;
    else if (myVar == monthlyPrecipitation)
        obsDataM[i].prec = myValue;
    else if (myVar == monthlyReferenceEvapotranspirationHS)
        obsDataM[i].et0_hs = myValue;
    else if (myVar == monthlyGlobalRadiation)
        obsDataM[i].globRad = myValue;
    else if (myVar == monthlyBIC)
        obsDataM[i].bic = myValue;
    else
        return false;

    return true;
}

float Crit3DMeteoPoint::getMeteoPointValueH(const Crit3DDate& myDate, int myHour, int myMinutes, meteoVariable myVar)
{
    if (myVar == noMeteoVar)
    {
        return NODATA;
    }
    if (_obsDataH == nullptr)
    {
        return NODATA;
    }

    // day index
    int i = _obsDataH[0].date.daysTo(myDate);

    //check if out of range (accept +1 date exceed)
    if (i < 0 || i > nrObsDataDaysH)
    {
        return NODATA;
    }

    // sub hourly index
    int subH = int(ceil(float(myMinutes) / float(60 / hourlyFraction)));

    // if +1 date exceed accept only hour 00:00
    if (i == nrObsDataDaysH && (myHour != 0 || subH != 0))
    {
        return NODATA;
    }

    // hour 0 becomes hour 24 of the previous day
    if (myHour == 0 && subH == 0)
    {
        myHour = 24;
        i--;
        if (i < 0) return NODATA;
    }

    // (sub)hour index
    int j = hourlyFraction * myHour + subH - 1;
    if (j < 0 || j >= hourlyFraction * 24)
    {
        return NODATA;
    }

    if (myVar == airTemperature)
        return (_obsDataH[i].tAir[j]);
    else if (myVar == precipitation)
        return (_obsDataH[i].prec[j]);
    else if (myVar == airRelHumidity)
        return (_obsDataH[i].rhAir[j]);
    else if (myVar == airDewTemperature)
    {
        if (! isEqual(_obsDataH[i].tDew[j], NODATA))
            return _obsDataH[i].tDew[j];
        else
            return tDewFromRelHum(_obsDataH[i].rhAir[j], _obsDataH[i].tAir[j]);
    }
    else if (myVar == globalIrradiance)
        return (_obsDataH[i].irradiance[j]);
    else if (myVar == netIrradiance)
        return (_obsDataH[i].netIrradiance[j]);
    else if (myVar == referenceEvapotranspiration)
        return (_obsDataH[i].et0[j]);
    else if (myVar == windScalarIntensity)
        return (_obsDataH[i].windScalInt[j]);
    else if (myVar == windVectorX)
        return (_obsDataH[i].windVecX[j]);
    else if (myVar == windVectorY)
        return (_obsDataH[i].windVecY[j]);
    else if (myVar == windVectorIntensity)
        return (_obsDataH[i].windVecInt[j]);
    else if (myVar == windVectorDirection)
        return (_obsDataH[i].windVecDir[j]);
    else if (myVar == leafWetness)
        return float(_obsDataH[i].leafW[j]);
    else if (myVar == atmTransmissivity)
        return (_obsDataH[i].transmissivity[j]);
    else
    {
        return NODATA;
    }
}


Crit3DDate Crit3DMeteoPoint::getMeteoPointHourlyValuesDate(int index) const
{
    if (index < 0 || index >= nrObsDataDaysH)
        return NO_DATE;

    return _obsDataH[index].date;
}


bool Crit3DMeteoPoint::getMeteoPointValueDayH(const Crit3DDate& myDate, TObsDataH* &hourlyValues)
{
    int d = _obsDataH[0].date.daysTo(myDate);
    if (d < 0 || d >= nrObsDataDaysH)
        return false;

    hourlyValues = &(_obsDataH[d]);

    return true;
}


bool Crit3DMeteoPoint::existDailyData(const Crit3DDate& myDate)
{
    if (obsDataD.size() == 0)
        return false;

    int index = obsDataD[0].date.daysTo(myDate);

    if ((index < 0) || (index >= nrObsDataDaysD))
        return false;
    else
        return true;
}


Crit3DDate Crit3DMeteoPoint::getLastDailyData() const
{
    if (obsDataD.size() == 0)
        return NO_DATE;

    return obsDataD[nrObsDataDaysD-1].date;
}


Crit3DDate Crit3DMeteoPoint::getFirstDailyData() const
{
    if (obsDataD.size() == 0)
        return NO_DATE;

    return obsDataD[0].date;
}


float Crit3DMeteoPoint::getMeteoPointValueD(const Crit3DDate &myDate, meteoVariable myVar, Crit3DMeteoSettings* meteoSettings) const
{
    //check
    if (myVar == noMeteoVar) return NODATA;
    if (nrObsDataDaysD == 0) return NODATA;

    int index = obsDataD[0].date.daysTo(myDate);
    if ((index < 0) || (index >= nrObsDataDaysD)) return NODATA;

    unsigned i = unsigned(index);

    if (myVar == dailyAirTemperatureMax)
        return (obsDataD[i].tMax);
    else if (myVar == dailyAirTemperatureMin)
        return (obsDataD[i].tMin);
    else if (myVar == dailyAirTemperatureAvg)
    {
        if (! isEqual(obsDataD[i].tAvg, NODATA))
            return obsDataD[i].tAvg;
        else if (meteoSettings->getAutomaticTavg() && !isEqual(obsDataD[i].tMin, NODATA) && !isEqual(obsDataD[i].tMax, NODATA))
            return ((obsDataD[i].tMin + obsDataD[i].tMax) / 2);
        else
            return NODATA;
    }
    else if (myVar == dailyPrecipitation)
        return (obsDataD[i].prec);
    else if (myVar == dailyAirRelHumidityMax)
        return (obsDataD[i].rhMax);
    else if (myVar == dailyAirRelHumidityMin)
        return float(obsDataD[i].rhMin);
    else if (myVar == dailyAirRelHumidityAvg)
        return (obsDataD[i].rhAvg);
    else if (myVar == dailyGlobalRadiation)
        return (obsDataD[i].globRad);
    else if (myVar == dailyReferenceEvapotranspirationHS || myVar == dailyBIC)
    {
        float et0 = NODATA;
        if (! isEqual(obsDataD[i].et0_hs, NODATA))
            et0 = obsDataD[i].et0_hs;
        else if (meteoSettings->getAutomaticET0HS() && !isEqual(obsDataD[i].tMin, NODATA) && !isEqual(obsDataD[i].tMax, NODATA))
            et0 = float(ET0_Hargreaves(meteoSettings->getTransSamaniCoefficient(), latitude,
                                        getDoyFromDate(myDate), obsDataD[i].tMax, obsDataD[i].tMin));

        if (myVar == dailyReferenceEvapotranspirationHS)
            return et0;
        else
        {
            float prec = NODATA;
            prec = obsDataD[i].prec;
            return computeDailyBIC(prec, et0);
        }
    }
    else if (myVar == dailyReferenceEvapotranspirationPM)
        return (obsDataD[i].et0_pm);
    else if (myVar == dailyHeatingDegreeDays)
        return (obsDataD[i].dd_heating);
    else if (myVar == dailyCoolingDegreeDays)
        return (obsDataD[i].dd_cooling);
    else if (myVar == dailyWindScalarIntensityAvg)
        return (obsDataD[i].windScalIntAvg);
    else if (myVar == dailyWindScalarIntensityMax)
        return (obsDataD[i].windScalIntMax);
    else if (myVar == dailyWindVectorIntensityAvg)
        return (obsDataD[i].windVecIntAvg);
    else if (myVar == dailyWindVectorIntensityMax)
        return (obsDataD[i].windVecIntMax);
    else if (myVar == dailyWindVectorDirectionPrevailing)
        return (obsDataD[i].windVecDirPrev);
    else if (myVar == dailyLeafWetness)
        return (obsDataD[i].leafW);
    else if (myVar == dailyWaterTableDepth)
        return (obsDataD[i].waterTable);
    else
        return NODATA;
}


float Crit3DMeteoPoint::getMeteoPointValueD(const Crit3DDate &myDate, meteoVariable myVar) const
{
    //check
    if (myVar == noMeteoVar) return NODATA;
    if (nrObsDataDaysD == 0) return NODATA;

    int index = obsDataD[0].date.daysTo(myDate);
    if ((index < 0) || (index >= nrObsDataDaysD)) return NODATA;

    unsigned i = unsigned(index);

    if (myVar == dailyAirTemperatureMax)
        return (obsDataD[i].tMax);
    else if (myVar == dailyAirTemperatureMin)
        return (obsDataD[i].tMin);
    else if (myVar == dailyAirTemperatureAvg)
        return obsDataD[i].tAvg;
    else if (myVar == dailyPrecipitation)
        return (obsDataD[i].prec);
    else if (myVar == dailyAirRelHumidityMax)
        return (obsDataD[i].rhMax);
    else if (myVar == dailyAirRelHumidityMin)
        return float(obsDataD[i].rhMin);
    else if (myVar == dailyAirRelHumidityAvg)
        return (obsDataD[i].rhAvg);
    else if (myVar == dailyGlobalRadiation)
        return (obsDataD[i].globRad);
    else if (myVar == dailyReferenceEvapotranspirationHS)
        return obsDataD[i].et0_hs;
    else if (myVar == dailyReferenceEvapotranspirationPM)
        return (obsDataD[i].et0_pm);
    else if (myVar == dailyHeatingDegreeDays)
        return obsDataD[i].dd_heating;
    else if (myVar == dailyCoolingDegreeDays)
        return obsDataD[i].dd_cooling;
    else if (myVar == dailyWindScalarIntensityAvg)
        return (obsDataD[i].windScalIntAvg);
    else if (myVar == dailyWindScalarIntensityMax)
        return (obsDataD[i].windScalIntMax);
    else if (myVar == dailyWindVectorIntensityAvg)
        return (obsDataD[i].windVecIntAvg);
    else if (myVar == dailyWindVectorIntensityMax)
        return (obsDataD[i].windVecIntMax);
    else if (myVar == dailyWindVectorDirectionPrevailing)
        return (obsDataD[i].windVecDirPrev);
    else if (myVar == dailyLeafWetness)
        return (obsDataD[i].leafW);
    else if (myVar == dailyWaterTableDepth)
        return (obsDataD[i].waterTable);
    else
        return NODATA;
}

float Crit3DMeteoPoint::getMeteoPointValueM(const Crit3DDate &myDate, meteoVariable myVar)
{
    //check
    if (myVar == noMeteoVar) return NODATA;
    if (nrObsDataDaysM == 0) return NODATA;

    int index;
    if (myDate.year == obsDataM[0]._year)
    {
        // same year of first data
        index = myDate.month-obsDataM[0]._month;
    }
    else if (myDate.year == obsDataM[0]._year+1)
    {
        // second year
        index = 12-obsDataM[0]._month + myDate.month;
    }
    else
    {
        // other years
        index = (myDate.year - obsDataM[0]._year -1)*12+(12-obsDataM[0]._month) + myDate.month;
    }
    if ((index < 0) || (index >= nrObsDataDaysM)) return NODATA;

    unsigned i = unsigned(index);

    if (myVar == monthlyAirTemperatureMax)
        return (obsDataM[i].tMax);
    else if (myVar == monthlyAirTemperatureMin)
        return (obsDataM[i].tMin);
    else if (myVar == monthlyAirTemperatureAvg)
        return obsDataM[i].tAvg;
    else if (myVar == monthlyPrecipitation)
        return (obsDataM[i].prec);
    else if (myVar == monthlyReferenceEvapotranspirationHS)
        return (obsDataM[i].et0_hs);
    else if (myVar == monthlyGlobalRadiation)
        return (obsDataM[i].globRad);
    else if (myVar == monthlyBIC)
        return (obsDataM[i].bic);
    else
        return NODATA;
}


float Crit3DMeteoPoint::getMeteoPointValue(const Crit3DTime& myTime, meteoVariable myVar, Crit3DMeteoSettings* meteoSettings)
{
    frequencyType frequency = getVarFrequency(myVar);
    if (frequency == hourly)
        return getMeteoPointValueH(myTime.date, myTime.getHour(), myTime.getMinutes(), myVar);
    else if (frequency == daily)
        return getMeteoPointValueD(myTime.date, myVar, meteoSettings);
    else
        return NODATA;
}

float Crit3DMeteoPoint::getProxyValue(unsigned pos)
{
    if (pos < proxyValues.size())
        return proxyValues[pos];
    else
        return NODATA;
}

std::vector <double> Crit3DMeteoPoint::getProxyValues()
{
    std::vector <double> myValues;
    for (unsigned int i=0; i < proxyValues.size(); i++)
        myValues.push_back(getProxyValue(i));

    return myValues;
}

bool Crit3DMeteoPoint::computeHourlyDerivedVar(const Crit3DTime &dateTime, meteoVariable myVar, bool useNetRad)
{
    Crit3DDate myDate = dateTime.date;
    int myHour = dateTime.getHour();
    float value = NODATA;
    short valueShort = NODATA;

    if (myVar == leafWetness)
    {
        if (computeLeafWetness(getMeteoPointValueH(myDate, myHour, 0, precipitation),
                               getMeteoPointValueH(myDate, myHour, 0, airRelHumidity), &valueShort))
            setMeteoPointValueH(myDate, myHour, 0, leafWetness, float(valueShort));
    }
    else if (myVar == referenceEvapotranspiration)
    {
        if (useNetRad)
        {
            value = float(ET0_Penman_hourly_net_rad(double(point.z),
                                                  double(getMeteoPointValueH(myDate, myHour, 0, netIrradiance)),
                                                  double(getMeteoPointValueH(myDate, myHour, 0, airTemperature)),
                                                  double(getMeteoPointValueH(myDate, myHour, 0, airRelHumidity)),
                                                  double(getMeteoPointValueH(myDate, myHour, 0, windScalarIntensity))));
        }
        else
        {
            // TODO improve transmissivity
            value = float(ET0_Penman_hourly(double(point.z),
                                    double(getMeteoPointValueH(myDate, myHour, 0, atmTransmissivity) / float(0.75)),
                                    double(getMeteoPointValueH(myDate, myHour, 0, globalIrradiance)),
                                    double(getMeteoPointValueH(myDate, myHour, 0, airTemperature)),
                                    double(getMeteoPointValueH(myDate, myHour, 0, airRelHumidity)),
                                    double(getMeteoPointValueH(myDate, myHour, 0, windScalarIntensity))));
        }
    }

    setMeteoPointValueH(myDate, myHour, 0, myVar, value);

    return true;
}

bool Crit3DMeteoPoint::computeDailyDerivedVar(const Crit3DDate &date, meteoVariable myVar, Crit3DMeteoSettings& meteoSettings)
{
    float value;

    if (myVar == dailyReferenceEvapotranspirationHS)
    {
        value = dailyEtpHargreaves(getMeteoPointValueD(date, dailyAirTemperatureMin),
                               getMeteoPointValueD(date, dailyAirTemperatureMax),
                               date, latitude, &meteoSettings);

        setMeteoPointValueD(date, dailyReferenceEvapotranspirationHS, value);
    }

    return true;
}


bool Crit3DMeteoPoint::computeMonthlyAggregate(const Crit3DDate &firstDate, const Crit3DDate &lastDate, meteoVariable dailyMeteoVar,
                                               Crit3DMeteoSettings* meteoSettings, Crit3DQuality* qualityCheck,
                                               Crit3DClimateParameters* climateParam)
{
    int currentMonth = firstDate.month;
    int nrDays = getDaysInMonth(currentMonth, firstDate.year);

    double sum = 0;
    int nrValid = 0;
    int indexMonth = 0;
    bool aggregateDailyInMonthly = false;

    for (Crit3DDate actualDate = firstDate; actualDate<=lastDate; ++actualDate)
    {
        float dailyValue = getMeteoPointValueD(actualDate, dailyMeteoVar, meteoSettings);
        quality::qualityType qualityT = qualityCheck->checkFastValueDaily_SingleValue(dailyMeteoVar, climateParam,
                                                                                      dailyValue, currentMonth, float(point.z));
        if (qualityT == quality::accepted)
        {
            sum += dailyValue;
            nrValid++;
        }
        if (actualDate.day == nrDays || actualDate == lastDate)
        {
            indexMonth++;
            float dataPercentage = float(nrValid) / float(nrDays) * 100;
            if (dataPercentage >= meteoSettings->getMinimumPercentage())
            {
                aggregateDailyInMonthly = true;
                if (dailyMeteoVar == dailyAirTemperatureMin || dailyMeteoVar == dailyAirTemperatureMax
                    || dailyMeteoVar == dailyAirTemperatureAvg)
                {
                    if (nrValid != 0)
                    {
                        setMeteoPointValueM(actualDate, updateMeteoVariable(dailyMeteoVar, monthly), float(sum / nrValid));
                    }
                    else
                    {
                        setMeteoPointValueM(actualDate,updateMeteoVariable(dailyMeteoVar, monthly), NODATA);
                    }
                }
                else if (dailyMeteoVar == dailyPrecipitation || dailyMeteoVar == dailyReferenceEvapotranspirationHS
                         || dailyMeteoVar == dailyReferenceEvapotranspirationPM || dailyMeteoVar == dailyGlobalRadiation
                         || dailyMeteoVar == dailyBIC)
                {
                    setMeteoPointValueM(actualDate, updateMeteoVariable(dailyMeteoVar, monthly), float(sum));
                }
                else if (updateMeteoVariable(dailyMeteoVar, monthly) != noMeteoVar)
                {
                    // default: average
                    setMeteoPointValueM(actualDate, updateMeteoVariable(dailyMeteoVar, monthly), float(sum / nrValid));
                }
            }
            else
            {
                setMeteoPointValueM(actualDate,updateMeteoVariable(dailyMeteoVar, monthly), NODATA);
            }
            sum = 0;
            nrValid = 0;
            currentMonth = actualDate.addDays(1).month;
            nrDays = getDaysInMonth(currentMonth, actualDate.year);
        }
    }

    return aggregateDailyInMonthly;
}


bool Crit3DMeteoPoint::getDailyDataCsv_TPrec(std::string &outStr)
{
    if (obsDataD.size() == 0)
        return false;

    outStr = "Date, Tmin (C), Tmax (C), Tavg (C), Prec (mm)\n";

    std::ostringstream valueStream;
    for (int i = 0; i < int(obsDataD.size()); i++)
    {
        // Date
        outStr += obsDataD[i].date.toISOString() + ",";

        if (obsDataD[i].tMin != NODATA)
        {
            valueStream << std::setprecision(1) << obsDataD[i].tMin;
            outStr += valueStream.str();
        }
        outStr += ",";

        if (obsDataD[i].tMax != NODATA)
        {
            valueStream << std::setprecision(1) << obsDataD[i].tMax;
            outStr += valueStream.str();
        }
        outStr += ",";

        if (obsDataD[i].tAvg != NODATA)
        {
            valueStream << std::setprecision(1) << obsDataD[i].tAvg;
            outStr += valueStream.str();
        }
        outStr += ",";

        if (obsDataD[i].prec != NODATA)
        {
            valueStream << std::setprecision(1) << obsDataD[i].prec;
            outStr += valueStream.str();
        }
        outStr += "\n";
    }

    return true;
}


float Crit3DMeteoPoint::getPercValueVariable(const Crit3DDate &firstDate, const Crit3DDate &lastDate, meteoVariable dailyMeteoVar)
{
    Crit3DQuality qualityCheck;
    int nrValidValues = 0;
    int nrTotValues = 0;

    for (Crit3DDate myDate = firstDate; myDate <= lastDate; ++myDate)
    {
        nrTotValues = nrTotValues + 1;
        float value = getMeteoPointValueD(myDate, dailyMeteoVar);
        quality::qualityType qualityT = qualityCheck.syntacticQualitySingleValue(dailyMeteoVar, value);
        if (qualityT == quality::accepted)
        {
            nrValidValues = nrValidValues + 1;
        }
    }

    float percValue = float(nrValidValues) / float(nrTotValues);
    return percValue;
}

// ---- end class


bool isSelectionPointsActive(Crit3DMeteoPoint* meteoPoints,int nrMeteoPoints)
{
    for (int i = 0; i < nrMeteoPoints; i++)
    {
        if (meteoPoints[i].selected)
            return true;
    }

    return false;
}
