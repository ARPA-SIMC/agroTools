[![Build Status](https://copr.fedorainfracloud.org/coprs/simc/stable/package/CRITERIA1D/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/simc/stable/package/agroTools/)

## CriteriaOutput tool
CriteriaOutput is a shell command to manage the agro-hydrological output of [CRITERIA1D / GEO](https://github.com/ARPA-SIMC/criteria1d).  
The output of CRITERIA is stored in a SQLite database and can be exported to CSV, shapefile, NetCDF or geo-raster format, using this tool.

#### Usage

>CriteriaOutput <DTX | CSV | SHAPEFILE | MAPS | NETCDF | AGGREGATION> <projectName.ini> [computationDate]

ComputationDate must be in YYYY-MM-DD format, default date is today.

#### Example of project.ini

>[software]  
software="CriteriaOutput"
>
>[project]  
name=BOLLAGRO
db_comp_units=../INCOLTO/data/comp_units_ER_incolto.db  
db_crop=../INCOLTO/data/crop.db  
db_data=../INCOLTO/output/incolto.db  
db_data_climate=../INCOLTO/output/incolto_storico.db  
add_date_to_log=false
>
>[csv]  
variable_list=./csvOutputList/bollettino.csv  
csv_output=./output/bollettino  
add_date_to_filename = false  
>
>[shapefile]  
UCM=./SHAPE/UCM_2021/ER_CompUnits_2021.shp  
field_list=./csvOutputList/bollettinoToShape.csv  
>
>[maps]  
map_list=./csvOutputList/bollettinoToMaps.csv  
palette_path=./palette/  
format = tif  
projection=3857  
cellsize = 200  
png_copy = true  
