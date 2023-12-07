#!/bin/bash

# Build project
mkdir -p ./build
cd ./build
cmake -G Ninja ..
ninja -j 6
cd ..


mkdir -p ./output/IDKGE

cp -R external/IDKGE/shipping output/IDKGE/.
cp external/lib/* output/IDKGE/shipping/.


# MapIBL
# ----------------------------------------------------------------------------------------------
mkdir -p ./output/MapIBL/{shaders,input,output}
cp src/MapIBL/mapIBL.sh output/MapIBL/.

cp -R src/MapIBL/shaders output/MapIBL/.
cp ./build/diffuseIBL ./output/MapIBL/diffuseIBL
cp ./build/specularIBL ./output/MapIBL/specularIBL
# ----------------------------------------------------------------------------------------------


# ModelConvert
# ----------------------------------------------------------------------------------------------
mkdir -p ./output/ModelConvert
# cp -R ./external/lib/. ./output/ModelConvert/.
cp ./build/modelconvert ./output/ModelConvert/modelconvert
# ----------------------------------------------------------------------------------------------

