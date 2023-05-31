# This file arbitrates between different methods of building.
cmake_minimum_required(VERSION 3.13)

if(DEFINED IDF_TARGET)
	# ESP-IDF detected (by IDF_TARGET being defined).
	message(STATUS "Building PAX Graphics as ESP-IDF module.")
	include(${CMAKE_CURRENT_LIST_DIR}/esp_idf.cmake)
elseif(DEFINED PICO_SDK_PATH)
	# Pi Pico SDK detected (by PICO_SDK_PATH being defined).
	project(pax_graphics C CXX)
	
	message(STATUS "Building PAX Graphics for Pi Pico SDK.")
	add_definitions(-DPAX_COMPILE_MCR=0)
	add_definitions(-DPAX_PI_PICO=1)
	include(${CMAKE_CURRENT_LIST_DIR}/standalone.cmake)
elseif(DEFINED ENV{PAX_CMAKE_STANDALONE})
	# Explicitly named SDK.
	set(CMAKE_C_COMPILER_FORCED true)
	set(CMAKE_CXX_COMPILER_FORCED true)
	
	project(pax_graphics C CXX)
	add_definitions(-DPAX_STANDALONE=1)
	add_definitions(-DPAX_COMPILE_MCR=0)
	
	message(STATUS "Building PAX Graphics for " $ENV{PAX_CMAKE_STANDALONE} ".")
	include(${CMAKE_CURRENT_LIST_DIR}/standalone.cmake)
else()
	# Unknown SDK.
	set(CMAKE_C_COMPILER_FORCED true)
	set(CMAKE_CXX_COMPILER_FORCED true)

	project(pax_graphics C CXX)
	add_definitions(-DPAX_STANDALONE=1)
	add_definitions(-DPAX_COMPILE_MCR=0)

	message(STATUS "Building PAX Graphics (unknown SDK).")
	include(${CMAKE_CURRENT_LIST_DIR}/standalone.cmake)
endif()
