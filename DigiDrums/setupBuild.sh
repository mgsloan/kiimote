#!/bin/sh
mkdir debugBuild
mkdir releaseBuild
cd debugBuild
cmake ../ -DCMAKE_BUILD_TYPE=Debug
cd ../releaseBuild
cmake ../ -DCMAKE_BUILD_TYPE=Release
