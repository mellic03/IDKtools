#!/bin/bash

# Delete old files
rm -R ./output/assets

# Build project
mkdir ./build
cd ./build
cmake -G Ninja ..
ninja -j 6
cd ..


mkdir ./output


# MapIBL
# ----------------------------------------------------------------------------------------------
mkdir ./output/MapIBL
mkdir ./output/MapIBL/shaders
mkdir ./output/MapIBL/input
mkdir ./output/MapIBL/output

cp -R ./src/shaders/. ./output/MapIBL/shaders/.

cp ./src/mapIBL.sh ./output/MapIBL/.

cp ./build/diffuseIBL ./output/MapIBL/diffuseIBL
cp ./build/specularIBL ./output/MapIBL/specularIBL
# ----------------------------------------------------------------------------------------------


# ModelConvert
# ----------------------------------------------------------------------------------------------
mkdir ./output/ModelConvert
cp -R ./external/lib/. ./output/ModelConvert/.
cp ./build/modelconvert ./output/ModelConvert/modelconvert
# ----------------------------------------------------------------------------------------------

