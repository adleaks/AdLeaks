include_directories("${PROJECT_BINARY_DIR}")

add_executable( Tier
                Tier.c
                Tier2.c
                Tier3.c
                TierLoadTest.c
                TierMessage.c
                TierMessage_evbuffer.c
                TierClient.c
                TierClientConnection.c
                TierConfig.c
                TierServer.c
                TierServerConnection.c
                AggregationQueue.c
                DecryptionQueue.c
                Tier3Storage.c
                )      

add_executable( Tier1
                Tier1.c 
                TierMessage.c
                TierMessage_evbuffer.c
                TierClient.c
                TierClientConnection.c
                TierConfig.c
                )

find_package(EVENT REQUIRED)
if(EVENT_FOUND)
  include_directories(${EVENT_INCLUDE_DIR})
  target_link_libraries(Tier ${EVENT_LIBRARIES})
  target_link_libraries(Tier1 ${EVENT_LIBRARIES})
endif(EVENT_FOUND)

find_package(GMP REQUIRED)
if(GMP_FOUND)
        include_directories(${GMP_INCLUDE_DIR})
	target_link_libraries(Tier1 ${GMP_LIBRARIES})
        target_link_libraries(Tier ${GMP_LIBRARIES})
endif(GMP_FOUND)

find_package(FCGI REQUIRED)
if(FCGI_FOUND)
	include_directories(${FCGI_INCLUDE_DIRS})
	target_link_libraries(Tier1 ${FCGI_LIBRARY})
endif(FCGI_FOUND)

include_directories(${OCBASE_INCLUDE_DIR})
include_directories(${ALSHARED_INCLUDE_DIR})
include_directories(${ALCRYPTO_INCLUDE_DIR})

include_directories("${PROJECT_SOURCE_DIR}/ALShared/ALProtocol_variants/ALProtocol1C")
include_directories("${PROJECT_SOURCE_DIR}/ALShared/ALProtocol_variants/ALProtocol2C")
include_directories("${PROJECT_SOURCE_DIR}/ALShared/ALProtocol_variants/ALProtocolTR")

target_link_libraries(Tier OCBase ALShared ALCrypto -lpthread)
target_link_libraries(Tier1 OCBase ALShared ALCrypto -lpthread -lm)


