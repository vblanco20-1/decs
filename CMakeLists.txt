# CMakeList.txt : CMake project for decs, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(decs VERSION 0.1.0)


add_library(Decs INTERFACE)

target_include_directories(
    Decs INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(BUILD_TESTING "Enable testing with ctest." ON)


if(BUILD_TESTING)
	include(ExternalProject)
	include(FetchContent)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    option(FIND_GTEST_PACKAGE "Enable finding gtest package." OFF)

    if(FIND_GTEST_PACKAGE)
        find_package(GTest REQUIRED)
    else()
        # gtest, gtest_main, gmock and gmock_main targets are available from now on
        set(GOOGLETEST_DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/googletest)
       #configure_file(${Decs_SOURCE_DIR}/cmake/in/googletest.in ${GOOGLETEST_DEPS_DIR}/CMakeLists.txt)

		FetchContent_Declare(googletest
		#ExternalProject_Add(googletest
		  GIT_REPOSITORY    https://github.com/google/googletest.git
		  GIT_TAG           master
		  SOURCE_DIR        "${GOOGLETEST_DEPS_DIR}/src"
		  BINARY_DIR        "${GOOGLETEST_DEPS_DIR}/build"
		  CONFIGURE_COMMAND ""
		  BUILD_COMMAND     ""
		  INSTALL_COMMAND   ""
		  TEST_COMMAND      ""
		)
		FetchContent_GetProperties(googletest)
		if(NOT googletest_POPULATED)
		  FetchContent_Populate(googletest)
		  
		  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
		endif()
        add_library(GTest::Main ALIAS gtest_main)
    endif()
	
    enable_testing()
    add_subdirectory(test)
endif()