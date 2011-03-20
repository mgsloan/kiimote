#!/bin/sh
cd releaseBuild
make -j12
cd debugBuild
make -j12
