# ======================================================================== #
#                                                                          #
# ======================================================================== #

option(BUILD_TBB_FROM_SOURCE "Build TBB from source or use pre-built version?" OFF)
option(BUILD_EMBREE_FROM_SOURCE "Build Embree or use pre-built version?" OFF)

set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE})
list(APPEND CMAKE_PREFIX_PATH ${COMPONENT_PATH})

set(COMPONENT_NAME ospray)
ExternalProject_Add(${COMPONENT_NAME}
  PREFIX ${COMPONENT_NAME}
  DOWNLOAD_DIR ${COMPONENT_NAME}
  STAMP_DIR ${COMPONENT_NAME}/stamp
  SOURCE_DIR ${COMPONENT_NAME}/src
  BINARY_DIR ${COMPONENT_NAME}/build
  URL https://github.com/ospray/ospray/archive/v3.1.0.zip
  SOURCE_SUBDIR scripts/superbuild
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${COMPONENT_PATH}
    -DINSTALL_IN_SEPARATE_DIRECTORIES=OFF
    -DBUILD_EMBREE_FROM_SOURCE=${BUILD_EMBREE_FROM_SOURCE}
    -DBUILD_TBB_FROM_SOURCE=${BUILD_TBB_FROM_SOURCE}
    -DBUILD_GLFW=OFF
    -DBUILD_OSPRAY_APPS=OFF
  INSTALL_COMMAND ""
)

set(COMPONENT_NAME anari-ospray)
ExternalProject_Add(${COMPONENT_NAME}
  PREFIX ${COMPONENT_NAME}
  DOWNLOAD_DIR ${COMPONENT_NAME}
  STAMP_DIR ${COMPONENT_NAME}/stamp
  SOURCE_DIR ${COMPONENT_NAME}/src
  BINARY_DIR ${COMPONENT_NAME}/build
  GIT_REPOSITORY "https://github.com/ospray/anari-ospray.git"
  GIT_TAG "main"
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${COMPONENT_PATH}
)

ExternalProject_Add_StepDependencies(anari-ospray configure ospray anari)
