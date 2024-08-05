# ======================================================================== #
# Copyright 2019-2024 Qi Wu                                                #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ======================================================================== #

set(COMPONENT_NAME vtkm)
set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE})

set(VTKm_ENABLE_CUDA OFF)

if(CMAKE_CUDA_COMPILER)
    find_package(CUDA REQUIRED)

    # adapted from https://stackoverflow.com/a/69353718
    include(FindCUDA/select_compute_arch)
    CUDA_DETECT_INSTALLED_GPUS(INSTALLED_GPU_CCS_1)
    string(STRIP "${INSTALLED_GPU_CCS_1}" INSTALLED_GPU_CCS_2)
    string(REPLACE " " ";" INSTALLED_GPU_CCS_3 "${INSTALLED_GPU_CCS_2}")
    string(REPLACE "." "" CUDA_ARCH_LIST "${INSTALLED_GPU_CCS_3}")
    if (NOT PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CUDA_ARCHITECTURES ${CUDA_ARCH_LIST} PARENT_SCOPE)
    endif()
    set(CMAKE_CUDA_ARCHITECTURES ${CUDA_ARCH_LIST} CACHE STRING "CUDA architectures" FORCE)
    message(STATUS "Automatically detected GPU architectures: ${CUDA_ARCH_LIST}")

    set(VTKm_ENABLE_CUDA ON)
    set(VTKm_EXTRA_CUDA_ARGS     
        -DCMAKE_CUDA_HOST_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_CUDA_ARCHITECTURES=${CMAKE_CUDA_ARCHITECTURES}
    )
endif()

ExternalProject_Add(${COMPONENT_NAME}
  PREFIX ${COMPONENT_NAME}
  DOWNLOAD_DIR ${COMPONENT_NAME}
  STAMP_DIR ${COMPONENT_NAME}/stamp
  SOURCE_DIR ${COMPONENT_NAME}/src
  BINARY_DIR ${COMPONENT_NAME}/build
  URL "https://github.com/Kitware/VTK-m/archive/refs/tags/v2.1.0.zip"
  CMAKE_ARGS
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX:PATH=${COMPONENT_PATH}
    -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
    -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
    -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
    -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
    -DCMAKE_BUILD_TYPE=${DEPENDENCIES_BUILD_TYPE}
    -DVTKm_ENABLE_CUDA=${VTKm_ENABLE_CUDA} ${VTKm_EXTRA_CUDA_ARGS}
    -DVTKm_NO_DEPRECATED_VIRTUAL=ON 
    -DVTKm_USE_64BIT_IDS=OFF
    -DVTKm_USE_DOUBLE_PRECISION=ON 
    -DVTKm_USE_DEFAULT_TYPES_FOR_ASCENT=ON 
    -DVTKm_ENABLE_RENDERING=OFF
    -DVTKm_ENABLE_TESTING=OFF
    -DVTKm_ENABLE_BENCHMARKS=OFF
  BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
  BUILD_ALWAYS ${ALWAYS_REBUILD}
)

list(APPEND CMAKE_PREFIX_PATH ${COMPONENT_PATH})
