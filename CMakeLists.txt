cmake_minimum_required(VERSION 3.10)
project(forestfire)
add_subdirectory(win)
set(CMAKE_CXX_STANDARD 17)

set(executable ForestFire)

set(sources
	src/main.cpp

	src/Renderer.hpp
	src/Renderer.cpp
)

if (ROLLDATA)
	file(REAL_PATH ${ROLLDATA} path BASE_DIRECTORY ${CMAKE_BINARY_DIR})
	add_compile_definitions(ROLLDATA=\"${path}\")
endif ()

#if (WIN32)
#	list(APPEND sources icon.rc)
#endif ()

if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
	add_executable(${executable} WIN32 ${sources})
else ()
	add_executable(${executable} ${sources})
endif ()

target_link_libraries(${executable} win)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(${executable} PRIVATE -fno-exceptions)
elseif (WIN32)
	# /w3 is too aggressive
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W1" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	endif ()
endif ()

if (USE_ASAN)
	message("Enabling ASAN")

	target_compile_options(${executable} PRIVATE -fsanitize=address)
	target_link_options(${executable} PRIVATE -fsanitize=address)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(${executable} PRIVATE -fsanitize=undefined)
		target_link_options(${executable} PRIVATE -fsanitize=undefined)
	endif ()
endif ()
