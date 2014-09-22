#!/bin/sh

cd dalvikhook/jni
ndk-build
cd ../..

cd examples
cd strmon/jni
ndk-build
cd ../..

cd smsdispatch/jni
ndk-build
cd ../..

cd ..

