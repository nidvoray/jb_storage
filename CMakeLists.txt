cmake_minimum_required(VERSION 3.0)

project(jb-storage)

add_subdirectory(storage)
add_subdirectory(tests)

target_include_directories(storage-tests
	PRIVATE storage/include
	PRIVATE storage/source
)

set_target_properties(storage storage-tests PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
