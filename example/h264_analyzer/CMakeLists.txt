set(DEMO_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bs.h
    )
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${DEMO_SOURCE})
# file_source_group(${DEMO_SOURCE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/output/bin/${Configuration})

add_executable(h264_analyzer_demo ${DEMO_SOURCE})

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT h264_analyzer_demo)

# add_custom_command(TARGET video_capture_demo
#                   POST_BUILD
#                   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/copy_file.bat
#                   COMMENT "copy file")