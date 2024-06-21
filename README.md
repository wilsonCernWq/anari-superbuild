# Ascent + ANARI InSitu Visualization Integrations

Create a build directory:
```bash
ROOT=$PWD # or where you want to build
mkdir build
```

## Build Anari

Outputs are installed into `build/install`.

```bash
cd build
cmake ..
make -j8
```

## Build Ascent Dependencies with Anari

Outputs are installed into `build/install`.

```bash
export PATH=/home/qiwu/cmake-3.29.2-linux-x86_64/bin:$PATH
cd build
bash ../build_ascent_cuda_polaris.sh
```

#### Changes in `build_ascent.sh`
1. Force all the dependencies to be installed in the same `install` directory.
2. Force to use the VTK-m submodule (v2.1.0) instead of downloading it from the internet.
3. Remove `CMakeCache.txt` everytime when re-running the script.


## Build Ascent

Clone Ascent:
```bash
git clone https://github.com/wilsonCernWq/ascent.git
cd ascent
git checkout develop_anari
```

Build Ascent:
```bash
mkdir build
cd build
cmake ../src -DENABLE_TESTS=OFF -DENABLE_ANARI=ON  \
    -DCMAKE_INSTALL_PREFIX="${ROOT}/build/install" \
    -DCMAKE_PREFIX_PATH="${ROOT}/build/install"    \
    -C "${ROOT}/build/ascent-config.cmake"
cmake --build . --config Release --target install -j16
```
