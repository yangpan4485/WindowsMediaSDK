set(DEMO_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    )
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${DEMO_SOURCE})
# file_source_group(${DEMO_SOURCE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/video_encoder)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mediasdk/video_render)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../DuiLib)
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

add_executable(video_encoder_demo ${DEMO_SOURCE})
target_link_libraries(video_encoder_demo mediasdk)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT video_encoder_demo)