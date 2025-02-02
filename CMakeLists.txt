cmake_minimum_required(VERSION 3.10)

project(MB DESCRIPTION "MB Slave Source C++ implementation of the Modbus slave protocol" VERSION 1.0.0)

# Definitions for Version.h file
add_definitions(-DLIBMB_MAJOR="${PROJECT_VERSION_MAJOR}")
add_definitions(-DLIBMB_MINOR="${PROJECT_VERSION_MINOR}")
add_definitions(-DLIBMB_FIX="${PROJECT_VERSION_PATCH}")

set(CMAKE_C_STANDARD 11)               
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(3rdparty)
add_subdirectory(src)

add_executable(use_mb_slave src/example/use_mb/main.cpp)

add_library(mb_slave-static STATIC src/version.cpp)
add_library(mb_slave-shared SHARED src/version.cpp)

# Линковка с библиотекой modbus
target_link_libraries(use_mb_slave mb_slave-static)

target_link_libraries(mb_slave-static 
	mb_slave
	action_manager
	data_manager  
	helpers 
	version
	modbus-static 
)

target_link_libraries(mb_slave-shared 
	mb_slave
	action_manager 
	data_manager
	helpers 
	version
	modbus-static 
)


file(GLOB_RECURSE PUBLIC_HEADERS
     LIST_DIRECTORIES false RELATIVE ${CMAKE_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/src/*.h")
install(FILES ${PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mb)
