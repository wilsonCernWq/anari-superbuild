## Copyright 2021-2024 The Khronos Group
## SPDX-License-Identifier: Apache-2.0

find_package(anari REQUIRED)

add_executable(demo anariTutorial.cpp stb_image_write.c stb_image.c)
target_link_libraries(demo PRIVATE anari::anari)
target_include_directories(demo PRIVATE ${CMAKE_CURRENT_LIST_DIR})
