# Ascent + ANARI InSitu Visualization Integrations

Create a build directory:
```bash
ROOT=$PWD # or where you want to build
mkdir build
```

## Dependencies

* Python

* On NVIDIA GPU, you need to install CUDA and [OptiX](https://developer.nvidia.com/rtx/ray-tracing/optix) 7+.

    And export the path to OptiX:
    ```bash
    # Linux (Bash)
    export OptiX_INSTALL_DIR="/media/data/qadwu/Software/NVIDIA-OptiX-SDK-7.4.0-linux64-x86_64"
    ```
    ```powershell
    # Windows (Powershell)
    $Env:OptiX_INSTALL_DIR = "C:\ProgramData\NVIDIA Corporation\OptiX SDK 7.4.0"
    ```

## Build Everything

The work directory is the root directory of this repository.
```bash
cd <root-directory>
```

### Generate Build Files

Linux
```bash
cd build
cmake -S . -B build 
```

Windows (Assume you have Visual Studio 2022 installed, if not, change the generator to the one you have)
```powershell
cd build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```

### Actual Compilation

For all platforms:
```bash
cmake --build build --config Release --parallel
```

Outputs are installed into `build/install`.


## Tutorials

### Tutorial 1: anariTutorialCpp

Linux:
```bash
cd build/anari/build
./anariTutorialCpp
```

Windows:
```powershell
cd build\anari\build\Release
.\anariTutorialCpp.exe
```

![tutorial_cpp.png](tutorial_cpp.png)


### Tutorial 2: anariViewer

Linux:

```bash
export LD_LIBRARY_PATH=/mnt/scratch/fast0/qadwu/anari-superbuild/build/install/lib:$LD_LIBRARY_PATH

cd build/anari/build
ANARI_LIBRARY=helide ./anariViewer 
# make sure libanari_library_ospray.so has been compiled
ANARI_LIBRARY=ospray ./anariViewer 
# make sure libanari_library_barney.so has been compiled
ANARI_LIBRARY=barney ./anariViewer 
```

Windows:

```powershell
$Env:PATH += ";E:\Projects\research\anari-superbuild\build\install\bin"
$Env:PATH += ";E:\Projects\research\anari-superbuild\build\install\redist\intel64\vc14"

cd build\anari\build\Release
$Env:ANARI_LIBRARY = "helide"
.\anariViewer 

# or ospray
$Env:ANARI_LIBRARY = "ospray"
.\anariViewer 

# or barney
$Env:ANARI_LIBRARY = "barney"
.\anariViewer 

# or visrtx
$Env:ANARI_LIBRARY = "visrtx"
.\anariViewer 
```

### Tutorial 3: Edit anariTutorialCpp

You can edit `anariTutorialCpp.cpp` to change the visualization.

Path to the file is: `build\anari\src\examples\simple\anariTutorial.cpp`

Then recompile it using:
```
cd build
cmake --build . --config Release --target demo
```

Then run it again
```
.\Release\demo.exe
```

