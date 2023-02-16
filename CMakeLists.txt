# This file arbitrates between different methods of building.
cmake_minimum_required(VERSION 3.13)

if(DEFINED ENV{PAX_CMAKE_STANDALONE})
	# Don't bother testing the compilers.
	set(CMAKE_C_COMPILER_FORCED true)
	set(CMAKE_CXX_COMPILER_FORCED true)
	
	project(pax_graphics)
	
	message(STATUS "Building PAX Graphics for " $ENV{PAX_CMAKE_STANDALONE} ".")
	include(Standalone.cmake)
else()
	message(STATUS "Building PAX Graphics as ESP-IDF module (by default).")
	include(ESP_IDF.cmake)
endif()
