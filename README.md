[![Build Status](https://copr.fedorainfracloud.org/coprs/simc/stable/package/CRITERIA1D/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/simc/stable/package/agroTools/)

## CriteriaOutput tool
CriteriaOutput is a shell command to manage the agro-hydrological output of [CRITERIA1D / GEO](https://github.com/ARPA-SIMC/criteria1d)

#### Usage

CriteriaOutput CSV | SHAPEFILE | MAPS | NETCDF | AGGREGATION projectName.ini [date]

#### Example of project.ini

[software]  
software="CriteriaOutput"

[project]  
name=DEFICIT MACRO AREE  
db_units=./data/units.db  
db_crop=./data/crop.db  
db_data=./output/incolto.db  
add_date_to_log=false  

[csv]  
variable_list=./csvOutputList/nitrati.csv  
csv_output=./output/nitrati  
add_date_to_filename = false  

[shapefile]  
UCM=./SHAPE/UCM/UCM.shp  
field_list=./csvOutputList/nitratiToShape.csv  

[aggregation]  
aggregation_shape=./SHAPE/ZONE_QA/zone_QA.shp  
shape_field=idQA  
aggregation_list=./csvOutputList/deficitAggregation.csv  
aggregation_cellsize=100  
aggregation_threshold=0.5  
aggregation_output=./nitrati/tabDeficit_macroAree.csv  
add_date_to_filename = false  
