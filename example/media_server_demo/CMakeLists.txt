set(DEMO_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket_server.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/socket_server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/main_window.h
    ${CMAKE_CURRENT_SOURCE_DIR}/main_window.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/my_window.h
    ${CMAKE_CURRENT_SOURCE_DIR}/my_window.cc
    )
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${DEMO_SOURCE})
add_definitions(-DSTB_IMAGE_IMPLEMENTATION)
# file_source_group(${DEMO_SOURCE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/camera_capture)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/video_render)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/audio_capture)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/local_log)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/screen_capture)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/video_decoder)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/video_encoder)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../DuiLib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/include/yuv)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/output/bin/${Configuration})


link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/lib/detours)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/lib/opengl)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/lib/openh264)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/lib/sdl2)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/lib/x264)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/lib/${Configuration}/ffmpeg)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/lib/${Configuration}/mfx)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/lib/${Configuration}/x265)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/lib/${Configuration}/yuv)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third/lib/${Configuration}/jpeg-turbo)

add_executable(media_server_demo ${DEMO_SOURCE})
target_link_libraries(media_server_demo mediasdk duilib)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT media_server_demo)

add_custom_command(TARGET media_server_demo
                   POST_BUILD
                   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../script/copy_file.bat
                   COMMENT "copy file")