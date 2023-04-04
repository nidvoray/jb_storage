cmake_minimum_required(VERSION 3.0)

project(storage-tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(gtest_force_shared_crt on)
add_subdirectory(thirdparty/googletest)

add_compile_definitions(USE_STDCXX_MUTEX)

add_executable(storage-tests
	PathViewTest.cpp
	TraceLockerTest.cpp
	SerializationTest.cpp
	VolumeTest.cpp
	StorageTest.cpp
	SaveLoadTest.cpp
	StabilityTest.cpp
	TestSet.cpp
)

target_link_libraries(storage-tests 
	storage
	gtest_main
)
