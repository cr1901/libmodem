#http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries#Using_external_libraries_that_CMake_doesn.27t_yet_have_modules_for

IF(NOT CMAKE_TOOLCHAIN_FILE MATCHES "msdos.cmake")
	SET(PICTOR_DEPENDS_SATISFIED 1)
	MESSAGE("PICTOR Libs not needed")
	RETURN()
ENDIF()

Message("Looking for: " ${CMAKE_BINARY_DIR}/../PICTOR)
find_path(PICTOR_INCLUDE_DIR PICTOR.H "${CMAKE_BINARY_DIR}/../PICTOR")
Message(${PICTOR_INCLUDE_DIR})

Message("Looking for: " ${CMAKE_BINARY_DIR}/../PICTOR)
find_library(PICTOR_LIBRARY NAMES PICTORSW.LIB HINTS "${CMAKE_BINARY_DIR}/../PICTOR")
Message(${PICTOR_LIBRARY})

set(PICTOR_LIBRARIES ${PICTOR_LIBRARY} )
set(PICTOR_INCLUDE_DIRS ${PICTOR_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
#Handle QUIETLY and REQUIRED, and take care of variables
find_package_handle_standard_args(PICTOR  DEFAULT_MSG PICTOR_LIBRARY PICTOR_INCLUDE_DIR)
