# cmake file for duilib
#Author Qi Gao(monkgau@gmail.com)
#Created: 2012/09/16

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} Root_src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Control Control_src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Core Core_src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Layout Layout_src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Utils Utils_src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Control)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Layout)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Utils)

file_source_group(${Control_src})
file_source_group(${Core_src})
file_source_group(${Layout_src})
file_source_group(${Utils_src})
file_source_group(${Root_src})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
add_library(duilib SHARED ${Control_src} ${Core_src} ${Layout_src} ${Utils_src} ${Root_src})

add_definitions(-DUILIB_EXPORTS)
target_link_libraries(duilib comctl32)
set_target_properties(duilib PROPERTIES OUTPUT_NAME "duilib")
