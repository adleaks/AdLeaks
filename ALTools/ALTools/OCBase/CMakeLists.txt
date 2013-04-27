include_directories("${PROJECT_BINARY_DIR}")
add_library(
	OCBase
	OCAllocator.c 
	OCBase.c 
	OCBufferedStreamReader.c
        OCConfig.c
        OCDictionary.c
	OCList.c
	OCLog.c 
	OCObject.c 
	OCSerializer.c
	OCString.c
	OCTime.c 
	OCTypes.c
        OCThread.c
        OCValue.c
        libs/inih/ini.c
)
link_libraries(OCBase -lpthread)

enable_testing()
add_subdirectory(tests)

