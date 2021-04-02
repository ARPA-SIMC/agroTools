#!/bin/bash

# specify your qmake directory
QT_DIR=/opt/Qt/5.12.8/gcc_64/
QMAKE=$QT_DIR/bin/qmake

# build csv2dbMeteo
cd ../bin/csv2dbMeteo
$QMAKE csv2dbMeteo.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build CRITERIAOUTPUT
cd ../bin/Makeall_criteriaOutputTool
$QMAKE Makeall_criteriaOutputTool.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# build csv2dbGrid
cd ../bin/Makeall_csv2dbGrid
$QMAKE Makeall_csv2dbGrid.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=/usr
make -f Makefile clean
make -f Makefile qmake_all
make 

cd -

# download linuxdeployqt
wget -c -nv -O linuxqtdeploy "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod +x linuxqtdeploy

function make_appimage {

    BIN_PATH=$1
    BIN_NAME=`basename $1`

    rm -rf build
    # make tree and copy executables and images
    cp -rf appimage build
    
    cp $BIN_PATH build/usr/bin/$BIN_NAME
    ./linuxqtdeploy --appimage-extract-and-run build/usr/share/applications/$BIN_NAME.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
}

# build appimage csv2dbMeteo
make_appimage ../bin/csv2dbMeteo/release/Csv2dbMeteo

# build appimage CriteriaOutput
make_appimage ../bin/criteriaOutputTool/release/CriteriaOutput

# build appimage csv2dbGrid
make_appimage ../bin/csv2dbGrid/release/Csv2dbGrid

mkdir -p agroTools/bin
mv *.AppImage agroTools/bin/


