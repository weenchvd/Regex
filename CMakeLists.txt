#
#
cmake_minimum_required(VERSION 3.14)

project("RegexEngine")

option(REGEXENGINE_ENABLE_TEST "Enable tests for RegexEngine" ON)



set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Install")

add_subdirectory("Source")

if(REGEXENGINE_ENABLE_TEST)
	include(FetchContent)
	FetchContent_Declare(
		googletest
		GIT_REPOSITORY	https://github.com/google/googletest.git
		GIT_TAG			release-1.10.0
		SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/External/googletest/src
	)

	# Prevent overriding the parent project's compiler/linker
	# settings on Windows
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

	FetchContent_MakeAvailable(googletest)

	enable_testing()

	add_subdirectory("Test")
endif()
