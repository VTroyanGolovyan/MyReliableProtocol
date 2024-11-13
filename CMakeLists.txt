cmake_minimum_required(VERSION 3.20)

project(MyTCP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

find_package(Boost REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )

include_directories(helpers vhtcp vhtcp/sockets)

add_executable(Testing 
  testing.cpp 
  vhtcp/TcpPackageHeader.cpp 
  vhtcp/sockets/Socket.cpp 
  vhtcp/sockets/helpers/TimeoutedUdpSocket.cpp 
)
target_link_libraries(Testing ${Boost_LIBRARIES})
