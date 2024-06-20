# ======================================================================== #
#                                                                          #
# ======================================================================== #

set(COMPONENT_NAME anari)
set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE})

if(BUILD_GLFW AND BUILD_GLAD)
  set(BUILD_VIEWER ON)
else()
  set(BUILD_VIEWER OFF)
endif()


ExternalProject_Add(${COMPONENT_NAME}
  PREFIX ${COMPONENT_NAME}
  DOWNLOAD_DIR ${COMPONENT_NAME}
  STAMP_DIR ${COMPONENT_NAME}/stamp
  SOURCE_DIR ${COMPONENT_NAME}/src
  BINARY_DIR ${COMPONENT_NAME}/build
  URL "https://github.com/KhronosGroup/ANARI-SDK/archive/refs/tags/v0.10.0.zip"
  CMAKE_ARGS
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX:PATH=${COMPONENT_PATH}
    -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
    -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
    -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
    -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
    -DCMAKE_BUILD_TYPE=${DEPENDENCIES_BUILD_TYPE}
    -DBUILD_VIEWER=${BUILD_VIEWER}
    -DBUILD_CTS=OFF
  BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
  BUILD_ALWAYS ${ALWAYS_REBUILD}
)

list(APPEND CMAKE_PREFIX_PATH ${COMPONENT_PATH})

if(BUILD_GLFW AND BUILD_GLAD)
  ExternalProject_Add_StepDependencies(anari configure glfw glad)
endif()
