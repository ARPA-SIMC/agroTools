#!/bin/bash

set -ex

image=$1

if [[ $image =~ ^centos:8 ]]
then
    # build csv2dbMeteo
    cd bin/csv2dbMeteo
    $QMAKE csv2dbMeteo.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make 

    cd -
    
    # build CRITERIAOUTPUT
    cd bin/Makeall_criteriaOutputTool
    $QMAKE Makeall_criteriaOutputTool.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make
    
    cd -
    
    # build csv2dbGrid
    cd bin/Makeall_csv2dbGrid
    $QMAKE Makeall_csv2dbGrid.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
    make clean
    make
    
    cd -
    
    
elif [[ $image =~ ^ubuntu: ]]
then

    # build csv2dbMeteo
    cd bin/csv2dbMeteo
    $QMAKE csv2dbMeteo.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make 

    cd -
    
    # build CRITERIAOUTPUT
    cd bin/Makeall_criteriaOutputTool
    $QMAKE Makeall_criteriaOutputTool.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make
    
    cd -
    
    # build csv2dbGrid
    cd bin/Makeall_csv2dbGrid
    $QMAKE Makeall_csv2dbGrid.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler PREFIX=$QT_DIR
    make clean
    make
    
    cd -
    
    # download linuxdeployqt
    wget -c -nv -O linuxqtdeploy "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
    chmod +x linuxqtdeploy
    
    # build appimage csv2dbMeteo
    # cp appimage in tmpbuild
    cp -rf deploy/appimage deploy/tmpbuild
    cp bin/csv2dbMeteo/release/Csv2dbMeteo deploy/tmpbuild/usr/bin/Csv2dbMeteo
    ./linuxqtdeploy --appimage-extract-and-run deploy/tmpbuild/usr/share/applications/Csv2dbMeteo.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
    cp deploy/tmpbuild/usr/bin/* deploy
    #rm tmpbuild
    rm -rf deploy/tmpbuild/
    
    # build appimage CriteriaOutput
    cp -rf deploy/appimage deploy/tmpbuild
    cp bin/criteriaOutputTool/release/CriteriaOutput deploy/tmpbuild/usr/bin/CriteriaOutput
    ./linuxqtdeploy --appimage-extract-and-run deploy/tmpbuild/usr/share/applications/CriteriaOutput.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
    cp deploy/tmpbuild/usr/bin/* deploy
    rm -rf deploy/tmpbuild/
    
    # build appimage csv2dbGrid
    cp -rf deploy/appimage deploy/tmpbuild
    cp bin/csv2dbGrid/release/Csv2dbGrid deploy/tmpbuild/usr/bin/Csv2dbGrid
    ./linuxqtdeploy --appimage-extract-and-run deploy/tmpbuild/usr/share/applications/Csv2dbGrid.desktop -qmake=$QMAKE -qmlimport=$QT_DIR/qml -appimage -always-overwrite
    cp deploy/tmpbuild/usr/bin/* deploy
    rm -rf deploy/tmpbuild/

else
    echo "Unknown image $image"
    exit 1
fi
