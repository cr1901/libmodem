set(SRCS XMODEM.C TRXCOM.C USR_DEF.C)
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
