cmake_minimum_required (VERSION 2.6)

#set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/Modules)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

project (MODEM)

#Add ANSI-C Checker
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "-Wall -Wextra -std=c99 -pedantic-errors -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition")
endif(CMAKE_COMPILER_IS_GNUCC)

add_subdirectory(src)

