file(GLOB_RECURSE COMMON_SOURCE "common/*.cc" "common/*.h")
file(GLOB_RECURSE AUDIO_CAPTURE_SOURCE "audio_capture/*.cc" "audio_capture/*.h")
file(GLOB_RECURSE LOCAL_LOG_SOURCE "local_log/*.cc" "local_log/*.h")
file(GLOB_RECURSE CAMERA_CAPTURE_SOURCE "camera_capture/*.cc" "camera_capture/*.h")
file(GLOB_RECURSE VIDEO_RENDER_SOURCE "video_render/*.cc" "video_render/*.h")
file(GLOB_RECURSE SAMPLE_COMMON_SOURCE "sample_common/*.cc" "sample_common/*.cpp" "sample_common/*.h")
file(GLOB_RECURSE SCREEN_CAPTURE_SOURCE "screen_capture/*.cc" "screen_capture/*.h")
file(GLOB_RECURSE VIDEO_DECODER_SOURCE "video_decoder/*.cc" "video_decoder/*.h")
file(GLOB_RECURSE VIDEO_ENCODER_SOURCE "video_encoder/*.cc" "video_encoder/*.h")

set(MEDIA_SOURCE ${AUDIO_CAPTURE_SOURCE}
                 ${COMMON_SOURCE}
                 ${LOCAL_LOG_SOURCE}
                 ${CAMERA_CAPTURE_SOURCE}
                 ${VIDEO_RENDER_SOURCE}
                 ${SAMPLE_COMMON_SOURCE}
                 ${SCREEN_CAPTURE_SOURCE}
                 ${VIDEO_DECODER_SOURCE}
                 ${VIDEO_ENCODER_SOURCE})

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${MEDIA_SOURCE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/audio_capture)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/local_log)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/camera_capture)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/video_render)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sample_common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sample_common/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/screen_capture)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/video_decoder)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/video_encoder)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include/Detours)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include/ffmpeg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include/GL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include/jpeg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include/libx264)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include/libx265)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include/mfx)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include/openh264)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include/SDL2)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include/yuv)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/lib/detours)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/lib/opengl)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/lib/openh264)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/lib/sdl2)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/lib/x264)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/lib/${Configuration}/ffmpeg)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/lib/${Configuration}/mfx)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/lib/${Configuration}/x265)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/lib/${Configuration}/yuv)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/lib/${Configuration}/jpeg-turbo)

add_library(mediasdk ${MEDIA_SOURCE})

target_link_libraries(mediasdk PRIVATE glew32
                                       glew32s
                                       OpenGL32
                                       SDL2
                                       SDL2main
                                       d3d9
                                       strmiids
                                       mfplat
                                       mf
                                       mfreadwrite
                                       mfuuid
                                       shlwapi
                                       dwmapi
                                       Magnification
                                       detours
                                       Gdiplus
                                       D3D11
                                       wtsapi32
                                       DXGI
                                       yuv
                                       libx264
                                       x265
                                       openh264
                                       avcodec
                                       avformat
                                       avutil
                                       swscale
                                       libmfx
                                       dxva2
                                       gdi32
                                       winmm
                                       jpeg-static)