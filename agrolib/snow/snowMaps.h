#ifndef SNOWMAPS_H
#define SNOWMAPS_H

    #ifndef SNOW_H
        #include "snow.h"
    #endif

    class Crit3DSnowMaps
    {
    public:
        bool isInitialized;

        Crit3DSnowMaps();
        ~Crit3DSnowMaps();

        void clear();
        void initialize(const gis::Crit3DRasterGrid &dtm, double snowSkinThickness);
        void resetSnowModel(double snowSkinThickness);

        void updateMap(Crit3DSnow &snowPoint, int row, int col);
        void setPoint(Crit3DSnow &snowPoint, int row, int col);

        gis::Crit3DRasterGrid* getSnowWaterEquivalentMap();
        gis::Crit3DRasterGrid* getSnowFallMap();
        gis::Crit3DRasterGrid* getSnowMeltMap();
        gis::Crit3DRasterGrid* getIceContentMap();
        gis::Crit3DRasterGrid* getLWContentMap();
        gis::Crit3DRasterGrid* getInternalEnergyMap();
        gis::Crit3DRasterGrid* getSurfaceInternalEnergyMap();
        gis::Crit3DRasterGrid* getSnowSurfaceTempMap();
        gis::Crit3DRasterGrid* getAgeOfSnowMap();

    private:
        gis::Crit3DRasterGrid* _snowWaterEquivalentMap;
        gis::Crit3DRasterGrid* _snowFallMap;
        gis::Crit3DRasterGrid* _snowMeltMap;
        gis::Crit3DRasterGrid* _iceContentMap;
        gis::Crit3DRasterGrid* _liquidWaterContentMap;
        gis::Crit3DRasterGrid* _internalEnergyMap;
        gis::Crit3DRasterGrid* _surfaceInternalEnergyMap;
        gis::Crit3DRasterGrid* _snowSurfaceTempMap;
        gis::Crit3DRasterGrid* _ageOfSnowMap;

        double _initSoilPackTemp;
        double _initSnowSurfaceTemp;
    };


#endif // SNOWMAPS_H
