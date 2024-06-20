# ======================================================================== #
#                                                                          #
# ======================================================================== #

set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE})
list(APPEND CMAKE_PREFIX_PATH ${COMPONENT_PATH})

set(COMPONENT_NAME anari-barney)
ExternalProject_Add(${COMPONENT_NAME}
  PREFIX ${COMPONENT_NAME}
  DOWNLOAD_DIR ${COMPONENT_NAME}
  STAMP_DIR  ${COMPONENT_NAME}/stamp
  BINARY_DIR ${COMPONENT_NAME}/build
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../anari/barney
  BUILD_ALWAYS TRUE
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${COMPONENT_PATH}
    -DBARNEY_BUILD_ANARI=ON
    -DOWL_BUILD_SAMPLES=OFF
)

ExternalProject_Add_StepDependencies(anari-barney configure anari)
