#Support for MSDOS- Adapted from eCos.cmake
#Most of this stuff (especially angle-bracket variables) is not documented.
#Source: http://stackoverflow.com/questions/13415222/variables-with-angle-brackets-in-cmake-scripts
#Reference: http://www.cmake.org/Wiki/CMake_Useful_Variables/Get_Variables_From_CMake_Dashboards
#C:\Program Files (x86)\CMake 2.8\share\cmake-2.8\Modules\Platform\Windows-wcl386.cmake also is a good refernce

# Guard against multiple inclusion, which e.g. leads to multiple calls to add_definition() #12987
IF(__MSDOS_CMAKE_INCLUDED)
  RETURN()
ENDIF()
SET(__MSDOS_CMAKE_INCLUDED TRUE)

SET(CMAKE_LIBRARY_PATH_FLAG "libpath ")
SET(CMAKE_LINK_LIBRARY_FLAG "library ")
SET(CMAKE_LINK_LIBRARY_FILE_FLAG "library")

IF(CMAKE_VERBOSE_MAKEFILE)
  SET(CMAKE_WCL_QUIET)
  SET(CMAKE_WLINK_QUIET)
  SET(CMAKE_LIB_QUIET)
ELSE(CMAKE_VERBOSE_MAKEFILE)
  SET(CMAKE_WCL_QUIET "-zq")
  SET(CMAKE_WLINK_QUIET "option quiet")
  SET(CMAKE_LIB_QUIET "-q")
ENDIF(CMAKE_VERBOSE_MAKEFILE)

SET (CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "debug all" )
#SET (CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT "debug all" )
SET (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "debug all" )

#No shared Libraries on MSDOS
#SET(CMAKE_SHARED_LIBRARY_C_FLAGS "")            # -pic
#SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "")       # -shared
#SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")         # +s, flag for exe link to use shared lib
#SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "")       # -rpath
#SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP "")   # : or empty

SET(CMAKE_LINK_LIBRARY_SUFFIX "")
SET(CMAKE_STATIC_LIBRARY_PREFIX "")
SET(CMAKE_STATIC_LIBRARY_SUFFIX ".LIB")
SET(CMAKE_SHARED_LIBRARY_PREFIX "")
SET(CMAKE_SHARED_LIBRARY_SUFFIX "")
SET(CMAKE_EXECUTABLE_SUFFIX ".EXE")
SET(CMAKE_DL_LIBS "" )


SET(CMAKE_FIND_LIBRARY_PREFIXES "")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".LIB")

GET_PROPERTY(_IN_TC GLOBAL PROPERTY IN_TRY_COMPILE)
IF(CMAKE_C_COMPILER AND NOT  "${CMAKE_C_COMPILER_ID}" MATCHES "Watcom" AND NOT _IN_TC)
  MESSAGE(FATAL_ERROR "OpenWatcom is currently required for MSDOS")
ENDIF(CMAKE_C_COMPILER AND NOT  "${CMAKE_C_COMPILER_ID}" MATCHES "Watcom" AND NOT _IN_TC)
IF(CMAKE_CXX_COMPILER AND NOT  "${CMAKE_CXX_COMPILER_ID}" MATCHES "Watcom" AND NOT _IN_TC)
  MESSAGE(FATAL_ERROR "OpenWatcom is currently required for MSDOS")
ENDIF(CMAKE_CXX_COMPILER AND NOT  "${CMAKE_CXX_COMPILER_ID}" MATCHES "Watcom" AND NOT _IN_TC)


#Add IF("${CMAKE_C_COMPILER_ID}" MATCHES "WATCOM") later...
SET(CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER> ${CMAKE_WCL_QUIET} <FLAGS> -fo=<OBJECT> -c <SOURCE>")
SET(CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> ${CMAKE_WCL_QUIET} <FLAGS> -fo=<OBJECT> -c <SOURCE>")

SET(CMAKE_C_LINK_EXECUTABLE
    "wlink ${CMAKE_START_TEMP_FILE} ${CMAKE_WLINK_QUIET} system dos name '<TARGET_UNQUOTED>' <LINK_FLAGS> option caseexact file {<OBJECTS>} <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")
SET(CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_C_LINK_EXECUTABLE})

#By default, assume all MSDOS applications are put in separate directories under a single root
SET(CMAKE_SYSTEM_INCLUDE_PATH "${CMAKE_INSTALL_PREFIX}/..")
SET(CMAKE_SYSTEM_LIBRARY_PATH "${CMAKE_INSTALL_PREFIX}/..")
SET(CMAKE_SYSTEM_PROGRAM_PATH "${CMAKE_INSTALL_PREFIX}/..")




#Ignore below variables- scratch space.

#SET(CMAKE_C_LINK_EXECUTABLE  "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -L<LINK_LIBRARIES> -fe=<TARGET>") 
#SET(CMAKE_CXX_LINK_EXECUTABLE  "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -L<LINK_LIBRARIES> -fe=<TARGET>") 
#-nostdlib -nostartfiles -L${ECOS_LIBTARGET_DIRECTORY} -Ttarget.ld  <LINK_LIBRARIES>")
#SET(CMAKE_C_LINK_EXECUTABLE    "<CMAKE_C_COMPILER>   <FLAGS> <CMAKE_C_LINK_FLAGS>   <LINK_FLAGS> <OBJECTS> -fo=<TARGET>") 
#-nostdlib -nostartfiles -L${ECOS_LIBTARGET_DIRECTORY} -Ttarget.ld  <LINK_LIBRARIES>")
