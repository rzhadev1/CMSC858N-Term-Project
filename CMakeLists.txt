cmake_minimum_required(VERSION 3.14)
project(par-max-flow)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_program(JEMALLOC_FOUND jemalloc-config)
if(JEMALLOC_FOUND)
	execute_process(
		COMMAND jemalloc-config --libs
		OUTPUT_VARIABLE JEMALLOC_LIBS
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(JEMALLOC_LIBS "${JEMALLOC_LIBS} -ljemalloc")
endif()

include(FetchContent)
FetchContent_Declare(
	parlaylib
	GIT_REPOSITORY https://github.com/cmuparlay/parlaylib.git
	GIT_TAG master
)

FetchContent_MakeAvailable(parlaylib)

# download the dataset
if(NOT EXISTS "${CMAKE_BINARY_DIR}/BL06-camel-sml.max")

	set(CAMEL_SML_FILE "${CMAKE_BINARY_DIR}/BL06-camel-sml.tbz2")
	message(STATUS "Downloading camel-small graph data...")

	file(DOWNLOAD https://vision.cs.uwaterloo.ca/files/BL06-camel-sml.tbz2 ${CAMEL_SML_FILE}
			SHOW_PROGRESS
			STATUS download_status
			LOG download_log)
	list(GET download_status 0 status_code)
	if(NOT status_code EQUAL 0)
		message(FATAL_ERROR "Error downloading graph data: ${download_log}")
	endif()	

	message(STATUS "Extracting graph data...")

	execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xvjf ${CAMEL_SML_FILE}
			RESULT_VARIABLE tar_result
			OUTPUT_VARIABLE tar_output
			ERROR_VARIABLE tar_error)
	if(NOT tar_result EQUAL 0)
		message(FATAL_ERROR "Error extracting graph data: ${tar_error}")
	endif()

file(REMOVE ${CAMEL_SML_FILE})

endif()

find_package(Boost 1.82 COMPONENTS program_options REQUIRED)
add_subdirectory(src)
add_subdirectory(parser)
