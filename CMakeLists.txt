cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(hello LANGUAGES CXX)
set(HELLO_MAJOR_VERSION 0)
set(HELLO_MINOR_VERSION 1)
SET(HELLO_VERSION ${HELLO_MINOR_VERSION}.${HELLO_MAJOR_VERSION})

message(STATUS "is the C++ compiler loaded? ${CMAKE_CXX_COMPILER_LOADED}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(PkgConfig REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(hello ${CMAKE_SOURCE_DIR}/hello/src/main.cpp 
  ${CMAKE_SOURCE_DIR}/hello/src/logging.cpp)

add_executable(shader ${CMAKE_SOURCE_DIR}/shader/src/main.cpp 
  ${CMAKE_SOURCE_DIR}/shader/src/logging.cpp)

set(LINK_LIBS ${OPENGL_gl_LIBRARY} ${GLEW_SHARED_LIBRARIES} GLEW glfw)
target_link_libraries(hello ${LINK_LIBS})
target_link_libraries(shader ${LINK_LIBS})


			  
