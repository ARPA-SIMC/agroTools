:: build AGROTOOLS
:: run on Qt shell (MSVC version)

:: before the execution call vcvarsall.bat (32 or 64 bit) to complete environment setup
:: i.e.  C:\"Program Files (x86)"\"Microsoft Visual Studio"\2017\Community\VC\Auxiliary\Build\vcvars64.bat

:: see agroTools/agrolib/gdalHandler/readme.md for the GDAL installation

:: run the batch in the deploy directory 
:: i.e.  cd [local path]\agroTools\deploy


:: clean all
cd ..\bin\csv2dbMeteo
nmake /S /NOLOGO distclean
cd ..\Makeall_csv2dbGrid
nmake /S /NOLOGO distclean
cd ..\Makeall_criteriaOutputTool
nmake /S /NOLOGO distclean
cd ..\Makeall_frostForecast
nmake /S /NOLOGO distclean

:: compile
cd ..\csv2dbMeteo
qmake CONFIG+=release
nmake /S /NOLOGO release

cd ..\Makeall_csv2dbGrid
qmake CONFIG+=release
nmake /S /NOLOGO release

cd ..\Makeall_criteriaOutputTool
qmake CONFIG+=release
nmake /S /NOLOGO release

cd ..\Makeall_frostForecast
qmake CONFIG+=release
nmake /S /NOLOGO release

:: copy executables
cd ..\..\deploy
mkdir agroTools\bin
cd agroTools\bin
copy ..\..\..\bin\csv2dbMeteo\release\Csv2dbMeteo.exe
copy ..\..\..\bin\csv2dbGrid\release\Csv2dbGrid.exe
copy ..\..\..\bin\criteriaOutputTool\release\CriteriaOutput.exe
copy ..\..\..\bin\frostForecast\release\FrostForecast.exe

:: deploy executables
windeployqt Csv2dbMeteo.exe
windeployqt Csv2dbGrid.exe
windeployqt CriteriaOutput.exe
windeployqt FrostForecast.exe

:: return to deploy directory
cd ..\..\
