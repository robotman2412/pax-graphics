# This file arbitrates between different methods of building.
cmake_minimum_required(VERSION 3.13)

if(DEFINED ENV{PAX_CMAKE_STANDALONE})
	# Don't bother testing the compilers.
	set(CMAKE_C_COMPILER_FORCED true)
	set(CMAKE_CXX_COMPILER_FORCED true)
	
	project(pax_graphics C CXX)
	
	message(STATUS "Building PAX Graphics for " $ENV{PAX_CMAKE_STANDALONE} ".")
	include(${CMAKE_CURRENT_LIST_DIR}/Standalone.cmake)
elseif(DEFINED PICO_SDK_PATH)
	# Pi Pico SDK detected (by PICO_SDK_PATH being defined).
	project(pax_graphics C CXX)
	
	message(STATUS "Building PAX Graphics for Pi Pico SDK.")
	add_definitions(-DPAX_COMPILE_MCR=0)
	add_definitions(-DPAX_PI_PICO=1)
	include(${CMAKE_CURRENT_LIST_DIR}/Standalone.cmake)
else()
	# By default ESP-IDF
	# TODO: Add proper detection for this.
	message(STATUS "Building PAX Graphics as ESP-IDF module (by default).")
	include(${CMAKE_CURRENT_LIST_DIR}/ESP_IDF.cmake)
endif()
