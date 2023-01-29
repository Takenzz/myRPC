cmake_minimum_required(VERSION 3.23)
project(RPC)

add_definitions(-DMSGPACK_NO_BOOST)
add_compile_definitions(ASIO_STANDALONE)
set(MSGPACK_DIR msgpack)
set(ASIO_DIR asio)
set(CMAKE_CXX_STANDARD 17)
include_directories(${MSGPACK_DIR})
include_directories(${ASIO_DIR})

link_libraries(ws2_32 wsock32)


add_executable(RPC main.cpp codec.h register.cpp register.h connection.cpp connection.h rpc_server.cpp rpc_server.h rpc_client.cpp rpc_client.h)
