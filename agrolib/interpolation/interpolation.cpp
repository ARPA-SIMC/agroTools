/*!
    \copyright 2016 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by A.R.P.A. Emilia-Romagna

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

#include <stdlib.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <functional>

#include "commonConstants.h"
#include "basicMath.h"
#include "furtherMathFunctions.h"
#include "statistics.h"
#include "basicMath.h"
#include "meteoPoint.h"
#include "gis.h"
#include "spatialControl.h"
#include "interpolationPoint.h"
#include "interpolation.h"
#include "interpolationSettings.h"
#include "meteo.h"


using namespace std;

float getMinHeight(const std::vector<Crit3DInterpolationDataPoint> &myPoints, bool useLapseRateCode)
{
    float myZmin = NODATA;

    for (unsigned i = 0; i < myPoints.size(); i++)
        if (myPoints[i].point->z != NODATA && myPoints[i].isActive && checkLapseRateCode(myPoints[i].lapseRateCode, useLapseRateCode, true))
            if (myZmin == NODATA || myPoints[i].point->z < myZmin)
                myZmin = float(myPoints[i].point->z);
    return myZmin;
}

float getMaxHeight(const std::vector<Crit3DInterpolationDataPoint> &myPoints, bool useLapseRateCode)
{
    float zMax;
    zMax = NODATA;

    for (unsigned i = 0; i < myPoints.size(); i++)
        if (myPoints[i].value != NODATA && myPoints[i].isActive && checkLapseRateCode(myPoints[i].lapseRateCode, useLapseRateCode, true))
            if (zMax == NODATA || (myPoints[i]).point->z > zMax)
                zMax = float(myPoints[i].point->z);

    return zMax;
}

float getZmin(const std::vector<Crit3DInterpolationDataPoint> &myPoints)
{
    float myZmin = NODATA;

    for (unsigned i = 0; i < myPoints.size(); i++)
        if (myPoints[i].point->z != NODATA)
            if (myZmin == NODATA || myPoints[i].point->z < myZmin)
                myZmin = float(myPoints[i].point->z);
    return myZmin;
}

float getZmax(const std::vector<Crit3DInterpolationDataPoint> &myPoints)
{
    float myZmax = 0;

    for (unsigned i = 0; i < myPoints.size(); i++)
        if (myPoints[i].point->z > myZmax)
            myZmax = float(myPoints[i].point->z);
    return myZmax;
}

float getProxyMaxValue(std::vector<Crit3DInterpolationDataPoint> &myPoints, unsigned pos)
{
    float maxValue = NODATA;

    for (unsigned i = 0; i < myPoints.size(); i++)
        if (myPoints[i].getProxyValue(pos) != NODATA)
            if (maxValue == NODATA || myPoints[i].getProxyValue(pos) > maxValue)
                maxValue = myPoints[i].getProxyValue(pos);

    return maxValue;
}

float getProxyMinValue(std::vector<Crit3DInterpolationDataPoint> &myPoints, unsigned pos)
{
    float minValue = NODATA;

    for (unsigned i = 0; i < myPoints.size(); i++)
        if (myPoints[i].getProxyValue(pos) != NODATA)
            if (minValue == NODATA || myPoints[i].getProxyValue(pos) < minValue)
                minValue = myPoints[i].getProxyValue(pos);

    return minValue;
}


// return number of sorted valid points
unsigned sortPointsByDistance(unsigned maxNrPoints, std::vector<Crit3DInterpolationDataPoint> &pointList,
                              std::vector<Crit3DInterpolationDataPoint> &validPointList)
{
    if (pointList.empty())
        return 0;

    // initializes the distance list
    std::vector<float> distanceList;
    for (unsigned i = 0; i < pointList.size(); i++)
    {
        if (! isEqual(pointList[i].distance, 0) && ! isEqual(pointList[i].distance, NODATA))
        {
            distanceList.push_back(pointList[i].distance);
        }
    }
    // check
    if (distanceList.empty())
        return 0;

    // sorts the distances
    std::sort(distanceList.begin(), distanceList.end());

    // initializes the list of points used
    std::vector<bool> isUsedPoint;
    isUsedPoint.resize(pointList.size());
    for (unsigned i = 0; i < isUsedPoint.size(); i++)
    {
        isUsedPoint[i] = false;
    }

    // saves the sorted points
    validPointList.clear();
    for (unsigned i = 0; i < distanceList.size(); i++)
    {
        float currentDistance = distanceList[i];
        for (unsigned i = 0; i < pointList.size(); i++)
        {
            if (! isUsedPoint[i])
            {
                if (isEqual(pointList[i].distance, currentDistance))
                {
                    validPointList.push_back(pointList[i]);
                    isUsedPoint[i] = true;
                    break;
                }
            }
        }
        if (validPointList.size() == maxNrPoints)
            break;
    }

    return unsigned(validPointList.size());
}

/*
 * old code
unsigned sortPointsByDistance(unsigned maxIndex, std::vector<Crit3DInterpolationDataPoint> &points, std::vector<Crit3DInterpolationDataPoint> &myValidPoints)
{
    if (points.size() == 0) return 0;

    unsigned i, first, index, outIndex;
    float min_value = NODATA;
    std::vector<unsigned> indici_ordinati;
    std::vector<unsigned> indice_minimo;

    indici_ordinati.resize(maxIndex);
    indice_minimo.resize(points.size());

    first = 0;
    index = 0;

    bool isExit = false;

    while (index < maxIndex && !isExit)
    {
        if (first == 0)
        {
            i = 0;
            while ((! points[i].isActive || (isEqual(points[i].distance, 0))) && (i < points.size()-1))
                i++;

            if (i == (points.size()-1) && ! points[i].isActive)
                isExit = true;
            else
            {
                first = 1;
                indice_minimo[first-1] = i;
                min_value = points[i].distance;
            }
        }
        else
        {
            min_value = points[unsigned(indice_minimo[first-1])].distance;
        }

        if (! isExit)
        {
            for (i = unsigned(indice_minimo[first-1]) + 1; i < points.size(); i++)
                if (isEqual(min_value, NODATA) || points[i].distance < min_value)
                    if (points[i].isActive)
                        if (points[i].distance > 0)
                        {
                            first++;
                            min_value = points[i].distance;
                            indice_minimo[first-1] = i;
                        }

            int currentIndex = indice_minimo[first-1];
            if (! isEqual(points[currentIndex].distance, 0))
            {
                indici_ordinati[index] = currentIndex;
                points[currentIndex].isActive = false;
                index++;
                first--;
            }
        }
    }

    outIndex = MINVALUE(index, maxIndex);
    myValidPoints.clear();

    for (i=0; i < outIndex; i++)
    {
        points[indici_ordinati[i]].isActive = true;
        myValidPoints.push_back(points[indici_ordinati[i]]);
    }

    indici_ordinati.clear();
    indice_minimo.clear();
    return outIndex;
}
*/


void computeDistances(meteoVariable myVar, vector <Crit3DInterpolationDataPoint> &myPoints,  Crit3DInterpolationSettings* mySettings,
                      float x, float y, float z, bool excludeSupplemental)
{
    int row, col;

    for (unsigned long i = 0; i < myPoints.size() ; i++)
    {
        if (excludeSupplemental && ! checkLapseRateCode(myPoints[i].lapseRateCode, mySettings->getUseLapseRateCode(), false))
        {
            myPoints[i].distance = 0;
        }
        else
        {
            myPoints[i].distance = gis::computeDistance(x, y, float(myPoints[i].point->utm.x), float(myPoints[i].point->utm.y));

            if (mySettings->getUseTD() && getUseTdVar(myVar))
            {
                float topoDistance = 0.;
                int kh = mySettings->getTopoDist_Kh();
                if (kh != 0)
                {
                    topoDistance = NODATA;
                    if (myPoints[i].topographicDistance != nullptr)
                    {
                        if (! gis::isOutOfGridXY(x, y, myPoints[i].topographicDistance->header))
                        {
                            gis::getRowColFromXY(*(myPoints[i].topographicDistance->header), x, y, &row, &col);
                            topoDistance = myPoints[i].topographicDistance->value[row][col];
                        }
                    }

                    if (isEqual(topoDistance, NODATA))
                        topoDistance = topographicDistance(x, y, z, float(myPoints[i].point->utm.x),
                                                           float(myPoints[i].point->utm.y),
                                                           float(myPoints[i].point->z), myPoints[i].distance,
                                                           *(mySettings->getCurrentDEM()));
                }

                myPoints[i].distance += (kh * topoDistance);
            }
        }
    }
}


bool neighbourhoodVariability(meteoVariable myVar, std::vector <Crit3DInterpolationDataPoint> &myInterpolationPoints,
                              Crit3DInterpolationSettings* mySettings,
                              float x, float y, float z, int maxNrPoints,
                              float* devSt, float* avgDeltaZ, float* minDistance)
{
    int i, nrValid;
    float* dataNeighborhood;
    float myValue;
    vector <float> deltaZ;
    vector <Crit3DInterpolationDataPoint> validPoints;

    computeDistances(myVar, myInterpolationPoints, mySettings, x, y, z, true);
    nrValid = sortPointsByDistance(maxNrPoints, myInterpolationPoints, validPoints);

    if (nrValid > 1)
    {
        dataNeighborhood = (float *) calloc (nrValid, sizeof(float));

        for (i=0; i<nrValid; i++)
        {
            myValue = validPoints[i].value;
            dataNeighborhood[i] = myValue;
        }

        *devSt = statistics::standardDeviation(dataNeighborhood, nrValid);

        *minDistance = validPoints[0].distance;

        deltaZ.clear();
        if (z != NODATA)
            deltaZ.push_back(1);

        for (i=0; i<nrValid;i++)
        {
            if ((validPoints[i]).point->z != NODATA)
            {
                deltaZ.push_back(float(fabs(validPoints[i].point->z - z)));
            }
        }

        *avgDeltaZ = statistics::mean(deltaZ.data(), int(deltaZ.size()));

        return true;
    }
    else
        return false;
}


bool regressionSimple(std::vector <Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* mySettings,
                      unsigned proxyPosition, bool isZeroIntercept, float* myCoeff, float* myIntercept, float* myR2)
{
    unsigned i;
    float myProxyValue;
    Crit3DInterpolationDataPoint myPoint;
    vector <float> myValues, myZ;

    *myCoeff = NODATA;
    *myIntercept = NODATA;
    *myR2 = NODATA;

    myValues.clear();
    myZ.clear();

    for (i = 0; i < myPoints.size(); i++)
    {
        myPoint = myPoints[i];
        if (myPoint.isActive)
        {
            if (proxyPosition != mySettings->getIndexHeight() || checkLapseRateCode(myPoint.lapseRateCode, mySettings->getUseLapseRateCode(), true))
            {
                myProxyValue = myPoint.getProxyValue(proxyPosition);
                if (! isEqual(myProxyValue, NODATA))
                {
                    myValues.push_back(myPoint.value);
                    myZ.push_back(myProxyValue);
                }
            }
        }
    }

    if (myValues.size() >= MIN_REGRESSION_POINTS)
    {
        statistics::linearRegression((float*)(myZ.data()), (float*)(myValues.data()), (long)(myZ.size()), isZeroIntercept,
                                     myIntercept, myCoeff, myR2);
        return true;
    }
    else
        return false;
}

bool regressionGeneric(std::vector <Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* mySettings,
                       int proxyPos, bool isZeroIntercept)
{
    float q, m, r2;

    if (! regressionSimple(myPoints, mySettings, proxyPos, isZeroIntercept, &m, &q, &r2))
        return false;

    Crit3DProxy* myProxy = mySettings->getProxy(proxyPos);
    myProxy->setRegressionSlope(m);
    myProxy->setRegressionIntercept(q);
    myProxy->setRegressionR2(r2);
    myProxy->setLapseRateT0(q);

    // clean inversion (only thermal variables)
    myProxy->setInversionIsSignificative(false);
    myProxy->setInversionLapseRate(NODATA);

    return (r2 >= mySettings->getMinRegressionR2());
}


bool regressionSimpleT(std::vector <Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* mySettings, Crit3DClimateParameters* myClimate,
                       Crit3DTime myTime, meteoVariable myVar, unsigned orogProxyPos)
{
    float slope, t0, r2;

    Crit3DProxy* myProxyOrog = mySettings->getProxy(orogProxyPos);
    myProxyOrog->initializeOrography();

    if (! regressionSimple(myPoints, mySettings, orogProxyPos, false, &slope, &t0, &r2))
        return false;

    if (r2 < mySettings->getMinRegressionR2())
        return false;

    myProxyOrog->setRegressionSlope(slope);
    myProxyOrog->setLapseRateT0(t0);
    myProxyOrog->setRegressionR2(r2);

    // only pre-inversion data
    if (slope > 0 && myVar != elaboration)
    {
        myProxyOrog->setInversionLapseRate(slope);

        float maxZ = MINVALUE(getMaxHeight(myPoints, mySettings->getUseLapseRateCode()), mySettings->getMaxHeightInversion());
        myProxyOrog->setLapseRateT1(t0 + slope * maxZ);
        myProxyOrog->setLapseRateH1(maxZ);
        myProxyOrog->setRegressionSlope(myClimate->getClimateLapseRate(myVar, myTime));
        myProxyOrog->setInversionIsSignificative(true);
    }
    else
    {
        myProxyOrog->setInversionIsSignificative(false);
        myProxyOrog->setInversionLapseRate(NODATA);
    }

    return true;
}


float findHeightIntervalAvgValue(bool useLapseRateCode, std::vector <Crit3DInterpolationDataPoint> &myPoints,
                                 float heightInf, float heightSup, float maxPointsZ)
{
    float myValue, mySum, nValues;

    mySum = 0.;
    nValues = 0;

    for (long i = 0; i < long(myPoints.size()); i++)
        if (myPoints[i].point->z != NODATA && myPoints[i].isActive && checkLapseRateCode(myPoints[i].lapseRateCode, useLapseRateCode, true))
            if (myPoints[i].point->z >= heightInf && myPoints[i].point->z <= heightSup)
            {
                myValue = (myPoints[i]).value;
                if (myValue != NODATA)
                {
                    mySum += myValue;
                    nValues ++;
                }
            }

    if (nValues > 1 || (nValues > 0 && heightSup >= maxPointsZ))
        return (mySum / nValues);
    else
        return NODATA;
}

bool regressionOrographyT(std::vector <Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* mySettings, Crit3DClimateParameters* myClimate,
                          Crit3DTime myTime, meteoVariable myVar, int orogProxyPos, bool climateExists)
{
    long i;
    float heightInf, heightSup;
    float myAvg;
    vector <float> myData1, myData2;
    vector <float> myHeights1, myHeights2;
    vector <float> myIntervalsHeight, myIntervalsHeight1, myIntervalsHeight2;
    vector <float> myIntervalsValues, myIntervalsValues1, myIntervalsValues2;
    float m, q, r2;
    float r2_values, r2_intervals;
    float q1, m1, r21, q2, m2, r22;
    float mySignificativeR2, mySignificativeR2Inv;
    float maxPointsZ, deltaZ;
    float climateLapseRate;
    float x,y;
    float DELTAZ_INI = 80.;
    float maxHeightInv = mySettings->getMaxHeightInversion();

    Crit3DProxy* myProxyOrog = mySettings->getProxy(orogProxyPos);

    mySignificativeR2 = MAXVALUE(mySettings->getMinRegressionR2(), float(0.2));
    mySignificativeR2Inv = MAXVALUE(mySettings->getMinRegressionR2(), float(0.1));

    /*! initialize */
    myProxyOrog->initializeOrography();

    if (climateExists)
        climateLapseRate = myClimate->getClimateLapseRate(myVar, myTime);
    else
        climateLapseRate = 0.;

    myProxyOrog->setRegressionSlope(climateLapseRate);

    maxPointsZ = getMaxHeight(myPoints, mySettings->getUseLapseRateCode());
    heightInf = getMinHeight(myPoints, mySettings->getUseLapseRateCode());

    /*! not enough data to define a curve (use climate) */
    if ((maxPointsZ == heightInf) || (myPoints.size() < MIN_REGRESSION_POINTS))
        return true;

    /*! find intervals averages */

    heightSup = heightInf;
    deltaZ = DELTAZ_INI;
    while (heightSup <= maxPointsZ)
    {
        myAvg = NODATA;
        while (myAvg == NODATA)
        {
            heightSup = heightSup + deltaZ;
            myAvg = findHeightIntervalAvgValue(mySettings->getUseLapseRateCode(), myPoints, heightInf, heightSup, maxPointsZ);
        }
        myIntervalsHeight.push_back((heightSup + heightInf) / float(2.));
        myIntervalsValues.push_back(myAvg);

        deltaZ = DELTAZ_INI * expf(heightInf / maxHeightInv);
        heightInf = heightSup;
    }

    /*! find inversion height */
    myProxyOrog->setLapseRateT1(myIntervalsValues[0]);
    myProxyOrog->setLapseRateH1(myIntervalsHeight[0]);
    for (i = 1; i < long(myIntervalsValues.size()); i++)
        if (myIntervalsHeight[i] <= maxHeightInv && (myIntervalsValues[i] >= myProxyOrog->getLapseRateT1()) && (myIntervalsValues[i] > (myIntervalsValues[0] + 0.001 * (myIntervalsHeight[i] - myIntervalsHeight[0]))))
        {
            myProxyOrog->setLapseRateH1(myIntervalsHeight[i]);
            myProxyOrog->setLapseRateT1(myIntervalsValues[i]);
            myProxyOrog->setInversionIsSignificative(true);
        }

    /*! no inversion: try regression with all data */
    if (! myProxyOrog->getInversionIsSignificative())
        return (regressionGeneric(myPoints, mySettings, orogProxyPos, false));

    /*! create vectors below and above inversion */
    for (i = 0; i < long(myPoints.size()); i++)
        if (myPoints[i].point->z != NODATA && checkLapseRateCode(myPoints[i].lapseRateCode, mySettings->getUseLapseRateCode(), true))
        {
            if (myPoints[i].point->z <= myProxyOrog->getLapseRateH1())
            {
                myData1.push_back(myPoints[i].value);
                myHeights1.push_back(float(myPoints[i].point->z));
            }
            else
            {
                myData2.push_back(myPoints[i].value);
                myHeights2.push_back(float(myPoints[i].point->z));
            }
        }

    /*! create vectors of height intervals below and above inversion */
    for (i = 0; i < long(myIntervalsValues.size()); i++)
        if (myIntervalsHeight[i] <= myProxyOrog->getLapseRateH1())
        {
            myIntervalsValues1.push_back(myIntervalsValues[i]);
            myIntervalsHeight1.push_back(myIntervalsHeight[i]);
        }
        else
        {
            myIntervalsValues2.push_back(myIntervalsValues[i]);
            myIntervalsHeight2.push_back(myIntervalsHeight[i]);
        }


    /*! only positive lapse rate*/
    if (myProxyOrog->getInversionIsSignificative() && myIntervalsValues1.size() == myIntervalsValues.size())
    {
        if (! regressionSimple(myPoints, mySettings, orogProxyPos, false, &m, &q, &r2))
            return false;

        if (r2 >= mySignificativeR2)
        {
            myProxyOrog->setInversionLapseRate(m);
            myProxyOrog->setRegressionR2(r2);
            myProxyOrog->setLapseRateT0(q);
            myProxyOrog->setLapseRateT1(q + m * myProxyOrog->getLapseRateH1());
        }
        else
        {
            statistics::linearRegression(myIntervalsHeight1.data(), myIntervalsValues1.data(), (long)myIntervalsHeight1.size(), false, &q, &m, &r2);

            myProxyOrog->setRegressionR2(NODATA);

            if (r2 >= mySignificativeR2)
            {
                myProxyOrog->setLapseRateT0(q);
                myProxyOrog->setInversionLapseRate(m);
                myProxyOrog->setLapseRateT1(q + m * myProxyOrog->getLapseRateH1());
            }
            else
            {
                myProxyOrog->setInversionLapseRate(0.);
                myProxyOrog->setLapseRateT0(myIntervalsValues[0]);
                myProxyOrog->setLapseRateT0(myIntervalsValues[0]);
            }
        }

        return true;
    }

    /*! check inversion significance */
    statistics::linearRegression(myHeights1.data(), myData1.data(), long(myHeights1.size()), false, &q1, &m1, &r2_values);
    if (myIntervalsValues1.size() > 2)
        statistics::linearRegression(myIntervalsHeight1.data(), myIntervalsValues1.data(), long(myIntervalsHeight1.size()), false, &q, &m, &r2_intervals);
    else
        r2_intervals = 0.;

    /*! inversion is not significant with data neither with intervals */
    if (r2_values < mySignificativeR2Inv && r2_intervals < mySignificativeR2Inv)
    {
        if (! regressionSimple(myPoints, mySettings, orogProxyPos, false, &m, &q, &r2))
            return false;

        /*! case 0: regression with all data much significant */
        if (r2 >= 0.5)
        {
            myProxyOrog->setInversionIsSignificative(false);
            myProxyOrog->setLapseRateH1(NODATA);
            myProxyOrog->setInversionLapseRate(NODATA);
            myProxyOrog->setRegressionSlope(MINVALUE(m, float(0.0)));
            myProxyOrog->setRegressionR2(r2);
            myProxyOrog->setLapseRateT0(q);
            myProxyOrog->setLapseRateT1(NODATA);
            return true;
        }

        /*! case 1: analysis only above inversion, flat lapse rate below */
        myProxyOrog->setInversionLapseRate(0.);
        statistics::linearRegression(myHeights2.data(), myData2.data(), long(myHeights2.size()), false, &q2, &m2, &r2);

        if (myData2.size() >= MIN_REGRESSION_POINTS)
        {
            if (r2 >= mySignificativeR2)
            {
                myProxyOrog->setRegressionSlope(MINVALUE(m2, float(0.0)));
                myProxyOrog->setLapseRateT0(q2 + myProxyOrog->getLapseRateH1() * myProxyOrog->getRegressionSlope());
                myProxyOrog->setLapseRateT1(myProxyOrog->getLapseRateT0());
                myProxyOrog->setRegressionR2(r2);
                return true;
            }
            else
            {
                statistics::linearRegression(myIntervalsHeight2.data(), myIntervalsValues2.data(),
                                             int(myIntervalsHeight2.size()), false, &q2, &m2, &r2);
                if (r2 >= mySignificativeR2)
                {
                    myProxyOrog->setRegressionSlope(MINVALUE(m2, float(0.0)));
                    myProxyOrog->setLapseRateT0(q2 + myProxyOrog->getLapseRateH1() * myProxyOrog->getRegressionSlope());
                    myProxyOrog->setLapseRateT1(myProxyOrog->getLapseRateT0());;
                    myProxyOrog->setRegressionR2(r2);
                    return true;
                }
            }
        }

        myProxyOrog->setInversionIsSignificative(false);
        myProxyOrog->setLapseRateH1(NODATA);
        myProxyOrog->setInversionLapseRate(NODATA);
        myProxyOrog->setLapseRateT0(q);
        myProxyOrog->setLapseRateT1(NODATA);

        /*! case 2: regression with data */
        if (! regressionSimple(myPoints, mySettings, orogProxyPos, false, &m, &q, &r2))
            return false;

        if (r2 >= mySignificativeR2)
        {
            myProxyOrog->setRegressionSlope(MINVALUE(m, 0));
            myProxyOrog->setLapseRateT0(q);
            myProxyOrog->setRegressionR2(r2);
            return true;
        }
        else
        {
            myProxyOrog->setLapseRateT0(myIntervalsValues[0]);
            if (m > 0.)
                myProxyOrog->setRegressionSlope(0.);
            else
                myProxyOrog->setRegressionSlope(climateLapseRate);

            return true;
        }

    }

    /*! significance analysis */
    statistics::linearRegression(myHeights1.data(), myData1.data(), int(myHeights1.size()), false, &q1, &m1, &r21);
    statistics::linearRegression(myHeights2.data(), myData2.data(), int(myHeights2.size()), false, &q2, &m2, &r22);

    if (m1 <= 0)
        r21 = 0;

    myProxyOrog->setRegressionR2(r22);

    if (r21 >= mySignificativeR2Inv && r22 >= mySignificativeR2)
    {
        if (myHeights2.size() < MIN_REGRESSION_POINTS && m2 > 0.)
        {
            m2 = 0.;
            q2 = myProxyOrog->getLapseRateT1();
        }
        findLinesIntersection(q1, m1, q2, m2, &x, &y);
        myProxyOrog->setLapseRateT0(q1);
        myProxyOrog->setLapseRateT1(y);
        myProxyOrog->setInversionLapseRate(m1);
        myProxyOrog->setRegressionSlope(m2);
        myProxyOrog->setLapseRateH1(x);
        if (myProxyOrog->getLapseRateH1() > maxHeightInv)
        {
            myProxyOrog->setLapseRateT1(myProxyOrog->getLapseRateT1() - (myProxyOrog->getLapseRateH1() - maxHeightInv) * myProxyOrog->getRegressionSlope());
            myProxyOrog->setLapseRateH1(maxHeightInv);
            myProxyOrog->setInversionLapseRate((myProxyOrog->getLapseRateT1() - myProxyOrog->getLapseRateT0()) / (myProxyOrog->getLapseRateH1() - myProxyOrog->getLapseRateH0()));
        }
        return true;
    }
    else if (r21 < mySignificativeR2Inv && r22 >= mySignificativeR2)
    {
        if (myHeights2.size() < MIN_REGRESSION_POINTS && m2 > 0.)
        {
            m2 = 0.;
            q2 = myProxyOrog->getLapseRateT1();
        }

        statistics::linearRegression(myIntervalsHeight1.data(), myIntervalsValues1.data(),
                                     long(myIntervalsHeight1.size()), false, &q, &m, &r2);

        myProxyOrog->setRegressionSlope(m2);
        if (r2 >= mySignificativeR2Inv)
        {
            if (findLinesIntersectionAboveThreshold(q, m, q2, m2, 40, &x, &y))
            {
                myProxyOrog->setLapseRateH1(x);
                myProxyOrog->setLapseRateT0(q);
                myProxyOrog->setLapseRateT1(y);
                myProxyOrog->setInversionLapseRate(m);
                if (myProxyOrog->getLapseRateH1() > maxHeightInv)
                {
                    myProxyOrog->setLapseRateT1(myProxyOrog->getLapseRateT1() - (myProxyOrog->getLapseRateH1() - maxHeightInv) * myProxyOrog->getRegressionSlope());
                    myProxyOrog->setLapseRateH1(maxHeightInv);
                    myProxyOrog->setInversionLapseRate((myProxyOrog->getLapseRateT1() - myProxyOrog->getLapseRateT0()) / (myProxyOrog->getLapseRateH1() - myProxyOrog->getLapseRateH0()));
                }
                return true;
            }
        }
        else
        {
            myProxyOrog->setInversionLapseRate(0.);
            myProxyOrog->setLapseRateT1(q2 + m2 * myProxyOrog->getLapseRateH1());
            myProxyOrog->setLapseRateT0(myProxyOrog->getLapseRateT1());
            return true;
        }
    }

    else if (r21 >= mySignificativeR2Inv && r22 < mySignificativeR2)
    {
        myProxyOrog->setLapseRateT0(q1);
        myProxyOrog->setInversionLapseRate(m1);

        statistics::linearRegression(myIntervalsHeight2.data(), myIntervalsValues2.data(),
                                     long(myIntervalsHeight2.size()), false, &q, &m, &r2);
        if (r2 >= mySignificativeR2)
        {
            myProxyOrog->setRegressionSlope(MINVALUE(m, float(0.)));
            findLinesIntersection(myProxyOrog->getLapseRateT0(), myProxyOrog->getInversionLapseRate(), q, myProxyOrog->getRegressionSlope(), &x, &y);
            myProxyOrog->setLapseRateH1(x);
            myProxyOrog->setLapseRateT1(y);
        }
        else
        {
            myProxyOrog->setRegressionSlope(climateLapseRate);
            findLinesIntersection(myProxyOrog->getLapseRateT0(), myProxyOrog->getInversionLapseRate(), myProxyOrog->getLapseRateT1() - myProxyOrog->getRegressionSlope()* myProxyOrog->getLapseRateH1(), myProxyOrog->getRegressionSlope(), &x, &y);
            myProxyOrog->setLapseRateH1(x);
            myProxyOrog->setLapseRateT1(y);
        }
        return true;
    }

    else if (r21 < mySignificativeR2Inv && r22 < mySignificativeR2)
    {
        statistics::linearRegression(myIntervalsHeight1.data(), myIntervalsValues1.data(),
                                     long(myIntervalsHeight1.size()), false, &q, &m, &r2);

        if (r2 >= mySignificativeR2Inv)
        {
            myProxyOrog->setLapseRateT0(q);
            myProxyOrog->setInversionLapseRate(m);
            myProxyOrog->setLapseRateT1(q + m * myProxyOrog->getLapseRateH1());
        }
        else
        {
            myProxyOrog->setInversionLapseRate(0.);
            myProxyOrog->setLapseRateT0(myIntervalsValues[0]);
            myProxyOrog->setLapseRateT1(myProxyOrog->getLapseRateT0());
        }

        statistics::linearRegression(myIntervalsHeight2.data(), myIntervalsValues2.data(),
                                     long(myIntervalsHeight2.size()), false, &q, &m, &r2);

        if (r2 >= mySignificativeR2)
        {
            myProxyOrog->setRegressionSlope(MINVALUE(m, 0));
            if (findLinesIntersectionAboveThreshold(myProxyOrog->getLapseRateT0(), myProxyOrog->getInversionLapseRate(), q, myProxyOrog->getRegressionSlope(), 40, &x, &y))
            {
                myProxyOrog->setLapseRateH1(x);
                myProxyOrog->setLapseRateT1(y);
                return true;
            }
        }
        else
        {
            myProxyOrog->setRegressionSlope(climateLapseRate);
            return true;
        }

    }

    /*! check max lapse rate (20 C / 1000 m) */
    if (myProxyOrog->getRegressionSlope() < -0.02)
        myProxyOrog->setRegressionSlope((float)-0.02);

    myProxyOrog->initializeOrography();
    return (regressionGeneric(myPoints, mySettings, orogProxyPos, false));

}

float computeShepardInitialRadius(float area, unsigned int allPointsNr, unsigned int minPointsNr)
{
    return float(sqrt((minPointsNr * area) / (float(PI) * allPointsNr)));
}


float shepardSearchNeighbour(vector <Crit3DInterpolationDataPoint> &inputPoints,
                             Crit3DInterpolationSettings* settings,
                             vector <Crit3DInterpolationDataPoint> &outputPoints)
{
    float shepardInitialRadius = computeShepardInitialRadius(settings->getPointsBoundingBoxArea(),
                                                             unsigned(inputPoints.size()), SHEPARD_AVG_NRPOINTS);

    // define a first neighborhood inside initial radius
    std::vector <Crit3DInterpolationDataPoint> shepardNeighbourPoints;
    for (unsigned int i=0; i < inputPoints.size(); i++)
    {
        if (inputPoints[i].distance <= shepardInitialRadius &&
            inputPoints[i].distance > 0 &&
            inputPoints[i].index != settings->getIndexPointCV())
        {
            shepardNeighbourPoints.push_back(inputPoints[i]);
        }
    }

    // If the points are too few, double the check radius
    if (shepardNeighbourPoints.size() < SHEPARD_MIN_NRPOINTS)
    {
        float doubleRadius = shepardInitialRadius * 2;
        for (unsigned int i=0; i < inputPoints.size(); i++)
        {
            if (inputPoints[i].distance <= doubleRadius &&
                inputPoints[i].distance > shepardInitialRadius &&
                inputPoints[i].index != settings->getIndexPointCV())
            {
                shepardNeighbourPoints.push_back(inputPoints[i]);
            }
        }
        shepardInitialRadius = doubleRadius;
    }

    float radius;

    if (shepardNeighbourPoints.size() < SHEPARD_MIN_NRPOINTS)
    {
        int nrPoints = sortPointsByDistance(SHEPARD_MIN_NRPOINTS, inputPoints, outputPoints);
        if (outputPoints.empty()) return NODATA;
        radius = outputPoints[nrPoints-1].distance + float(EPSILON);
    }
    else if (shepardNeighbourPoints.size() > SHEPARD_MAX_NRPOINTS)
    {
        int nrPoints = sortPointsByDistance(SHEPARD_MAX_NRPOINTS, shepardNeighbourPoints, outputPoints);
        radius = outputPoints[nrPoints-1].distance + float(EPSILON);
    }
    else
    {
        outputPoints = shepardNeighbourPoints;
        radius = shepardInitialRadius;
    }

    return radius;
}


float shepardIdw(vector <Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* settings, float X, float Y)
{
    std::vector <Crit3DInterpolationDataPoint> shepardValidPoints;

    float radius = shepardSearchNeighbour(myPoints, settings, shepardValidPoints);

    unsigned int i, j;
    float weightSum, radius_27_4, radius_3, tmp, cosine, result;
    std::vector <float> weight, t, S;

    unsigned int nrValid = unsigned(shepardValidPoints.size());

    weight.resize(nrValid);
    t.resize(nrValid);
    S.resize(nrValid);

    weightSum = 0;
    radius_3 = radius / 3;
    radius_27_4 = (27 / 4) / radius;
    for (i=0; i < nrValid; i++)
        if (shepardValidPoints[i].distance > 0)
        {
            if (shepardValidPoints[i].distance <= radius_3)
                S[i] = 1 / (shepardValidPoints[i].distance);
            else if (shepardValidPoints[i].distance <= radius)
            {
                tmp = (shepardValidPoints[i].distance / radius) - 1;
                S[i] = radius_27_4 * tmp * tmp;
            }
            else
                S[i] = 0;

            weightSum = weightSum + S[i];
        }

    if (weightSum == 0)
        return NODATA;

    // including direction
    for (i=0; i < nrValid; i++)
    {
        t[i] = 0;
        for (j=0; j < nrValid; j++)
            if (i != j)
            {
                cosine = ((X - (float)shepardValidPoints[i].point->utm.x) * (X - (float)shepardValidPoints[j].point->utm.x) + (Y - (float)shepardValidPoints[i].point->utm.y) * (Y - (float)shepardValidPoints[j].point->utm.y)) / (shepardValidPoints[i].distance * shepardValidPoints[j].distance);
                t[i] = t[i] + S[j] * (1 - cosine);
            }

        if (weightSum != 0)
            t[i] /= weightSum;
    }

    // weights
    weightSum = 0;
    for (i=0; i < nrValid; i++)
    {
        weight[i] = S[i] * S[i] * (1 + t[i]);
        weightSum += weight[i];
    }
    for (i=0; i < nrValid; i++)
        weight[i] /= weightSum;

    result = 0;
    for (i=0; i < nrValid; i++)
        result += weight[i] * shepardValidPoints[i].value;

    return result;
}


float modifiedShepardIdw(vector <Crit3DInterpolationDataPoint> &myPoints,
                         Crit3DInterpolationSettings* settings, float radius, float X, float Y)
{
    float weightSum, cosine, result;
    std::vector <float> weight, t, S;
    std::vector <Crit3DInterpolationDataPoint> validPoints;

    if (radius == NODATA)
    {
        radius = shepardSearchNeighbour(myPoints, settings, validPoints);
        /*settings->setMinPointsLocalDetrending(8);
        localSelection(myPoints, validPoints, X, Y, *settings, true);
        radius = settings->getLocalRadius() + EPSILON;*/
    }
    else
        validPoints = myPoints;

    if (myPoints.empty())
        return NODATA;

    weight.resize(validPoints.size());
    t.resize(validPoints.size());
    S.resize(validPoints.size());

    weightSum = 0;
    for (unsigned int i=0; i < validPoints.size(); i++)
    {
        if (validPoints[i].distance > 0)
        {
            if (validPoints[i].distance <= radius)
                S[i] = (radius - validPoints[i].distance) / (radius * validPoints[i].distance);
            else
                S[i] = 0;

            weightSum += S[i];
        }
    }

    if (weightSum == 0)
    {
        return NODATA;
    }

    // including direction
    for (unsigned int i=0; i < validPoints.size(); i++)
    {
        t[i] = 0;
        for (unsigned int j=0; j < validPoints.size(); j++)
        {
            if (i != j && S[i] > 0 && validPoints[i].distance > 0 && validPoints[j].distance > 0)
            {
                cosine = ((X - (float)validPoints[i].point->utm.x) * (X - (float)validPoints[j].point->utm.x)
                          + (Y - (float)validPoints[i].point->utm.y) * (Y - (float)validPoints[j].point->utm.y))
                         / (validPoints[i].distance * validPoints[j].distance);
                t[i] = t[i] + S[j] * (1 - cosine);
            }
        }

        if (weightSum != 0)
            t[i] /= weightSum;
    }

    // weights
    weightSum = 0;
    for (unsigned int i=0; i < validPoints.size(); i++)
    {
        weight[i] = S[i] * S[i] * (1 + t[i]);
        weightSum += weight[i];
    }
    for (unsigned int i=0; i < validPoints.size(); i++)
        weight[i] /= weightSum;

    result = 0;
    for (unsigned int i=0; i < validPoints.size(); i++)
        result += weight[i] * validPoints[i].value;

    return result;
}


float inverseDistanceWeighted(vector <Crit3DInterpolationDataPoint> &myPointList)
{
    double sum, sumWeights, weight;

    sum = 0 ;
    sumWeights = 0 ;
    for (unsigned int i = 0 ; i < myPointList.size(); i++)
    {
        if (myPointList[i].distance > 0.f)
        {
            weight = double(myPointList[i].distance) / 10000.;
            weight = fabs(1 / (weight * weight * weight));
            sumWeights += weight;
            sum += double(myPointList[i].value) * weight;
        }
    }

    if (sumWeights > 0.0)
        return float(sum / sumWeights);
    else
        return NODATA;
}

/*
 * wind?
float gaussWeighted(vector <Crit3DInterpolationDataPoint> &myPointList)
{
    double sum, sumWeights, weight;
    double distance, deltaZ;
    double Rd=10;
    double Rz=1;

    sum = 0 ;
    sumWeights = 0 ;
    for (int i = 0 ; i < (int)(myPointList.size()); i++)
    {
        Crit3DInterpolationDataPoint myPoint = myPointList[i];
        distance = myPoint.distance / 1000.;
        deltaZ = myPoint.deltaZ / 1000.;
        if (myPoint.distance > 0.)
        {
            weight = 1 - exp(-(distance*distance)/(Rd*Rd)) * exp(-(deltaZ*deltaZ)/(Rz*Rz));
            weight = fabs(1 / (weight * weight * weight));
            sumWeights += weight;
            sum += myPoint.value * weight;
        }
    }

    if (sumWeights > 0.0)
        return float(sum / sumWeights);
    else
        return NODATA;
}
*/


// TODO elevation std dev?
void localSelection(vector <Crit3DInterpolationDataPoint> &inputPoints, vector <Crit3DInterpolationDataPoint> &selectedPoints,
                    float x, float y, Crit3DInterpolationSettings& mySettings, bool excludeSupplemental)
{
    // search more stations to assure min points with all valid proxies
    float ratioMinPoints = 1.2f;
    unsigned minPoints = unsigned(mySettings.getMinPointsLocalDetrending() * ratioMinPoints);

    if (inputPoints.size() <= minPoints)
    {
        selectedPoints = inputPoints;
        mySettings.setLocalRadius(computeShepardInitialRadius(mySettings.getPointsBoundingBoxArea(),
                                                              unsigned(inputPoints.size()), minPoints));
    }

    for (unsigned long i = 0; i < inputPoints.size() ; i++)
    {
        inputPoints[i].distance = gis::computeDistance(x, y, float((inputPoints[i]).point->utm.x), float((inputPoints[i]).point->utm.y));
    }

    unsigned int nrValid = 0;
    unsigned int nrPrimaries = 0;
    float maxDistance = 0;              // [m]
    float stepRadius = 7500;            // [m]
    float r0 = 0;                       // [m]
    float r1 = stepRadius;              // [m]
    bool beyondLastPoint = false;

    while (((! mySettings.getUseLapseRateCode() && nrValid < minPoints) || (mySettings.getUseLapseRateCode() && nrPrimaries < minPoints)) && !beyondLastPoint)
    {
        beyondLastPoint = true;
        for (unsigned int i=0; i < inputPoints.size(); i++)
        {
            if ((! isEqual(inputPoints[i].distance, NODATA) && inputPoints[i].distance > r0 && inputPoints[i].distance <= r1) && ! (mySettings.getUseLapseRateCode() && excludeSupplemental && inputPoints[i].lapseRateCode == supplemental))
            {
                selectedPoints.push_back(inputPoints[i]);
                nrValid++;
                if (inputPoints[i].distance > maxDistance)
                {
                    maxDistance = inputPoints[i].distance;
                }

                if (checkLapseRateCode(inputPoints[i].lapseRateCode, mySettings.getUseLapseRateCode(), true))
                {
                    nrPrimaries++;
                }
            }
            if (beyondLastPoint && inputPoints[i].distance > r1 && !(mySettings.getUseLapseRateCode() && excludeSupplemental && inputPoints[i].lapseRateCode == supplemental)) //check if there are still stations beyond current r1 value
                beyondLastPoint = false;
        }

        if (nrValid > unsigned(minPoints*0.8)) stepRadius = 1000;

        r0 = r1;
        r1 += stepRadius;
    }

    if (! isEqual(maxDistance, 0))
    {
        for (unsigned int i=0; i < selectedPoints.size(); i++)
        {
            selectedPoints[i].regressionWeight = MAXVALUE(1.f - selectedPoints[i].distance / maxDistance, float(EPSILON));

            //selectedPoints[i].regressionWeight = MAXVALUE(std::exp(-selectedPoints[i].distance*selectedPoints[i].distance/((0.5*maxDistance)*(0.5*maxDistance))),EPSILON);
            //selectedPoints[i].regressionWeight = float(MAXVALUE((-(1/std::pow(maxDistance,4)*(std::pow(selectedPoints[i].distance,4)))+1),EPSILON));
            //selectedPoints[i].regressionWeight = 1;
        }
    }
    mySettings.setLocalRadius(maxDistance);

    return;
}


bool checkPrecipitationZero(const std::vector<Crit3DInterpolationDataPoint> &myPoints, float precThreshold, int &nrNotNull)
{
    nrNotNull = 0;

    for (unsigned int i = 0; i < myPoints.size(); i++)
        if (myPoints[i].isActive)
            if (! isEqual(myPoints[i].value, NODATA))
                if (myPoints[i].value >= precThreshold)
                    nrNotNull++;

    return (nrNotNull == 0);
}


// predisposta per eventuale aggiunta wind al detrend
bool isThermal(meteoVariable myVar)
{
    if ( myVar == airTemperature ||
        myVar == airDewTemperature ||
        myVar == dailyAirTemperatureAvg ||
        myVar == dailyAirTemperatureMax ||
        myVar == dailyAirTemperatureMin ||
        myVar == dailyReferenceEvapotranspirationHS ||
        myVar == elaboration )
        return true;
    else
        return false;
}


bool getUseDetrendingVar(meteoVariable myVar)
{
    if ( myVar == airTemperature ||
        myVar == airDewTemperature ||
        myVar == dailyAirTemperatureAvg ||
        myVar == dailyAirTemperatureMax ||
        myVar == dailyAirTemperatureMin ||
        myVar == dailyReferenceEvapotranspirationHS ||
        myVar == elaboration )

        return true;
    else
        return false;
}

bool getUseTdVar(meteoVariable myVar)
{
    //exclude large scale variables
    if (myVar == precipitation ||
        myVar == dailyPrecipitation ||
        myVar == globalIrradiance ||
        myVar == atmTransmissivity ||
        myVar == dailyGlobalRadiation ||
        myVar == atmPressure ||
        myVar == dailyWaterTableDepth)

        return false;
    else
        return true;
}

void detrendPoints(std::vector <Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* mySettings,
                   meteoVariable myVar, unsigned pos)
{
    float detrendValue, proxyValue;
    unsigned myIndex;
    Crit3DInterpolationDataPoint* myPoint;
    Crit3DProxy* myProxy;

    if (! getUseDetrendingVar(myVar)) return;

    myProxy = mySettings->getProxy(pos);

    for (myIndex = 0; myIndex < myPoints.size(); myIndex++)
    {
        detrendValue = 0;
        myPoint = &(myPoints[myIndex]);

        proxyValue = myPoint->getProxyValue(pos);

        if (getProxyPragaName(myProxy->getName()) == proxyHeight)
        {
            if (proxyValue != NODATA)
            {
                float LR_above = myProxy->getRegressionSlope();
                if (myProxy->getInversionIsSignificative())
                {
                    float LR_H1 = myProxy->getLapseRateH1();
                    float LR_H0 = myProxy->getLapseRateH0();
                    float LR_below = myProxy->getInversionLapseRate();

                    if (proxyValue <= LR_H1)
                        detrendValue = MAXVALUE(proxyValue - LR_H0, 0) * LR_below;
                    else
                        detrendValue = ((LR_H1 - LR_H0) * LR_below) + (proxyValue - LR_H1) * LR_above;
                }
                else
                    detrendValue = MAXVALUE(proxyValue, 0) * LR_above;
            }
        }

        else
        {
            if (proxyValue != NODATA)
                if (myProxy->getRegressionR2() >= mySettings->getMinRegressionR2())
                    detrendValue = proxyValue * myProxy->getRegressionSlope();
        }

        myPoint->value -= detrendValue;
    }
}

float retrend(meteoVariable myVar, vector<double> myProxyValues, Crit3DInterpolationSettings* mySettings)
{
    if (! getUseDetrendingVar(myVar)) return 0.;

    double retrendValue = 0.;
    double myProxyValue;
    Crit3DProxy* myProxy = nullptr;
    float proxySlope;
    Crit3DProxyCombination myCombination = mySettings->getCurrentCombination();

    if (mySettings->getUseMultipleDetrending())
    {
        std::vector <double> activeProxyValues;

        //functions have been set in setFittingParameters_elevation and _otherProxies (height proxy first)
        std::vector<std::function<double(double, std::vector<double>&)>> myFunc = mySettings->getFittingFunction();
        //parameters have been set after bestFitting function in multipleDetrendingElevation
        // and multipleDetrending (height proxy first)
        std::vector <std::vector <double>> fittingParameters = mySettings->getFittingParameters();

        if (getMultipleDetrendingValues(*mySettings, myProxyValues, activeProxyValues, myFunc, fittingParameters))
        {
            if (myFunc.size() > 0 && fittingParameters.size() > 0)
                retrendValue = float(functionSum(myFunc, activeProxyValues, fittingParameters));
        }
    }
    else
    {
        for (int pos=0; pos < int(mySettings->getProxyNr()); pos++)
        {
            myProxy = mySettings->getProxy(pos);

            if (myCombination.isProxyActive(pos) && myCombination.isProxySignificant(pos))
            {
                myProxyValue = mySettings->getProxyValue(pos, myProxyValues);

                if (myProxyValue != NODATA)
                {
                    proxySlope = myProxy->getRegressionSlope();

                    if (getProxyPragaName(myProxy->getName()) == proxyHeight)
                    {
                        if (mySettings->getUseThermalInversion() && myProxy->getInversionIsSignificative())
                        {
                            float LR_H0 = myProxy->getLapseRateH0();
                            float LR_H1 = myProxy->getLapseRateH1();
                            float LR_Below = myProxy->getInversionLapseRate();
                            if (myProxyValue <= LR_H1)
                                retrendValue += (MAXVALUE(myProxyValue - LR_H0, 0) * LR_Below);
                            else
                                retrendValue += ((LR_H1 - LR_H0) * LR_Below) + (myProxyValue - LR_H1) * proxySlope;
                        }
                        else
                            retrendValue += MAXVALUE(myProxyValue, 0) * proxySlope;
                    }
                    else
                        retrendValue += myProxyValue * proxySlope;
                }
            }
        }
    }

    return float(retrendValue);
}


bool regressionOrography(std::vector <Crit3DInterpolationDataPoint> &myPoints,
                         Crit3DProxyCombination myCombination, Crit3DInterpolationSettings* mySettings, Crit3DClimateParameters* myClimate,
                         Crit3DTime myTime, meteoVariable myVar, int orogProxyPos)
{
    if (getUseDetrendingVar(myVar))
    {
        if (isThermal(myVar))
        {
            if (myCombination.getUseThermalInversion())
                return regressionOrographyT(myPoints, mySettings, myClimate, myTime, myVar, orogProxyPos, true);
            else
                return regressionSimpleT(myPoints, mySettings, myClimate, myTime, myVar, orogProxyPos);
        }
        else
        {
            return regressionGeneric(myPoints, mySettings, orogProxyPos, false);
        }
    }
    else
    {
        return false;
    }
}


void detrending(std::vector <Crit3DInterpolationDataPoint> &myPoints,
                Crit3DProxyCombination inCombination,
                Crit3DInterpolationSettings* mySettings, Crit3DClimateParameters* myClimate,
                meteoVariable myVar, Crit3DTime myTime)
{
    if (! getUseDetrendingVar(myVar)) return;

    Crit3DProxy* myProxy;

    mySettings->setCurrentCombination(inCombination);

    for (int pos=0; pos < int(mySettings->getProxyNr()); pos++)
    {
        if (inCombination.isProxyActive(pos))
        {
            myProxy = mySettings->getProxy(pos);
            mySettings->setSignificantCurrentCombination(pos, false);

            if (getProxyPragaName(myProxy->getName()) == proxyHeight)
            {
                if (regressionOrography(myPoints, inCombination, mySettings, myClimate, myTime, myVar, pos))
                {
                    mySettings->setSignificantCurrentCombination(pos, true);
                    detrendPoints(myPoints, mySettings, myVar, pos);
                }
            }
            else
            {
                if (regressionGeneric(myPoints, mySettings, pos, false))
                {
                    mySettings->setSignificantCurrentCombination(pos, true);
                    detrendPoints(myPoints, mySettings, myVar, pos);
                }
            }
        }
    }
}


bool proxyValidity(std::vector <Crit3DInterpolationDataPoint> &myPoints, int proxyPos, float stdDevThreshold, double &avg, double &stdDev)
{
    std::vector<float> proxyValues;
    const int MIN_NR = 10;

    avg = NODATA;
    stdDev = NODATA;

    double sum = 0;
    for (unsigned i = 0; i < myPoints.size(); i++)
    {
        if (myPoints[i].isActive)
        {
            float myValue = myPoints[i].getProxyValue(proxyPos);
            if (myValue != NODATA)
            {
                proxyValues.push_back(myValue);
                sum += myValue;
            }
        }
    }

    if (proxyValues.size() < MIN_NR) return false;

    avg = sum / proxyValues.size();

    sum = 0;
    for (unsigned i = 0; i < proxyValues.size(); i++)
    {
        sum += (proxyValues[i] - avg) * (proxyValues[i] - avg);
    }

    stdDev = sqrt(sum / (proxyValues.size() - 1));

    if (stdDevThreshold != NODATA)
        return (stdDev > stdDevThreshold);
    else
        return true;
}

bool proxyValidityWeighted(std::vector <Crit3DInterpolationDataPoint> &myPoints, int proxyPos, float stdDevThreshold)
{
    double stdDev;
    std::vector<float> proxyValues;

    std::vector<double> data, weights;
    data.resize(myPoints.size());
    weights.resize(myPoints.size());

    for (unsigned i = 0; i < myPoints.size(); i++)
    {
        data[i] = myPoints[i].getProxyValue(proxyPos);
        weights[i] = myPoints[i].regressionWeight;
    }


    if (data.size() <= 0) {
        // Handle the case when there is no data or weights
        return 0.0;
    }

    double sum_weights = 0.0;
    double sum_weighted_data = 0.0;
    double sum_squared_weighted_data = 0.0;

    // Calculate the necessary sums for weighted variance calculation
    for (int i = 0; i < int(data.size()); i++)
    {
        sum_weights += weights[i];
        sum_weighted_data += data[i] * weights[i];
        sum_squared_weighted_data += data[i] * data[i] * weights[i];
    }

    // Calculate the weighted variance
    double weighted_mean = sum_weighted_data / sum_weights;
    double variance = (sum_squared_weighted_data / sum_weights) - (weighted_mean * weighted_mean);

    stdDev = sqrt(variance);

    if (stdDevThreshold != NODATA)
        return (stdDev > stdDevThreshold);
    else
        return true;
}

bool setMultipleDetrendingHeightTemperatureRange(Crit3DInterpolationSettings* mySettings)
{
    if (mySettings->getPointsRange().empty() || !mySettings->getUseMultipleDetrending())
        return 0;

    Crit3DProxyCombination myCombination = mySettings->getSelectedCombination();

    for (unsigned i=0; i < myCombination.getProxySize(); i++)
        if (myCombination.isProxyActive(i) == true)
        {
            if (getProxyPragaName(mySettings->getProxy(i)->getName()) == proxyHeight)
            {
                const double MIN_T = mySettings->getPointsRange()[0];
                const double MAX_T = mySettings->getPointsRange()[1];

                std::vector<double> tempParam;
                tempParam = mySettings->getProxy(i)->getFittingParametersRange();
                if (!tempParam.empty())
                {
                    if (mySettings->getChosenElevationFunction() == piecewiseTwo)
                    {
                        tempParam[1] = MIN_T-2;
                        tempParam[5] = MAX_T+6;
                        mySettings->addFittingFunction(lapseRatePiecewise_two);
                    }
                    else if (mySettings->getChosenElevationFunction() == piecewiseThreeFree)
                    {
                        tempParam[1] = MIN_T-2;
                        tempParam[7] = MAX_T+6;
                        mySettings->addFittingFunction(lapseRatePiecewise_three_free);
                    }
                    else if (mySettings->getChosenElevationFunction() == piecewiseThree)
                    {
                        tempParam[1] = MIN_T-2;
                        tempParam[6] = MAX_T+6;
                        mySettings->addFittingFunction(lapseRatePiecewise_three);
                    }
                    mySettings->getProxy(i)->setFittingParametersRange(tempParam);
                }

                calculateFirstGuessCombinations(mySettings->getProxy(i));
            }
        }
    return true;
}

void calculateFirstGuessCombinations(Crit3DProxy* myProxy)
{
    std::vector<double> tempParam = myProxy->getFittingParametersRange();
    std::vector <int> firstGuessPosition = myProxy->getFittingFirstGuess();
    std::vector <double> tempFirstGuess;
    int numSteps = 15;
    std::vector <double> stepSize;
    unsigned nrParam = int(tempParam.size()/2);

    double min_,max_;
    for (unsigned j=0; j < nrParam; j++)
    {
        min_ = tempParam[j];
        max_ = tempParam[nrParam+j];
        stepSize.push_back((max_ - min_)/numSteps);
        if (firstGuessPosition[j] == 0)
            tempFirstGuess.push_back(min_);
        else if (firstGuessPosition[j] == 1)
            tempFirstGuess.push_back((min_ + max_) / 2);
        else
            tempFirstGuess.push_back(max_);
    }

    std::vector <std::vector<double>> firstGuessCombinations;
    std::vector <double> firstGuessParam = tempFirstGuess;
    firstGuessCombinations.push_back(tempFirstGuess);
    std::vector<int> paramIndex;

    if (myProxy->getFittingFunctionName() == piecewiseTwo)
        paramIndex = {0};
    else
        paramIndex = {0,2};

    for (int k = 0; k < int(paramIndex.size()); k++)
    {
        for (int m = 1; m < numSteps+1; m++)
        {
            if (firstGuessPosition[k] == 0)
            {
                tempFirstGuess[paramIndex[k]] = firstGuessParam[paramIndex[k]] + m * stepSize[paramIndex[k]];
                firstGuessCombinations.push_back(tempFirstGuess);
                tempFirstGuess = firstGuessParam;
            }
            else if (firstGuessPosition[k] == 2)
            {
                tempFirstGuess[paramIndex[k]] = firstGuessParam[paramIndex[k]] - m * stepSize[paramIndex[k]];
                firstGuessCombinations.push_back(tempFirstGuess);
                tempFirstGuess = firstGuessParam;
            }
            else
            {
                if (m < int((numSteps)/2)+1)
                {
                    tempFirstGuess[paramIndex[k]] = MINVALUE(firstGuessParam[paramIndex[k]] + m * stepSize[paramIndex[k]], tempParam[nrParam+paramIndex[k]]);
                    firstGuessCombinations.push_back(tempFirstGuess);
                    tempFirstGuess = firstGuessParam;

                    tempFirstGuess[paramIndex[k]] = MAXVALUE(firstGuessParam[paramIndex[k]] - m * stepSize[paramIndex[k]], tempParam[paramIndex[k]]);
                    firstGuessCombinations.push_back(tempFirstGuess);
                    tempFirstGuess = firstGuessParam;
                }
            }
        }
    }
    myProxy->setFirstGuessCombinations(firstGuessCombinations);

    return;
}

bool setFittingParameters_elevation(int elevationPos, Crit3DInterpolationSettings* mySettings,
                             std::function<double(double, std::vector<double>&)>& myFunc,
                             std::vector<double> &paramMin, std::vector<double> &paramMax,
                             std::vector<double> &paramDelta, std::vector<double> &paramFirstGuess,
                             std::vector<double> &stepSize, int numSteps,
                             std::string &errorStr)
{
    const double RATIO_DELTA = 800;

    if (mySettings->getChosenElevationFunction() == piecewiseTwo)
    {
        mySettings->addFittingFunction(lapseRatePiecewise_two);
        myFunc = lapseRatePiecewise_two;
    }
    else if (mySettings->getChosenElevationFunction() == piecewiseThreeFree)
    {
        mySettings->addFittingFunction(lapseRatePiecewise_three_free);
        myFunc = lapseRatePiecewise_three_free;
    }
    else if (mySettings->getChosenElevationFunction() == piecewiseThree)
    {
        mySettings->addFittingFunction(lapseRatePiecewise_three);
        myFunc = lapseRatePiecewise_three;
    }
    else
    {
        errorStr = "Missing or wrong fitting function for proxy: height";
        return false;
    }

    std::vector <double> myParam = mySettings->getProxy(elevationPos)->getFittingParametersRange();
    unsigned int nrParam = unsigned(myParam.size() / 2);

    if (nrParam == 0)
    {
        errorStr = "Missing fitting parameters for proxy: height";
        return false;
    }

    double min_,max_;
    for (unsigned j=0; j < nrParam; j++)
    {
        min_ = myParam[j];
        max_ = myParam[nrParam+j];
        paramMin.push_back(min_);
        paramMax.push_back(max_);
        paramDelta.push_back((max_ - min_) / RATIO_DELTA);
        paramFirstGuess.push_back((max_ + min_) / 2);
        stepSize.push_back((max_ - min_)/numSteps);
    }

    return true;
}

bool setFittingParameters_otherProxies(int elevationPos, Crit3DInterpolationSettings* mySettings,
                                     std::vector<std::function<double(double, std::vector<double>&)>>& myFunc,
                                     std::vector <std::vector<double>> &paramMin, std::vector <std::vector<double>> &paramMax,
                                     std::vector <std::vector<double>> &paramDelta, std::vector <std::vector<double>> &paramFirstGuess,
                                     std::string &errorStr)
{
    const double RATIO_DELTA = 1000;

    Crit3DProxyCombination myCombination = mySettings->getCurrentCombination();

    for (unsigned i=0; i < myCombination.getProxySize(); i++)
    {
        if (i != unsigned(elevationPos) && myCombination.isProxyActive(i) && myCombination.isProxySignificant(i))
        {
            mySettings->addFittingFunction(functionLinear_intercept);
            myFunc.push_back(functionLinear_intercept);

            std::vector <double> myParam = mySettings->getProxy(i)->getFittingParametersRange();
            unsigned int nrParam = unsigned(myParam.size() / 2);

            if (nrParam == 0)
            {
                errorStr = "Missing fitting parameters for proxy: " + mySettings->getProxy(i)->getName();
                return false;
            }

            double min_,max_;
            std::vector <double> proxyParamMin;
            std::vector <double> proxyParamMax;
            std::vector <double> proxyParamDelta;
            std::vector <double> proxyParamFirstGuess;

            for (unsigned j=0; j < nrParam; j++)
            {
                min_ = myParam[j];
                max_ = myParam[nrParam+j];
                proxyParamMin.push_back(min_);
                proxyParamMax.push_back(max_);
                proxyParamDelta.push_back((max_ - min_) / RATIO_DELTA);
                proxyParamFirstGuess.push_back((max_ + min_) / 2);
            }
            paramMin.push_back(proxyParamMin);
            paramMax.push_back(proxyParamMax);
            paramDelta.push_back(proxyParamDelta);
            paramFirstGuess.push_back(proxyParamFirstGuess);
        }
    }
    return myFunc.size() > 0;
}


std::vector <double> getfittingParameters(Crit3DProxyCombination myCombination, Crit3DInterpolationSettings* mySettings, std::vector <double> paramOut, unsigned pos)
{
    std::vector <double> myParam;
    unsigned i,j,index;

    index=0;
    for (i=0; i < myCombination.getProxySize(); i++)
        if (myCombination.isProxyActive(i))
        {
            if (getProxyPragaName(mySettings->getProxy(i)->getName()) == proxyHeight)
            {
                if (i == pos)
                    for (j=0; j<5; j++)
                        myParam.push_back(paramOut[index+j]);

                index+=5;
            }
            else
            {
                if (i == pos) myParam.push_back(paramOut[index]);
                index++;
            }
        }

    return myParam;
}


void macroAreaDetrending(Crit3DMacroArea myArea, meteoVariable myVar, Crit3DInterpolationSettings &mySettings, Crit3DMeteoSettings* meteoSettings, Crit3DMeteoPoint* meteoPoints, std::vector <Crit3DInterpolationDataPoint> interpolationPoints, std::vector <Crit3DInterpolationDataPoint> &subsetInterpolationPoints, int elevationPos)
{
    //take the parameters+combination for that area
    mySettings.setFittingParameters(myArea.getParameters());
    mySettings.setCurrentCombination(myArea.getCombination());

    //find the fitting functions vector based on the length of the parameters vector for every proxy
    std::vector<std::function<double (double, std::vector<double> &)> > fittingFunction;

    for (int l = 0; l < (int)myArea.getParameters().size(); l++)
    {
        if (myArea.getParameters()[l].size() == 2)
            fittingFunction.push_back(functionLinear_intercept);
        else if (myArea.getParameters()[l].size() == 4)
            fittingFunction.push_back(lapseRatePiecewise_two);
        else if (myArea.getParameters()[l].size() == 5)
            fittingFunction.push_back(lapseRatePiecewise_three);
        else if (myArea.getParameters()[l].size() == 6)
            fittingFunction.push_back(lapseRatePiecewise_three_free);
    }

    mySettings.setFittingFunction(fittingFunction);

    // create vector of macro area interpolation points
    std::vector<int> temp = myArea.getMeteoPoints();

    for (int l = 0; l < (int)temp.size(); l++)
    {
        for (int k = 0; k < (int)interpolationPoints.size(); k++)
        {
            if (interpolationPoints[k].index == temp[l])
            {
                subsetInterpolationPoints.push_back(interpolationPoints[k]);
            }
        }
    }

    //detrending
    if (elevationPos != NODATA && myArea.getCombination().isProxyActive(elevationPos) && myArea.getCombination().isProxySignificant(elevationPos))
    {
        detrendingElevation(elevationPos, subsetInterpolationPoints, &mySettings);
    }

    detrendingOtherProxies(elevationPos, subsetInterpolationPoints, &mySettings);

    Crit3DMeteoPoint* myMeteoPoints = new Crit3DMeteoPoint[unsigned(myArea.getMeteoPointsNr())];
    std::vector<int> meteoPointsList = myArea.getMeteoPoints();

    for (unsigned i = 0; i < meteoPointsList.size(); i++)
    {
        myMeteoPoints[i] = meteoPoints[meteoPointsList[i]];
    }

    if (mySettings.getUseTD() && getUseTdVar(myVar))
    {
        topographicDistanceOptimize(myVar, myMeteoPoints, myArea.getMeteoPointsNr(),
                                    subsetInterpolationPoints, &mySettings, meteoSettings);
    }

    myMeteoPoints->clear();
    delete[] myMeteoPoints;

    return;
}

bool multipleDetrendingMain(std::vector <Crit3DInterpolationDataPoint> &myPoints,
                            Crit3DInterpolationSettings* mySettings, meteoVariable myVar, std::string &errorStr)
{
    mySettings->setCurrentCombination(mySettings->getSelectedCombination());
    mySettings->clearFitting();

    int elevationPos = NODATA;
    for (unsigned int pos=0; pos < mySettings->getCurrentCombination().getProxySize(); pos++)
    {
        if (getProxyPragaName(mySettings->getProxy(pos)->getName()) == proxyHeight)
            elevationPos = pos;
    }

    if (elevationPos != NODATA && mySettings->getCurrentCombination().isProxyActive(elevationPos))
    {
        if (!multipleDetrendingElevationFitting(elevationPos, myPoints, mySettings, myVar, errorStr, true))
            return false;

        if (mySettings->getCurrentCombination().isProxySignificant(elevationPos)) detrendingElevation(elevationPos, myPoints, mySettings);
    }

    if (!multipleDetrendingOtherProxiesFitting(elevationPos, myPoints, mySettings, myVar, errorStr))
        return false;

    detrendingOtherProxies(elevationPos, myPoints, mySettings);

    return true;

}

bool multipleDetrendingElevationFitting(int elevationPos, std::vector <Crit3DInterpolationDataPoint> &myPoints,
                                 Crit3DInterpolationSettings* mySettings, meteoVariable myVar, std::string &errorStr, bool isWeighted)
{
    mySettings->getProxy(elevationPos)->setRegressionR2(NODATA);
    if (! getUseDetrendingVar(myVar)) return true;
    if (elevationPos == NODATA) return true;

    // find points with valid elevation and role
    std::vector <Crit3DInterpolationDataPoint> elevationPoints = myPoints;
    vector<Crit3DInterpolationDataPoint>::iterator it = elevationPoints.begin();
    while (it != elevationPoints.end())
    {
        if (! checkLapseRateCode(it->lapseRateCode, mySettings->getUseLapseRateCode(), true)  || it->getProxyValue(elevationPos) == NODATA)
            it = elevationPoints.erase(it);
        else
            it++;
    }

    // proxy spatial variability and minimum points number for local detrending
    bool isValid = false;

    isValid = proxyValidityWeighted(elevationPoints, elevationPos, mySettings->getProxy(elevationPos)->getStdDevThreshold());
    if (mySettings->getUseLocalDetrending()) isValid = (isValid && elevationPoints.size() >= unsigned(mySettings->getMinPointsLocalDetrending()));

    mySettings->setSignificantCurrentCombination(elevationPos, isValid);

    if (! isValid) return true;

    // filling vectors
    std::vector <double> predictors;
    std::vector <double> predictands;
    std::vector <double> weights;

    unsigned i;
    for (i=0; i < elevationPoints.size(); i++)
    {
        predictors.push_back(elevationPoints[i].getProxyValue(elevationPos));
        predictands.push_back(elevationPoints[i].value);
        if (elevationPoints[i].regressionWeight != NODATA)
            weights.push_back(elevationPoints[i].regressionWeight);
        else weights.push_back(1);
    }

    std::vector<double> parametersMin;
    std::vector<double> parametersMax;
    std::vector<double> parametersDelta;
    std::vector<double> parameters;
    std::vector<double> stepSize;
    int numSteps = 15;
    std::function<double(double, std::vector<double>&)> myFunc;


    if (! setFittingParameters_elevation(elevationPos, mySettings, myFunc, parametersMin, parametersMax,
                                        parametersDelta, parameters, stepSize, numSteps, errorStr))
    {
        errorStr = "couldn't prepare the fitting parameters for proxy: elevation.";
        return false;
    }

    std::vector<std::vector<double>> firstGuessCombinations = mySettings->getProxy(elevationPos)->getFirstGuessCombinations();
    // multiple non linear fitting
    double R2 = NODATA;
    if (isWeighted)
        R2 = interpolation::bestFittingMarquardt_nDimension(*(myFunc.target<double(*)(double, std::vector<double>&)>()), 4, parametersMin, parametersMax, parameters, parametersDelta,
                                                                  1000, 0.002, 0.005, predictors, predictands, weights,firstGuessCombinations);
    else
        R2 = interpolation::bestFittingMarquardt_nDimension(*(myFunc.target<double(*)(double, std::vector<double>&)>()), 4, parametersMin, parametersMax, parameters, parametersDelta,
                                                                  1000, 0.002, 0.005, predictors, predictands,firstGuessCombinations);

    mySettings->getProxy(elevationPos)->setRegressionR2(float(R2));

    std::vector<std::vector<double>> newParameters;
    newParameters.push_back(parameters);
    mySettings->addFittingParameters(newParameters);

    return true;
}

void detrendingElevation(int elevationPos, std::vector <Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* mySettings)
{
    // detrending
    if (mySettings->getFittingFunction().empty() || mySettings->getFittingParameters().empty())
        return;

    std::function<double(double, std::vector<double>&)> myFunc = mySettings->getFittingFunction().front();
    std::vector <double> parameters = mySettings->getFittingParameters().front();

    float proxyValue, detrendValue;
    for (unsigned int i = 0; i < myPoints.size(); i++)
    {
        proxyValue = myPoints[i].getProxyValue(elevationPos);

        detrendValue = float((myFunc)(proxyValue, parameters));
        myPoints[i].value -= detrendValue;
    }

    return;
}

bool multipleDetrendingOtherProxiesFitting(int elevationPos, std::vector <Crit3DInterpolationDataPoint> &myPoints,
                        Crit3DInterpolationSettings* mySettings, meteoVariable myVar, std::string &errorStr)
{
    if (! getUseDetrendingVar(myVar)) return true;

    Crit3DProxyCombination myCombination = mySettings->getCurrentCombination();

    // verify predictors number
    unsigned nrPredictors = 0;
    std::vector <unsigned int> proxyIndex;
    int proxyNr = int(mySettings->getProxyNr());
    for (int pos=0; pos < proxyNr; pos++)
    {
        if (pos != elevationPos && myCombination.isProxyActive(pos))
        {
            mySettings->setSignificantCurrentCombination(pos, false);
            proxyIndex.push_back(pos);
            nrPredictors++;
        }
    }

    if (nrPredictors == 0) return true;

    //lapse rate code
    std::vector <Crit3DInterpolationDataPoint> othersPoints = myPoints;
    vector<Crit3DInterpolationDataPoint>::iterator it = othersPoints.begin();
    while (it != othersPoints.end())
    {
        if (!checkLapseRateCode(it->lapseRateCode, mySettings->getUseLapseRateCode(), false))
            it = othersPoints.erase(it);
        else
            it++;
    }

    // proxy spatial variability (1st step)
    // this is done before check of incomplete to keep as many points as possible
    unsigned validNr;
    validNr = 0;

    for (int pos=0; pos < proxyNr; pos++)
    {
        if (pos != elevationPos && myCombination.isProxyActive(pos))
        {
            if (proxyValidityWeighted(othersPoints, pos, mySettings->getProxy(pos)->getStdDevThreshold()))
            {
                mySettings->setSignificantCurrentCombination(pos, true);
                validNr++;
            }
            else
            {
                mySettings->setSignificantCurrentCombination(pos, false);
            }
        }
    }

    if (validNr == 0) return true;

    // exclude points with incomplete proxies
    unsigned i;
    bool isValid;
    float proxyValue;
    it = myPoints.begin();

    while (it != myPoints.end())
    {
        isValid = true;
        for (int pos=0; pos < proxyNr; pos++)
            if (pos != elevationPos && myCombination.isProxyActive(pos) && mySettings->getCurrentCombination().isProxySignificant(pos))
            {
                proxyValue = it->getProxyValue(pos);
                if (proxyValue == NODATA)
                {
                    isValid = false;
                    break;
                }
            }

        if (! isValid)
        {
            it = myPoints.erase(it);
        }
        else {
            it++;
        }
    }

    it = othersPoints.begin();
    while (it != othersPoints.end())
    {
        isValid = true;
        for (int pos=0; pos < proxyNr; pos++)
            if (pos != elevationPos && myCombination.isProxyActive(pos) && mySettings->getCurrentCombination().isProxySignificant(pos))
            {
                proxyValue = it->getProxyValue(pos);
                if (proxyValue == NODATA)
                {
                    isValid = false;
                    break;
                }
            }

        if (! isValid)
        {
            it = othersPoints.erase(it);
        }
        else {
            it++;
        }
    }

    // proxy spatial variability (2nd step)
    // to be done because we might have excluded some points
    validNr = 0;
    for (int pos=0; pos < proxyNr; pos++)
    {
        if (pos != elevationPos && myCombination.isProxyActive(pos))
        {
            if (proxyValidityWeighted(othersPoints, pos, mySettings->getProxy(pos)->getStdDevThreshold()))
            {
                mySettings->setSignificantCurrentCombination(pos, true);
                validNr++;
            }
            else
            {
                mySettings->setSignificantCurrentCombination(pos, false);
            }
        }
    }

    if (validNr == 0) return true;

    // filling vectors
    std::vector <double> rowPredictors;
    std::vector <std::vector <double>> predictors;
    std::vector <double> predictands;
    std::vector <double> weights;

    for (i=0; i < othersPoints.size(); i++)
    {
        rowPredictors.clear();
        for (int pos=0; pos < proxyNr; pos++)
            if (pos != elevationPos && myCombination.isProxyActive(pos) && mySettings->getCurrentCombination().isProxySignificant(pos))
            {
                proxyValue = othersPoints[i].getProxyValue(pos);
                rowPredictors.push_back(proxyValue);
            }

        predictors.push_back(rowPredictors);
        predictands.push_back(othersPoints[i].value);
        if (!isEqual(othersPoints[i].regressionWeight, NODATA))
            weights.push_back(othersPoints[i].regressionWeight);
        else
            weights.push_back(1);
    }

    if (mySettings->getUseLocalDetrending() && int(othersPoints.size()) < mySettings->getMinPointsLocalDetrending())
    {
        for (int pos = 0; pos < proxyNr; pos++)
        {
            if (pos != elevationPos)
            {
                mySettings->setSignificantCurrentCombination(pos, false);
            }
        }
        return true;
    }

    std::vector <std::vector<double>> parametersMin;
    std::vector <std::vector<double>> parametersMax;
    std::vector <std::vector<double>> parametersDelta;
    std::vector <std::vector<double>> parameters;
    std::vector<std::function<double(double, std::vector<double>&)>> myFunc;

    if (! setFittingParameters_otherProxies(elevationPos, mySettings, myFunc, parametersMin, parametersMax,
                                 parametersDelta, parameters, errorStr))
        return false;


    // multilinear fitting
    interpolation::bestFittingMarquardt_nDimension(&functionSum, myFunc, parametersMin, parametersMax, parameters, parametersDelta,
                                                   100, 0.005, predictors, predictands, weights);


    mySettings->addFittingParameters(parameters);

    return true;
}

void detrendingOtherProxies(int elevationPos, std::vector<Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* mySettings)
{
    std::vector <double> proxyValues;
    double proxyValue;

    std::vector<std::function<double(double, std::vector<double>&)>> myFunc = mySettings->getFittingFunction();
    std::vector<std::vector<double>> parameters = mySettings->getFittingParameters();

    if (parameters.empty()) return;

    //if height was significative and height function/parameters are present, delete them
    if (parameters.front().size() > 2)
    {
        parameters.erase(parameters.begin());
        myFunc.erase(myFunc.begin());
    }

    //if no other proxies are active/significant, return
    if (parameters.empty()) return;

    std::vector<std::vector<double>> fullParameters = parameters;
    std::vector<std::function<double(double, std::vector<double>&)>> fullFunc = myFunc;

    // detrending
    float detrendValue;
    for (unsigned int i = 0; i < myPoints.size(); i++)
    {
        proxyValues.clear();

        for (int pos=0; pos < int(mySettings->getProxyNr()); pos++)
        {
            if (pos != elevationPos && mySettings->getCurrentCombination().isProxyActive(pos) && mySettings->getCurrentCombination().isProxySignificant(pos))
            {
                proxyValue = myPoints[i].getProxyValue(pos);
                if (! isEqual(proxyValue, NODATA))
                    proxyValues.push_back(double(proxyValue));
                else
                {
                    parameters.erase(parameters.begin()+proxyValues.size());
                    myFunc.erase(myFunc.begin()+proxyValues.size());
                }
            }
        }

        detrendValue = float(functionSum(myFunc, proxyValues, parameters));
        myPoints[i].value -= detrendValue;

        myFunc = fullFunc;
        parameters = fullParameters;
    }
}


bool glocalDetrendingFitting(std::vector <Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* mySettings,  meteoVariable myVar, std::string& errorStr)
{
    std::vector<Crit3DInterpolationDataPoint> subsetPoints;
    std::vector<Crit3DMacroArea> macroAreas = mySettings->getMacroAreas();
    int areasNr = int(macroAreas.size());
    int i = 0;

    int elevationPos = NODATA;
    for (unsigned int pos=0; pos < mySettings->getSelectedCombination().getProxySize(); pos++)
    {
        if (getProxyPragaName(mySettings->getProxy(pos)->getName()) == proxyHeight)
            elevationPos = pos;
    }

    //create the subset of points starting from the meteopoints vector saved in every macro area
    for (i = 0; i < areasNr; i++) //ATTENZIONE INDICI ?
    {
        std::vector<int> temp = macroAreas[i].getMeteoPoints();
        if (! temp.empty())
        {
            std::vector<Crit3DInterpolationDataPoint> subsetPoints;
            for (unsigned int l = 0; l < temp.size(); l++)
            {

                for (unsigned int k = 0; k < myPoints.size(); k++)
                    if (myPoints[k].index == temp[l])
                    {
                        subsetPoints.push_back(myPoints[k]);
                        break;
                    }
            }

            mySettings->setCurrentCombination(mySettings->getSelectedCombination());
            mySettings->clearFitting();

            //fitting and elevation detrending (necessary for the fitting of other proxies)
            if (elevationPos != NODATA && mySettings->getSelectedCombination().isProxyActive(elevationPos))
            {
                if (!multipleDetrendingElevationFitting(elevationPos, subsetPoints, mySettings, myVar, errorStr, false)) return false;
                if (mySettings->getCurrentCombination().isProxySignificant(elevationPos)) detrendingElevation(elevationPos, subsetPoints, mySettings);
            }
            if (!multipleDetrendingOtherProxiesFitting(elevationPos, subsetPoints, mySettings, myVar, errorStr))
                return false;

            //save parameters and combination in the macro area
            macroAreas[i].setParameters(mySettings->getFittingParameters());
            macroAreas[i].setCombination(mySettings->getCurrentCombination());
        }
        subsetPoints.clear();
    }
    //update macro areas in settings
    mySettings->setMacroAreas(macroAreas);
    return true;
}


double goldenSectionSearch(meteoVariable myVar,Crit3DMeteoPoint* &myMeteoPoints,int nrMeteoPoints,
                           std::vector <Crit3DInterpolationDataPoint> &interpolationPoints,Crit3DInterpolationSettings* mySettings,
                           Crit3DMeteoSettings* meteoSettings, double a, double b)
{
    // this function finds the minimum by means of golden section method
    double tol = 1;
    //const double phi = (1 + std::sqrt(5)) / 2;  // golden section
    double x1 = b - (b - a) / GOLDEN_SECTION;
    double x2 = a + (b - a) / GOLDEN_SECTION;
    int counter=0;
    while (std::abs(b - a) > tol && counter<100)
    {
        counter++;
        if (topographicDistanceInternalFunction(myVar,myMeteoPoints,nrMeteoPoints,interpolationPoints, mySettings,meteoSettings, x1) <
            topographicDistanceInternalFunction(myVar,myMeteoPoints,nrMeteoPoints,interpolationPoints, mySettings,meteoSettings,x2))
        {
            b = x2;
            x2 = x1;
            x1 = b - (b - a) / GOLDEN_SECTION;
            mySettings->addToKhSeries(float(x1), (float)topographicDistanceInternalFunction(myVar,myMeteoPoints,nrMeteoPoints,interpolationPoints, mySettings,meteoSettings, x1));
        }
        else
        {
            a = x1;
            x1 = x2;
            x2 = a + (b - a) / GOLDEN_SECTION;
            mySettings->addToKhSeries(float(x2), (float)topographicDistanceInternalFunction(myVar,myMeteoPoints,nrMeteoPoints,interpolationPoints, mySettings,meteoSettings, x2));
        }
    }
    mySettings->addToKhSeries(float((a + b) / 2), (float)topographicDistanceInternalFunction(myVar,myMeteoPoints,nrMeteoPoints,interpolationPoints, mySettings,meteoSettings, (a + b) / 2));
    return (a + b) / 2;  // approximated minimum
}


double topographicDistanceInternalFunction(meteoVariable myVar,
                                 Crit3DMeteoPoint* &myMeteoPoints,
                                 int nrMeteoPoints,
                                 std::vector <Crit3DInterpolationDataPoint> &interpolationPoints,
                                 Crit3DInterpolationSettings* mySettings,
                                 Crit3DMeteoSettings* meteoSettings, double khFloat)
{
    float avgError = 0;
    int kh = int(khFloat);
    mySettings->setTopoDist_Kh(kh);
    if (computeResiduals(myVar, myMeteoPoints, nrMeteoPoints, interpolationPoints, mySettings, meteoSettings, true, true))
    {
        avgError = computeErrorCrossValidation(myMeteoPoints, nrMeteoPoints);
    }
    return avgError;
}


void topographicDistanceOptimize(meteoVariable myVar,
                                 Crit3DMeteoPoint* &myMeteoPoints,
                                 int nrMeteoPoints,
                                 std::vector <Crit3DInterpolationDataPoint> &interpolationPoints,
                                 Crit3DInterpolationSettings* mySettings,
                                 Crit3DMeteoSettings* meteoSettings)
{
    //float avgError;

    mySettings->initializeKhSeries();

    //float kh = 0;
    double bestKh = 0;
    //float bestError = NODATA;
    //std::vector<float> avgErrorVec;
    double khMin = 0;
    double khMax = double(mySettings->getTopoDist_maxKh());
    //kh = 0.5*(khMin+khMax);
    bestKh = goldenSectionSearch(myVar, myMeteoPoints, nrMeteoPoints, interpolationPoints, mySettings, meteoSettings, khMin, khMax);
    mySettings->setTopoDist_Kh(int(bestKh));
    /*
    while (kh <= mySettings->getTopoDist_maxKh())
    {
        mySettings->setTopoDist_Kh(kh);
        if (computeResiduals(myVar, myMeteoPoints, nrMeteoPoints, interpolationPoints, mySettings, meteoSettings, true, true))
        {
            avgError = computeErrorCrossValidation(myMeteoPoints, nrMeteoPoints);
            avgErrorVec.push_back(avgError);

            if (isEqual(bestError, NODATA) || avgError < bestError)
            {
                bestError = avgError;
                bestKh = kh;
            }

            mySettings->addToKhSeries(float(kh), avgError);
        }
        kh = ((kh == 0) ? 1 : kh*2);
    }

    mySettings->setTopoDist_Kh(bestKh);
    */
}


void optimalDetrending(meteoVariable myVar, Crit3DMeteoPoint* &myMeteoPoints, int nrMeteoPoints,
                       std::vector <Crit3DInterpolationDataPoint> &outInterpolationPoints,
                       Crit3DInterpolationSettings* mySettings, Crit3DMeteoSettings* meteoSettings, Crit3DClimateParameters* myClimate,
                       const Crit3DTime &myTime)
{

    std::vector <Crit3DInterpolationDataPoint> interpolationPoints;
    int i, nrCombination, bestCombinationIndex;
    float avgError, minError;
    size_t proxyNr = mySettings->getProxyNr();
    Crit3DProxyCombination myCombination, bestCombination;

    nrCombination = int(pow(2, double(proxyNr) + 1));

    minError = NODATA;
    bestCombinationIndex = 0;

    for (i=0; i < nrCombination; i++)
    {
        if (mySettings->getCombination(i, myCombination))
        {
            passDataToInterpolation(myMeteoPoints, nrMeteoPoints, interpolationPoints, mySettings);
            detrending(interpolationPoints, myCombination, mySettings, myClimate, myVar, myTime);

            if (mySettings->getUseTD() && getUseTdVar(myVar))
                topographicDistanceOptimize(myVar, myMeteoPoints, nrMeteoPoints, interpolationPoints, mySettings, meteoSettings);

            if (computeResiduals(myVar, myMeteoPoints, nrMeteoPoints, interpolationPoints, mySettings, meteoSettings, mySettings->getUseExcludeStationsOutsideDEM(), true))
            {
                avgError = computeErrorCrossValidation(myMeteoPoints, nrMeteoPoints);
                if (! isEqual(avgError, NODATA) && (isEqual(minError, NODATA) || avgError < minError))
                {
                    minError = avgError;
                    bestCombinationIndex = i;
                }
            }
        }
    }

    if (mySettings->getCombination(bestCombinationIndex, bestCombination))
    {
        passDataToInterpolation(myMeteoPoints, nrMeteoPoints, outInterpolationPoints, mySettings);
        detrending(outInterpolationPoints, bestCombination, mySettings, myClimate, myVar, myTime);
    }

    return;
}


bool preInterpolation(std::vector <Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* mySettings, Crit3DMeteoSettings* meteoSettings,
                      Crit3DClimateParameters* myClimate, Crit3DMeteoPoint* myMeteoPoints, int nrMeteoPoints,
                      meteoVariable myVar, Crit3DTime myTime, std::string &errorStr)
{
    if (myVar == precipitation || myVar == dailyPrecipitation)
    {
        int nrPrecNotNull;
        if (checkPrecipitationZero(myPoints, meteoSettings->getRainfallThreshold(), nrPrecNotNull))
        {
            mySettings->setPrecipitationAllZero(true);
            return true;
        }
        else
            mySettings->setPrecipitationAllZero(false);
    }

    if (getUseDetrendingVar(myVar))
    {
        if (mySettings->getUseMultipleDetrending())
        {
            if (!mySettings->getUseLocalDetrending())
                setMultipleDetrendingHeightTemperatureRange(mySettings);

            if (mySettings->getUseGlocalDetrending())
            {
                glocalDetrendingFitting(myPoints, mySettings, myVar, errorStr);
            }
            else
            {
                if (! multipleDetrendingMain(myPoints, mySettings, myVar, errorStr)) return false;
            }
        }
        else
        {
            if (mySettings->getUseBestDetrending())
                optimalDetrending(myVar, myMeteoPoints, nrMeteoPoints, myPoints, mySettings, meteoSettings, myClimate, myTime);
            else
                detrending(myPoints, mySettings->getSelectedCombination(), mySettings, myClimate, myVar, myTime);
        }
    }

    if (mySettings->getUseTD() && getUseTdVar(myVar) && ! mySettings->getUseGlocalDetrending())
    {
        topographicDistanceOptimize(myVar, myMeteoPoints, nrMeteoPoints, myPoints, mySettings, meteoSettings);
    }

    return true;
}


float interpolate(vector <Crit3DInterpolationDataPoint> &myPoints, Crit3DInterpolationSettings* mySettings, Crit3DMeteoSettings* meteoSettings,
                  meteoVariable myVar, float myX, float myY, float myZ, std::vector <double> myProxyValues,
                  bool excludeSupplemental)

{
    if ((myVar == precipitation || myVar == dailyPrecipitation) && mySettings->getPrecipitationAllZero())
        return 0.;

    float myResult = NODATA;

    computeDistances(myVar, myPoints, mySettings, myX, myY, myZ, excludeSupplemental);

    if (! mySettings->getUseRetrendOnly())
    {
        if (mySettings->getInterpolationMethod() == idw)
        {
            myResult = inverseDistanceWeighted(myPoints);
        }
        else if (mySettings->getInterpolationMethod() == shepard)
        {
            myResult = shepardIdw(myPoints, mySettings, myX, myY);
        }
        else if (mySettings->getInterpolationMethod() == shepard_modified)
        {
            float radius = NODATA;
            if (mySettings->getUseLocalDetrending()) radius = mySettings->getLocalRadius();
            myResult = modifiedShepardIdw(myPoints, mySettings, radius, myX, myY);
        }
        //else if (mySettings->getInterpolationMethod() == kriging)
        //    myResult = NODATA;  //TODO
    }
    else myResult = 0;

    if (isEqual(myResult, NODATA))
    {
        return NODATA;
    }
    else if (!mySettings->getUseDoNotRetrend())
        myResult += retrend(myVar, myProxyValues, mySettings);

    if (myVar == precipitation || myVar == dailyPrecipitation)
    {
        if (myResult < meteoSettings->getRainfallThreshold())
            return 0.;
    }
    else if (myVar == airRelHumidity || myVar == dailyAirRelHumidityAvg
             || myVar == dailyAirRelHumidityMax || myVar == dailyAirRelHumidityMin)
        myResult = MAXVALUE(MINVALUE(myResult, 100), 0);
    else if (myVar == dailyAirTemperatureRange || myVar == leafWetness || myVar == dailyLeafWetness
             || myVar == globalIrradiance || myVar == dailyGlobalRadiation || myVar == atmTransmissivity
             || myVar == windScalarIntensity || myVar == windVectorIntensity || myVar == dailyWindScalarIntensityAvg || myVar == dailyWindScalarIntensityMax || myVar == dailyWindVectorIntensityAvg || myVar == dailyWindVectorIntensityMax
             || myVar == atmPressure)
        myResult = MAXVALUE(myResult, 0);

    return myResult;
}


bool getMultipleDetrendingValues(Crit3DInterpolationSettings &mySettings, const std::vector<double> &allProxyValues,
                                 std::vector<double> &activeProxyValues,
                                 std::vector< std::function<double(double, std::vector<double>&)> > &myFunc,
                                 std::vector<std::vector<double>> &myParameters)
{
    //this function should be used for multiple detrending only, since it loads the elevation proxy before the others
    Crit3DProxyCombination myCombination = mySettings.getCurrentCombination();

    if (allProxyValues.size() != myCombination.getProxySize())
        return false;

    activeProxyValues.clear();
    int elevationPos = NODATA;

    for (unsigned int i = 0; i < myCombination.getProxySize(); i++)
    {
        if (getProxyPragaName(mySettings.getProxy(i)->getName()) == proxyHeight && myCombination.isProxyActive(i)
            && myCombination.isProxySignificant(i))
        {
            elevationPos = i;

            if (allProxyValues[i] == NODATA)
            {
                //code for using present proxies only
                //myFunc.erase(myFunc.begin());
                //myParameters.erase(myParameters.begin());

                activeProxyValues.clear();
                return false;
            }
            else
                activeProxyValues.push_back(allProxyValues[i]);
        }
    }

    for (unsigned int i=0; i < myCombination.getProxySize(); i++)
    {
        if (i != unsigned(elevationPos) && myCombination.isProxyActive(i) && myCombination.isProxySignificant(i))
        {
            if (allProxyValues[i] == NODATA)
            {
                //code for using present proxies only
                //myFunc.erase(myFunc.begin()+activeProxyValues.size());
                //myParameters.erase(myParameters.begin()+activeProxyValues.size());

                activeProxyValues.clear();
                return false;
            }
            else
                activeProxyValues.push_back(allProxyValues[i]);
        }
    }

    return (activeProxyValues.size() > 0);
}


bool getProxyValuesXY(float x, float y, Crit3DInterpolationSettings* mySettings, std::vector<double> &myValues)
{
    float myValue;
    gis::Crit3DRasterGrid* proxyGrid;
    bool proxyComplete = true;

    Crit3DProxyCombination myCombination = mySettings->getCurrentCombination();

    for (unsigned int i=0; i < mySettings->getProxyNr(); i++)
    {
        myValues[i] = NODATA;

        if (myCombination.isProxyActive(i))
        {
            proxyGrid = mySettings->getProxy(i)->getGrid();
            if (proxyGrid != nullptr && proxyGrid->isLoaded)
            {
                myValue = gis::getValueFromXY(*proxyGrid, x, y);
                if (myValue != proxyGrid->header->flag)
                    myValues[i] = myValue;
                else
                    proxyComplete = false;
            }
        }
    }

    return proxyComplete;
}

bool getSignificantProxyValuesXY(float x, float y, Crit3DInterpolationSettings* mySettings, std::vector<double> &myValues)
{
    float myValue;
    gis::Crit3DRasterGrid* proxyGrid;
    bool proxyComplete = true;

    Crit3DProxyCombination myCombination = mySettings->getCurrentCombination();

    for (unsigned int i=0; i < mySettings->getProxyNr(); i++)
    {
        myValues[i] = NODATA;

        if (myCombination.isProxyActive(i) && myCombination.isProxySignificant(i))
        {
            proxyGrid = mySettings->getProxy(i)->getGrid();
            if (proxyGrid != nullptr && proxyGrid->isLoaded)
            {
                myValue = gis::getValueFromXY(*proxyGrid, x, y);
                if (myValue != proxyGrid->header->flag)
                    myValues[i] = myValue;
                else
                    proxyComplete = false;
            }
        }
    }

    return proxyComplete;
}


float getFirstIntervalHeightValue(std::vector <Crit3DInterpolationDataPoint> &myPoints, bool useLapseRateCode)
{
    float maxPointsZ = getMaxHeight(myPoints, useLapseRateCode);
    float lowerHeight = getZmin(myPoints);
    float higherHeight = lowerHeight;
    float getFirstIntervalHeightValue = NODATA;

    while (getFirstIntervalHeightValue == NODATA && higherHeight < maxPointsZ)
    {
        higherHeight = std::min(higherHeight + 50, maxPointsZ);
        getFirstIntervalHeightValue = findHeightIntervalAvgValue(useLapseRateCode, myPoints,
                                                                 lowerHeight, higherHeight, maxPointsZ);
    }
    return getFirstIntervalHeightValue;
}

