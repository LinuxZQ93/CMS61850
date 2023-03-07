#!/bin/sh
if [ ! -d Build ];then
    mkdir Build
fi
cd ./Build
if [ ! -n "$1" ];then
cmake .. -DCMAKE_BUILD_TYPE=Debug -Dplatform=x86
else
cmake .. -DCMAKE_BUILD_TYPE=Debug -Dplatform=$1
fi
make -j8
