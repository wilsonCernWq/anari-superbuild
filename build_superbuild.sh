#!/bin/bash -l

mkdir -p superbuild
cd superbuild

cmake ../../superbuild \
    -DBUILD_GLAD=OFF \
    -DBUILD_GLFW=OFF \
    -DBUILD_ANARI_SDK=ON \
    -DBUILD_ICET=OFF \
    -DBUILD_PYBIND11=OFF \
    -DBUILD_SZ3=OFF \
    -DBUILD_ZFP=OFF \
    || exit 1
cmake --build . --config Release -j16  || exit 1

cd -
