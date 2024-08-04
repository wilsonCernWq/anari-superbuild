# ======================================================================== #
#                                                                          #
# ======================================================================== #
cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

## Install options ##
if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE STRING "Final install location." FORCE)
endif()
get_filename_component(INSTALL_DIR_ABSOLUTE ${CMAKE_INSTALL_PREFIX} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

# ------------------------------------------------------------------
#
# ------------------------------------------------------------------
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type." FORCE)
endif()
set(CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CONFIGURATION_TYPES})

# ------------------------------------------------------------------
## Establish project ##
# ------------------------------------------------------------------

project(anari_demo)

include(CheckLanguage)
check_language(CUDA) # Check if CUDA is available

if(POLICY CMP0114)
  # allow old CMake behavior: NO_DEPENDS for ExternalProject_Add_StepTargets
  cmake_policy(SET CMP0114 OLD) # 3.19
endif()

if (POLICY CMP0135)
  # use timestamp of extraction of downloaded archives
  cmake_policy(SET CMP0135 NEW) # 3.24
endif()

include(ExternalProject)
include(GNUInstallDirs)
include(ProcessorCount)
include(CMakeDependentOption)

# output to top-level 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/example)
