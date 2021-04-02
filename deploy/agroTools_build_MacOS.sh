#!/bin/bash

# specify your Qt directory
QT_DIR=/Users/gabrieleantolini/Qt/5.15.2/clang_64/bin
QMAKE=$QT_DIR/qmake
QDEPLOY=$QT_DIR/macdeployqt

# build csv2dbMeteo
cd ../bin/csv2dbMeteo
$QMAKE csv2dbMeteo.pro -spec macx-clang CONFIG+=release CONFIG+=force_debug_info CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build CRITERIAOUTPUT
cd ../bin/Makeall_criteriaOutputTool
$QMAKE Makeall_criteriaOutputTool.pro -spec macx-clang CONFIG+=release CONFIG+=force_debug_info CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build csv2dbGrid
cd ../bin/Makeall_csv2dbGrid
$QMAKE Makeall_csv2dbGrid.pro -spec macx-clang CONFIG+=release CONFIG+=force_debug_info CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

mkdir agroTools
mkdir agroTools/bin

cp -r ../bin/csv2dbMeteo/release/Csv2dbMeteo agroTools/bin/Csv2dbMeteo
cp -r ../bin/criteriaOutputTool/release/CriteriaOutput agroTools/bin/CriteriaOutput
cp -r ../bin/csv2dbGrid/release/Csv2dbGrid agroTools/bin/Csv2dbGrid

# deploy apps
cd agroTools/bin
$DEPLOY Csv2dbMeteo.app
$DEPLOY CriteriaOutput.app
$DEPLOY Csv2dbGrid.app

