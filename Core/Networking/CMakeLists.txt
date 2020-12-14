cmake_minimum_required(VERSION 3.14)

project(FastNetLib)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VS_STARTUP_PROJECT FastNet)


set(libs 
    ${CMAKE_SOURCE_DIR}/libs/
)

set(api
    include/FastNet.hxx
)

set(headers
    include/Socket.hxx
    include/Macros.hxx
    include/Defines.hxx
    include/Types.hxx
    include/Helpers.hxx
    include/Logger.hxx
    include/Client.hxx
    include/Server.hxx
    include/Peer.hxx
    include/Connection.hxx
)

source_group("API" FILES ${api})
source_group("Header Files" FILES ${headers})

set(src
   src/FastNet.cxx
   src/Client.cxx
   src/Server.cxx
   src/Peer.cxx
)

if(WIN32)
    set(src_platform
        src/Socket.cxx
    )
else()
    set(src_platform)
endif()


include_directories(${libs})

add_library(FastNet STATIC ${src} ${src_platform} ${api} ${headers})
add_executable(FastNetTest src/Main.cxx ${src} ${src_platform} ${api} ${headers})
