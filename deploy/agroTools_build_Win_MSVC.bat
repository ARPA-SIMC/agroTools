:: build AGROTOOLS
:: run on Qt shell (MSVC version)
:: inside deploy directory (cd [local path]\AGROTOOLS\deploy)
:: before the execution call vcvarsall.bat (32 or 64 bit) to complete environment setup
:: example: C:\"Program Files (x86)"\"Microsoft Visual Studio"\2017\Community\VC\Auxiliary\Build\vcvars64.bat


:: clean all
cd ..\bin\csv2dbMeteo
nmake /S /NOLOGO distclean
cd ..\Makeall_criteriaOutputTool
nmake /S /NOLOGO distclean
cd ..\Makeall_csv2dbGrid
nmake /S /NOLOGO distclean

:: compile
cd ..\csv2dbMeteo
qmake CONFIG+=release
nmake /S /NOLOGO release

cd ..\Makeall_criteriaOutputTool
qmake CONFIG+=release
nmake /S /NOLOGO release

cd ..\Makeall_csv2dbGrid
qmake CONFIG+=release
nmake /S /NOLOGO release

:: copy executables
cd ..\..\deploy
mkdir agroTools\bin
cd agroTools\bin
copy ..\..\..\bin\criteriaOutputTool\release\CriteriaOutput.exe
copy ..\..\..\bin\csv2dbMeteo\release\Csv2dbMeteo.exe
copy ..\..\..\bin\csv2dbGrid\release\Csv2dbGrid.exe

:: deploy executables
windeployqt Csv2dbMeteo.exe
windeployqt CriteriaOutput.exe
windeployqt Csv2dbGrid.exe

:: return to deploy directory
cd ..\..\
