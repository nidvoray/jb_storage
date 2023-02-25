cmake_minimum_required(VERSION 3.0)

project(storage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(USE_STDCXX_MUTEX)

add_library(storage
	source/BaseImpl.cpp
	source/Path.cpp
	source/Serialization.cpp
	source/Storage.cpp
	source/Volume.cpp
	source/VolumeImpl.cpp
)

target_include_directories(storage 
	PUBLIC include
)
