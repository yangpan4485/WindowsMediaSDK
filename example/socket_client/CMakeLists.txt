set(DEMO_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/socket_client.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/socket_client.h
    )
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${DEMO_SOURCE})
# file_source_group(${DEMO_SOURCE})

add_executable(socket_client_demo ${DEMO_SOURCE})

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT socket_client_demo)