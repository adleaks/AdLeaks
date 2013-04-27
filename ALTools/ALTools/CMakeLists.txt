cmake_minimum_required(VERSION 2.6)
project(ALTools)

ADD_DEFINITIONS(-g -O2 -fsigned-char -freg-struct-return
	-Wall -W -Wshadow -Wstrict-prototypes -Wpointer-arith
	-Wcast-qual -Winline -Werror -Wno-unknown-pragmas)

set(ALTools_VERSION_MAJOR 0)
set(ALTools_VERSION_MINOR 1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

configure_file(
  "${PROJECT_SOURCE_DIR}/ALToolsConfig.h.in"
  "${PROJECT_BINARY_DIR}/ALToolsConfig.h"
)

include_directories("${PROJECT_SOURCE_DIR}/OCBase")
add_subdirectory(OCBase)

include_directories("${PROJECT_SOURCE_DIR}/ALCrypto")
add_subdirectory(ALCrypto)

include_directories("${PROJECT_SOURCE_DIR}/ALShared")
add_subdirectory(ALShared)

include_directories("${PROJECT_SOURCE_DIR}/Tiers")
add_subdirectory(Tiers)


include_directories("${PROJECT_BINARY_DIR}")

include_directories("${PROJECT_SOURCE_DIR}/ALShared/ALProtocol_variants/ALProtocol1C")
include_directories("${PROJECT_SOURCE_DIR}/ALShared/ALProtocol_variants/ALProtocolTR")

add_executable(ALTools ALTools.c ALShared/libs/libb64/cdecode.c)
find_package(GMP REQUIRED)
if(GMP_FOUND)
        include_directories(${GMP_INCLUDE_DIR})
        target_link_libraries(ALTools ${GMP_LIBRARIES})
endif(GMP_FOUND)


target_link_libraries(ALTools OCBase ALShared ALCrypto -lm)

enable_testing()
