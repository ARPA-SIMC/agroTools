/*!
    \name hydrall.cpp
    \brief 
    \authors Antonio Volta, Caterina Toscano
	
*/


//#include <stdio.h>
#include <math.h>
#include "crit3dDate.h"
#include "commonConstants.h"
#include "hydrall.h"
#include "furtherMathFunctions.h"
#include "physics.h"

Crit3DHydrallMaps::Crit3DHydrallMaps(const gis::Crit3DRasterGrid& DEM)
{
    mapLAI = new gis::Crit3DRasterGrid;
    mapLast30DaysTavg = new gis::Crit3DRasterGrid;

    mapLAI->initializeGrid(DEM);
    mapLast30DaysTavg->initializeGrid(DEM);
}


bool computeHydrallPoint(Crit3DDate myDate, double myTemperature, double myElevation, int secondPerStep)
{
    getCO2(myDate, myTemperature, myElevation);

    // da qui in poi bisogna fare un ciclo su tutte le righe e le colonne

    double actualLAI = getLAI();
    /* necessaria per ogni specie:
     *  il contenuto di clorofilla (g cm-2) il default è 500
     *  lo spessore della foglia 0.2 cm default
     *  un booleano che indichi se la specie è anfistomatica oppure no
     *  parametro alpha del modello di Leuning
     *
    */
    // la temperatura del mese precedente arriva da fuori



    return true;
}

double getCO2(Crit3DDate myDate, double myTemperature, double myElevation)
{
    double atmCO2 ; //https://www.eea.europa.eu/data-and-maps/daviz/atmospheric-concentration-of-carbon-dioxide-5/download.table
    double year[24] = {1750,1800,1850,1900,1910,1920,1930,1940,1950,1960,1970,1980,1990,2000,2010,2020,2030,2040,2050,2060,2070,2080,2090,2100};
    double valueCO2[24] = {278,283,285,296,300,303,307,310,311,317,325,339,354,369,389,413,443,473,503,530,550,565,570,575};

    interpolation::linearInterpolation(double(myDate.year), year, valueCO2, 24);

    // exponential fitting Mauna Loa
    /*if (myDate.year < 1990)
    {
        atmCO2= 280 * exp(0.0014876*(myDate.year -1840));//exponential change in CO2 concentration (ppm)
    }
    else
    {
        atmCO2= 353 * exp(0.00630*(myDate.year - 1990));
    }
*/
    atmCO2 += 3*cos(2*PI*getDoyFromDate(myDate)/365.0);		     // to consider the seasonal effects
    return atmCO2*getPressureFromElevation(myTemperature, myElevation)/1000000  ;   // [Pa] in +- ppm/10
}

double getPressureFromElevation(double myTemperature, double myElevation)
{
    return SEA_LEVEL_PRESSURE * exp((- GRAVITY * M_AIR * myElevation) / (R_GAS * myTemperature));
}

double getLAI()
{
    // TODO
    return 4;
}

double photosynthesisAndTranspiration()
{
    TweatherDerivedVariable weatherDerivedVariable;

    return 0;
}

void weatherVariables(double myInstantTemp,double myRelativeHumidity,double myCloudiness,TweatherDerivedVariable weatherDerivedVariable)
{
    // taken from Hydrall Model, Magnani UNIBO
    weatherDerivedVariable.airVapourPressure = saturationVaporPressure(myInstantTemp)*myRelativeHumidity/100.;
    weatherDerivedVariable.emissivitySky = 1.24 * pow((weatherDerivedVariable.airVapourPressure/100.0) / (myInstantTemp+ZEROCELSIUS),(1.0/7.0))*(1 - 0.84*myCloudiness)+ 0.84*myCloudiness;
    weatherDerivedVariable.longWaveIrradiance = pow(myInstantTemp+ZEROCELSIUS,4) * weatherDerivedVariable.emissivitySky * STEFAN_BOLTZMANN ;
    weatherDerivedVariable.slopeSatVapPressureVSTemp = 2588464.2 / pow(240.97 + myInstantTemp, 2) * exp(17.502 * myInstantTemp / (240.97 + myInstantTemp)) ;
}

void hydrall::radiationAbsorption(double mySunElevation, double leafAreaIndex)
{
    // taken from Hydrall Model, Magnani UNIBO
    double sineSolarElevation;
    // TODO chiedere a Magnani questi parametri
    static double   leafAbsorbanceNIR= 0.2;
    static double   hemisphericalIsotropyParameter = 0. ; // in order to change the hemispherical isotropy from -0.4 to 0.6 Wang & Leuning 1998
    static double   clumpingParameter = 1.0 ; // from 0 to 1 <1 for needles
    double  diffuseLightSector1K,diffuseLightSector2K,diffuseLightSector3K ;
    double scatteringCoefPAR, scatteringCoefNIR ;
    double dum[17];
    double  sunlitAbsorbedNIR ,  shadedAbsorbedNIR, sunlitAbsorbedLW , shadedAbsorbedLW;
    double directIncomingPAR, directIncomingNIR , diffuseIncomingPAR , diffuseIncomingNIR,leafAbsorbancePAR;
    double directReflectionCoefficientPAR , directReflectionCoefficientNIR , diffuseReflectionCoefficientPAR , diffuseReflectionCoefficientNIR;
    //projection of the unit leaf area in the direction of the sun's beam, following Sellers 1985 (in Wang & Leuning 1998)
    double diffuseLightK,diffuseLightKPAR,diffuseLightKNIR;
    double directLightK,directLightKPAR,directLightKNIR;

    sineSolarElevation = MAXVALUE(0.0001,sin(mySunElevation*DEG_TO_RAD));
    directLightK = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*sineSolarElevation) - pow(hemisphericalIsotropyParameter,2)*(0.33-0.579*sineSolarElevation))/ sineSolarElevation;

    /*Extinction coeff for canopy of black leaves, diffuse radiation
    The average extinctio coefficient is computed considering three sky sectors,
    assuming SOC conditions (Goudriaan & van Laar 1994, p 98-99)*/
    diffuseLightSector1K = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*0.259) - hemisphericalIsotropyParameter*hemisphericalIsotropyParameter*(0.33-0.579*0.259))/0.259;  //projection of unit leaf for first sky sector (0-30 elevation)
    diffuseLightSector2K = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*0.707) - hemisphericalIsotropyParameter*hemisphericalIsotropyParameter*(0.33-0.579*0.707))/0.707 ; //second sky sector (30-60 elevation)
    diffuseLightSector3K = (0.5 - hemisphericalIsotropyParameter*(0.633-1.11*0.966) - hemisphericalIsotropyParameter*hemisphericalIsotropyParameter*(0.33-0.579*0.966))/ 0.966 ; // third sky sector (60-90 elevation)
    diffuseLightK =- 1.0/leafAreaIndex * log(0.178 * exp(-diffuseLightSector1K*leafAreaIndex) + 0.514 * exp(-diffuseLightSector2K*leafAreaIndex)
                                                                      + 0.308 * exp(-diffuseLightSector3K*leafAreaIndex));  //approximation based on relative radiance from 3 sky sectors
    //Include effects of leaf clumping (see Goudriaan & van Laar 1994, p 110)
    directLightK  *= clumpingParameter ;//direct light
    diffuseLightK *= clumpingParameter ;//diffuse light
    if (0.001 < sineSolarElevation)
    {
        //Leaf area index of sunlit (1) and shaded (2) big-leaf
        sunlit.leafAreaIndex = UPSCALINGFUNC(directLightK,leafAreaIndex);
        shaded.leafAreaIndex = leafAreaIndex - sunlit.leafAreaIndex ;
        //Extinction coefficients for direct and diffuse PAR and NIR radiation, scattering leaves
        //Based on approximation by Goudriaan 1977 (in Goudriaan & van Laar 1994)
        double exponent= -pow(10,0.28 + 0.63*log10(chlorophyllContent*0.85/1000));
        leafAbsorbancePAR = 1 - pow(10,exponent);//from Agusti et al (1994), Eq. 1, assuming Chl a = 0.85 Chl (a+b)
        scatteringCoefPAR = 1.0 - leafAbsorbancePAR ; //scattering coefficient for PAR
        scatteringCoefNIR = 1.0 - leafAbsorbanceNIR ; //scattering coefficient for NIR
        dum[0]= sqrt(1-scatteringCoefPAR);
        dum[1]= sqrt(1-scatteringCoefNIR);
        diffuseLightKPAR = diffuseLightK * dum[0] ;//extinction coeff of PAR, direct light
        diffuseLightKNIR = diffuseLightK * dum[1]; //extinction coeff of NIR radiation, direct light
        directLightKPAR  = directLightK * dum[0]; //extinction coeff of PAR, diffuse light
        directLightKNIR  = directLightK * dum[1]; //extinction coeff of NIR radiation, diffuse light
        //Canopy+soil reflection coefficients for direct, diffuse PAR, NIR radiation
        dum[2]= (1-dum[0]) / (1+dum[0]);
        dum[3]= (1-dum[1]) / (1+dum[1]);
        dum[4]= 2.0 * directLightK / (directLightK + diffuseLightK) ;
        directReflectionCoefficientNIR = directReflectionCoefficientPAR = dum[4] * dum[2];
        diffuseReflectionCoefficientNIR = diffuseReflectionCoefficientPAR = dum[4] * dum[3];
        //Incoming direct PAR and NIR (W m-2)
        directIncomingNIR = directIncomingPAR = myDirectIrradiance * 0.5 ;
        //Incoming diffuse PAR and NIR (W m-2)
        diffuseIncomingNIR = diffuseIncomingPAR = myDiffuseIrradiance * 0.5 ;
        //Preliminary computations
        dum[5]= diffuseIncomingPAR * (1.0-diffuseReflectionCoefficientPAR) * diffuseLightKPAR ;
        dum[6]= directIncomingPAR * (1.0-directReflectionCoefficientPAR) * directLightKPAR ;
        dum[7]= directIncomingPAR * (1.0-scatteringCoefPAR) * directLightK ;
        dum[8]=  diffuseIncomingNIR * (1.0-diffuseReflectionCoefficientNIR) * diffuseLightKNIR;
        dum[9]= directIncomingNIR * (1.0-directReflectionCoefficientNIR) * directLightKNIR;
        dum[10]= directIncomingNIR * (1.0-scatteringCoefNIR) * directLightKNIR ;
        dum[11]= UPSCALINGFUNC((diffuseLightKPAR+directLightK),leafAreaIndex);
        dum[12]= UPSCALINGFUNC((directLightKPAR+directLightK),leafAreaIndex);
        dum[13]= UPSCALINGFUNC((diffuseLightKPAR+directLightK),leafAreaIndex);
        dum[14]= UPSCALINGFUNC((directLightKNIR+directLightK),leafAreaIndex);
        dum[15]= UPSCALINGFUNC(directLightK,leafAreaIndex) - UPSCALINGFUNC((2.0*directLightK),leafAreaIndex) ;
        // PAR absorbed by sunlit (1) and shaded (2) big-leaf (W m-2) from Wang & Leuning 1998
        sunlit.absorbedPAR = dum[5] * dum[11] + dum[6] * dum[12] + dum[7] * dum[15] ;
        shaded.absorbedPAR = dum[5]*(UPSCALINGFUNC(diffuseLightKPAR,leafAreaIndex)- dum[11]) + dum[6]*(UPSCALINGFUNC(directLightKPAR,leafAreaIndex)- dum[12]) - dum[7] * dum[15];
        // NIR absorbed by sunlit (1) and shaded (2) big-leaf (W m-2) fromWang & Leuning 1998
        sunlitAbsorbedNIR = dum[8]*dum[13]+dum[9]*dum[14]+dum[10]*dum[15];
        shadedAbsorbedNIR = dum[8]*(UPSCALINGFUNC(diffuseLightKNIR,leafAreaIndex)-dum[13])+dum[9]*(UPSCALINGFUNC(directLightKNIR,leafAreaIndex)- dum[14]) - dum[10] * dum[15];
        // Long-wave radiation balance by sunlit (1) and shaded (2) big-leaf (W m-2) from Wang & Leuning 1998
        dum[16]= myLongWaveIrradiance -STEFAN_BOLTZMANN*pow(myInstantTemp+ZEROCELSIUS,4); //negativo
        dum[16] *= diffuseLightK ;
        double emissivityLeaf, emissivitySoil;
        emissivityLeaf = 0.96 ; // supposed constant because variation is very small
        emissivitySoil= 0.94 ;   // supposed constant because variation is very small
        sunlitAbsorbedLW = (dum[16] * UPSCALINGFUNC((directLightK+diffuseLightK),leafAreaIndex))*emissivityLeaf+(1.0-emissivitySoil)*(emissivityLeaf-myEmissivitySky)* UPSCALINGFUNC((2*diffuseLightK),leafAreaIndex)* UPSCALINGFUNC((directLightK-diffuseLightK),leafAreaIndex);
        shadedAbsorbedLW = dum[16] * UPSCALINGFUNC(diffuseLightK,leafAreaIndex) - sunlitAbsorbedLW ;
        // Isothermal net radiation for sunlit (1) and shaded (2) big-leaf
        sunlit.isothermalNetRadiation= sunlit.absorbedPAR + sunlitAbsorbedNIR + sunlitAbsorbedLW ;
        shaded.isothermalNetRadiation = shaded.absorbedPAR + shadedAbsorbedNIR + shadedAbsorbedLW ;
    }
    else
    {
        sunlit.leafAreaIndex =  0.0 ;
        sunlit.absorbedPAR = 0.0 ;

        // TODO: non servono?
        sunlitAbsorbedNIR = 0.0 ;
        sunlitAbsorbedLW = 0.0 ;
        sunlit.isothermalNetRadiation =  0.0 ;

        shaded.leafAreaIndex = leafAreaIndex;
        shaded.absorbedPAR = 0.0 ;
        shadedAbsorbedNIR = 0.0 ;
        dum[16]= myLongWaveIrradiance -STEFAN_BOLTZMANN*pow(myInstantTemp + ZEROCELSIUS,4) ;
        dum[16] *= diffuseLightK ;
        shadedAbsorbedLW= dum[16] * (UPSCALINGFUNC(diffuseLightK,leafAreaIndex) - UPSCALINGFUNC(directLightK+diffuseLightK,leafAreaIndex)) ;
        shaded.isothermalNetRadiation = shaded.absorbedPAR + shadedAbsorbedNIR + shadedAbsorbedLW ;
    }

    // Convert absorbed PAR into units of mol m-2 s-1
    sunlit.absorbedPAR *= 4.57E-6 ;
    shaded.absorbedPAR *= 4.57E-6 ;
}
/*
double meanLastMonthTemperature(double previousLastMonthTemp, double simulationStepInSeconds, double myInstantTemp)
{
    double newTemperature;
    double monthFraction;
    monthFraction = simulationStepInSeconds/(2592000.0); // seconds of 30 days
    newTemperature = previousLastMonthTemp * (1 - monthFraction) + myInstantTemp * monthFraction ;
    return newTemperature;
}*/
