cmake_minimum_required(VERSION 3.10)
project(media_sdk)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
set(CMAKE_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

function(add_source_group file_source)
  # 如果给定路径是一个绝对路径
  if (IS_ABSOLUTE "${file_source}")
    file(RELATIVE_PATH source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${file_source}")
  else()
    set(source_rel "${file_source}")
  endif()
  get_filename_component(source_dir "${source_rel}" PATH)
  string(REPLACE "/" "\\" dir_name "${source_path}")
  # message("====${source_dir}  ${file_source}")
  source_group("${source_dir}" FILES "${file_source}")
endfunction()

function(file_source_group)
  # message("start file source group")
  foreach(file_source IN ITEMS ${ARGN})
    add_source_group(${file_source})
  endforeach()
endfunction(file_source_group)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/bin/${Configuration})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/bin/${Configuration})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output/bin/${Configuration})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/output/bin/${Configuration})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mediasdk)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/DuiLib)

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/audio_capture)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/local_log)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/camera_capture)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/h264_analyzer)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/screen_capture)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/video_decoder)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/video_encoder)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/video_render)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/media_sdk_demo)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/media_server_demo)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/socket_client)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example/socket_server)