include_directories("${PROJECT_BINARY_DIR}")

include_directories(./ALProtocol_variants/ALProtocol1C/)
include_directories(./ALProtocol_variants/ALProtocol2C/)
include_directories(./ALProtocol_variants/ALProtocolTR/)

add_library(ALShared 
  #            ALProtocol_variants/ALProtocol1C/ALProtocol1C.c 
  #            ALProtocol_variants/ALProtocol1C/ALChunk1C.c 
  #            ALProtocol_variants/ALProtocol1C/ALPayloadChunk1C.c
            ALProtocol_variants/ALProtocolTR/ALProtocolTR.c 
            ALProtocol_variants/ALProtocolTR/ALChunkTR.c 
            ALProtocol_variants/ALProtocolTR/ALPayloadChunkTR.c
            TreeDecrypter.c
            libs/libb64/cdecode.c
            )
          
find_package(GMP REQUIRED)
if(GMP_FOUND)
        include_directories(${GMP_INCLUDE_DIR})
        target_link_libraries(ALShared ${GMP_LIBRARIES})
endif(GMP_FOUND)
target_link_libraries(ALShared OCBase -lm)

#add_executable(benchmarkALProto benchmark.c)
#target_link_libraries(benchmarkALProto ALShared ALCrypto OCBase)

enable_testing()
add_executable(test_ALProtocolTR  ALProtocol_variants/tests/test_ALProtocolTR.c)
find_package(Check REQUIRED)
if(CHECK_FOUND)
  include_directories(${CHECK_INCLUDE_DIR})
  target_link_libraries(test_ALProtocolTR ${CHECK_LIBRARY})
endif(CHECK_FOUND)
target_link_libraries(test_ALProtocolTR ALShared ALCrypto)

add_test(test_ALProtocolTR ${CMAKE_CURRENT_BINARY_DIR}/test_ALProtocolTR)

