/*!
    \copyright 2010-2016 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    You should have received a copy of the GNU General Public License
    along with Nome-Programma.  If not, see <http://www.gnu.org/licenses/>.

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

#include <math.h>
#include "commonConstants.h"
#include "basicMath.h"
#include "snowMaps.h"
#include "snow.h"


Crit3DSnowMaps::Crit3DSnowMaps()
{
    _snowWaterEquivalentMap = new gis::Crit3DRasterGrid;
    _snowFallMap = new gis::Crit3DRasterGrid;
    _snowMeltMap = new gis::Crit3DRasterGrid;
    _iceContentMap = new gis::Crit3DRasterGrid;
    _liquidWaterContentMap = new gis::Crit3DRasterGrid;
    _internalEnergyMap = new gis::Crit3DRasterGrid;
    _surfaceInternalEnergyMap = new gis::Crit3DRasterGrid;
    _snowSurfaceTempMap = new gis::Crit3DRasterGrid;
    _ageOfSnowMap = new gis::Crit3DRasterGrid;

    _initSoilPackTemp = NODATA;
    _initSnowSurfaceTemp = NODATA;

    isInitialized = false;
}


Crit3DSnowMaps::~Crit3DSnowMaps()
{
    this->clear();
}


void Crit3DSnowMaps::clear()
{
    _snowWaterEquivalentMap->clear();
    _snowFallMap->clear();
    _snowMeltMap->clear();
    _iceContentMap->clear();
    _liquidWaterContentMap->clear();
    _internalEnergyMap->clear();
    _surfaceInternalEnergyMap->clear();
    _snowSurfaceTempMap->clear();
    _ageOfSnowMap->clear();

    _initSoilPackTemp = NODATA;
    _initSnowSurfaceTemp = NODATA;

    isInitialized = false;
}


void Crit3DSnowMaps::initialize(const gis::Crit3DRasterGrid& dtm, double snowSkinThickness)
{
    _snowFallMap->initializeGrid(dtm);
    _snowMeltMap->initializeGrid(dtm);
    _iceContentMap->initializeGrid(dtm);
    _liquidWaterContentMap->initializeGrid(dtm);
    _internalEnergyMap->initializeGrid(dtm);
    _surfaceInternalEnergyMap->initializeGrid(dtm);
    _snowSurfaceTempMap->initializeGrid(dtm);
    _ageOfSnowMap->initializeGrid(dtm);

    // TODO: pass initial temperature
    _initSoilPackTemp = 3.4;
    _initSnowSurfaceTemp = 5.0;

    _snowWaterEquivalentMap->initializeGrid(dtm);
    // initialize with zero values
    _snowWaterEquivalentMap->setConstantValueWithBase(0, dtm);

    resetSnowModel(snowSkinThickness);

    isInitialized = true;
}



void Crit3DSnowMaps::updateMap(Crit3DSnow &snowPoint, int row, int col)
{
    _snowWaterEquivalentMap->value[row][col] = float(snowPoint.getSnowWaterEquivalent());
    _snowFallMap->value[row][col] = float(snowPoint.getSnowFall());
    _snowMeltMap->value[row][col] = float(snowPoint.getSnowMelt());
    _iceContentMap->value[row][col] = float(snowPoint.getIceContent());
    _liquidWaterContentMap->value[row][col] = float(snowPoint.getLiquidWaterContent());
    _internalEnergyMap->value[row][col] = float(snowPoint.getInternalEnergy());
    _surfaceInternalEnergyMap->value[row][col] = float(snowPoint.getSurfaceInternalEnergy());
    _snowSurfaceTempMap->value[row][col] = float(snowPoint.getSnowSurfaceTemp());
    _ageOfSnowMap->value[row][col] = float(snowPoint.getAgeOfSnow());
}


void Crit3DSnowMaps::setPoint(Crit3DSnow &snowPoint, int row, int col)
{
    snowPoint.setSnowWaterEquivalent(_snowWaterEquivalentMap->value[row][col]);
    snowPoint.setIceContent(_iceContentMap->value[row][col]);
    snowPoint.setLiquidWaterContent(_liquidWaterContentMap->value[row][col]);
    snowPoint.setInternalEnergy(_internalEnergyMap->value[row][col]);
    snowPoint.setSurfaceInternalEnergy(_surfaceInternalEnergyMap->value[row][col]);
    snowPoint.setSnowSurfaceTemp(_snowSurfaceTempMap->value[row][col]);
    snowPoint.setAgeOfSnow(_ageOfSnowMap->value[row][col]);
}


void Crit3DSnowMaps::resetSnowModel(double snowSkinThickness)
{
    float initSWE;                  /*!<  [mm]     */
    int surfaceBulkDensity;         /*!<  [kg/m^3] */

    // TODO pass real bulk density for each point if available
    surfaceBulkDensity = DEFAULT_BULK_DENSITY;

    for (long row = 0; row < _snowWaterEquivalentMap->header->nrRows; row++)
    {
        for (long col = 0; col < _snowWaterEquivalentMap->header->nrCols; col++)
        {
            initSWE = _snowWaterEquivalentMap->value[row][col];
            if (! isEqual(initSWE, _snowWaterEquivalentMap->header->flag))
            {
                _snowFallMap->value[row][col] = 0;
                _snowMeltMap->value[row][col] = 0;
                _iceContentMap->value[row][col] = 0;
                _liquidWaterContentMap->value[row][col] = 0;
                _ageOfSnowMap->value[row][col] = 0;

                _snowSurfaceTempMap->value[row][col] = float(_initSnowSurfaceTemp);

                /*! from [mm] to [m] */
                initSWE /=  1000;

                _surfaceInternalEnergyMap->value[row][col] = float(computeSurfaceInternalEnergy(_initSnowSurfaceTemp, surfaceBulkDensity, initSWE, snowSkinThickness));

                _internalEnergyMap->value[row][col] = float(computeInternalEnergy(_initSoilPackTemp, surfaceBulkDensity, initSWE));
            }
        }
    }
}


gis::Crit3DRasterGrid* Crit3DSnowMaps::getSnowWaterEquivalentMap()
{
    return _snowWaterEquivalentMap;
}

gis::Crit3DRasterGrid* Crit3DSnowMaps::getSnowFallMap()
{
    return _snowFallMap;
}

gis::Crit3DRasterGrid* Crit3DSnowMaps::getSnowMeltMap()
{
    return _snowMeltMap;
}

gis::Crit3DRasterGrid* Crit3DSnowMaps::getIceContentMap()
{
    return _iceContentMap;
}

gis::Crit3DRasterGrid* Crit3DSnowMaps::getLWContentMap()
{
    return _liquidWaterContentMap;
}

gis::Crit3DRasterGrid* Crit3DSnowMaps::getInternalEnergyMap()
{
    return _internalEnergyMap;
}

gis::Crit3DRasterGrid* Crit3DSnowMaps::getSurfaceInternalEnergyMap()
{
    return _surfaceInternalEnergyMap;
}

gis::Crit3DRasterGrid* Crit3DSnowMaps::getSnowSurfaceTempMap()
{
    return _snowSurfaceTempMap;
}

gis::Crit3DRasterGrid* Crit3DSnowMaps::getAgeOfSnowMap()
{
    return _ageOfSnowMap;
}


