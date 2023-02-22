cmake_minimum_required(VERSION 3.0)

project(storage-tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(thirdparty/googletest)

add_compile_definitions(USE_STDCXX_MUTEX)

add_executable(storage-tests
	PathTest.cpp
	TraceLockerTest.cpp
	VolumeTest.cpp
)

target_link_libraries(storage-tests 
	storage
	gtest_main
)
