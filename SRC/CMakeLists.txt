set(SRCS XMODEM.c TRXCOM.c USR_DEF.c)
add_executable(TRXCOM  ${SRCS})

#CMake thinks these are CXX files for some reason
SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES LANGUAGE C)

#Will return if not necessary, error if necessary and not found.
find_package(PICTOR REQUIRED)
if (PICTOR_FOUND)
  Message("Libraries found here: " ${PICTOR_LIBRARIES})
  include_directories(${PICTOR_INCLUDE_DIRS})
  target_link_libraries (TRXCOM ${PICTOR_LIBRARIES})
endif (PICTOR_FOUND)

if(WIN32)
	#include_directories(${CMAKE_SOURCE_DIR}/src)
	#set(SRCS ${SRCS} trxdlg.rc trxwin.c)
	#add_executable(TRXWIN WIN32 ${SRCS})
endif(WIN32)
