## Copyright 2021-2024 The Khronos Group
## SPDX-License-Identifier: Apache-2.0

find_package(anari REQUIRED COMPONENTS viewer)

find_package(VTKm REQUIRED)

# Example 1
add_executable(demo 
    anariTutorial.cpp stb_image_write.c stb_image.c
)
target_link_libraries(demo PRIVATE anari::anari)
target_include_directories(demo PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Example 2
add_executable(demoFiberTracks
    demoFiberTracks.cpp 
    stb_image_write.c stb_image.c
    PolyLineDataReader.cxx    
)
target_link_libraries(demoFiberTracks PRIVATE anari::anari
    vtkm::cont
    vtkm::io 
)
target_include_directories(demoFiberTracks PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Example 3
add_executable(demoViewer demoViewer.cpp)
target_link_libraries(demoViewer PRIVATE anari::anari_viewer)
target_include_directories(demoViewer PRIVATE ${CMAKE_CURRENT_LIST_DIR})
