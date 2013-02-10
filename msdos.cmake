#Most of this stuff (especially angle-bracket variables) is not documented.
#Source: http://stackoverflow.com/questions/13415222/variables-with-angle-brackets-in-cmake-scripts
#Reference: http://www.cmake.org/Wiki/CMake_Useful_Variables/Get_Variables_From_CMake_Dashboards

SET(CMAKE_SYSTEM_NAME MSDOS)

SET(CMAKE_C_COMPILER wcl)
SET(CMAKE_CXX_COMPILER wcl)

LIST(APPEND CMAKE_C_FLAGS_DEBUG_INIT "-DMODEM_DEBUG")
LIST(APPEND CMAKE_CXX_FLAGS_DEBUG_INIT "-DMODEM_DEBUG")
