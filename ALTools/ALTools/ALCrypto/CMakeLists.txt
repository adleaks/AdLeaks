include_directories("${PROJECT_BINARY_DIR}")

add_library(ALCrypto CryptoRandom.c DJ.c CryptoMac.c libs/sha2/sha2.c libs/sha2/hmac_sha2.c libs/CRT/crt.c libs/damgard_jurik/damgard_jurik.c)
find_package(GMP REQUIRED)
if(GMP_FOUND)
	include_directories(${GMP_INCLUDE_DIR})
	target_link_libraries(ALCrypto ${GMP_LIBRARIES})
endif(GMP_FOUND)
include_directories(${OCBASE_INCLUDE_DIR})
include_directories(./libs/sha2/)
include_directories(./libs/damgard_jurik/)
include_directories(./libs/CRT/)
target_link_libraries(ALCrypto OCBase)

add_executable(benchmark benchmark.c)
target_link_libraries(benchmark ALCrypto)

