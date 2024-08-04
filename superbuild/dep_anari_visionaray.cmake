# ======================================================================== #
#                                                                          #
# ======================================================================== #

set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE})
list(APPEND CMAKE_PREFIX_PATH ${COMPONENT_PATH})

find_package(Git)

set(COMPONENT_NAME visionaray)
ExternalProject_Add(${COMPONENT_NAME}
  PREFIX ${COMPONENT_NAME}
  DOWNLOAD_DIR ${COMPONENT_NAME}
  STAMP_DIR ${COMPONENT_NAME}/stamp
  SOURCE_DIR ${COMPONENT_NAME}/src
  BINARY_DIR ${COMPONENT_NAME}/build
  GIT_REPOSITORY "https://github.com/szellmann/visionaray.git"
  GIT_TAG "f400667340c8e7a0ba008dae10a51ee2eb5e6e47"
  PATCH_COMMAND ${GIT_EXECUTABLE} apply ${CMAKE_CURRENT_SOURCE_DIR}/../anari/visionaray.patch
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${COMPONENT_PATH}
    -DINSTALL_IN_SEPARATE_DIRECTORIES=OFF
    -DVSNRAY_ENABLE_EXAMPLES:BOOL=OFF 
    -DVSNRAY_ENABLE_VIEWER:BOOL=OFF
    -DVSNRAY_ENABLE_COMMON:BOOL=OFF
    -DVSNRAY_ENABLE_CUDA:BOOL=OFF
)

# cmake anari-visionaray/ -B anari-visionaray/build -DBUILD_SHARED_LIBS:BOOL=ON -DANARI_VISIONARAY_ENABLE_CUDA:BOOL=OFF -DCMAKE_INSTALL_PREFIX=anari-visionaray/build/install -Danari_DIR=/home/qiwu/projects/anari_superbuild/deps/superbuild/install/lib64/cmake/anari-0.8.0/ -Dvisionaray_DIR=/home/qiwu/projects/anari_superbuild/anari/opt/lib/cmake/visionaray
set(COMPONENT_NAME anari-visionaray)
ExternalProject_Add(${COMPONENT_NAME}
  PREFIX ${COMPONENT_NAME}
  DOWNLOAD_DIR ${COMPONENT_NAME}
  STAMP_DIR ${COMPONENT_NAME}/stamp
  SOURCE_DIR ${COMPONENT_NAME}/src
  BINARY_DIR ${COMPONENT_NAME}/build
  GIT_REPOSITORY "https://github.com/szellmann/anari-visionaray.git"
  GIT_TAG "88a6b49e1b24c5edbe1096e5a3e89e945eb5dc22"
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${COMPONENT_PATH}
    -DBUILD_SHARED_LIBS:BOOL=ON 
    -DANARI_VISIONARAY_ENABLE_CUDA:BOOL=OFF
)

ExternalProject_Add_StepDependencies(anari-visionaray configure visionaray)
