#Most of this stuff (especially angle-bracket variables) is not documented.
#Source: http://stackoverflow.com/questions/13415222/variables-with-angle-brackets-in-cmake-scripts
#Reference: http://www.cmake.org/Wiki/CMake_Useful_Variables/Get_Variables_From_CMake_Dashboards

INCLUDE(CMakeForceCompiler)

SET(CMAKE_SYSTEM_NAME MSDOS)

CMAKE_FORCE_C_COMPILER(wcl Watcom)
CMAKE_FORCE_CXX_COMPILER(wcl Watcom)

LIST(APPEND CMAKE_C_FLAGS "-D__DOS__")
LIST(APPEND CMAKE_CXX_FLAGS "-D__DOS__")
