:: build AGROTOOLS
:: run on Qt shell (MINGW version) 
:: inside deploy directory (cd [local path]\agroTools\deploy)

:: CLEAN distribution
cd ..\bin\csv2dbMeteo
mingw32-make --silent distclean
cd ..\Makeall_criteriaOutputTool
mingw32-make --silent distclean
cd ..\Makeall_csv2dbGrid
mingw32-make --silent distclean

cd ..\..\deploy

:: build csv2dbMeteo
cd ..\bin\csv2dbMeteo
qmake -platform win32-g++ CONFIG+=release
mingw32-make --silent release

:: build criteriaOutput
cd ..\Makeall_criteriaOutputTool
qmake -platform win32-g++ CONFIG+=release
mingw32-make --silent release

:: build criteriaOutput
cd ..\Makeall_csv2dbGrid
qmake -platform win32-g++ CONFIG+=release
mingw32-make --silent release

cd ..\..\deploy

:: copy executables
mkdir agroTools\bin
cd agroTools\bin
copy ..\..\..\bin\csv2dbMeteo\release\Csv2dbMeteo.exe
copy ..\..\..\bin\criteriaOutputTool\release\CriteriaOutput.exe
copy ..\..\..\bin\csv2dbGrid\release\Csv2dbGrid.exe

:: deploy
windeployqt Csv2dbMeteo.exe
windeployqt CriteriaOutput.exe
windeployqt Csv2dbGrid.exe

:: return to deploy directory
cd ..\..\


