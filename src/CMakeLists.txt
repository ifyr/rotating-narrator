file(GLOB_RECURSE SOURCES "*.cpp" "*.cxx" "*.cc")
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE FORMS *.ui)
file(GLOB_RECURSE RESOURCES "*.qrc" "*.rc")

include_directories(${CMAKE_SOURCE_DIR}/ffmpeg71/include)

set(SRCS ${SOURCES} ${HEADERS} ${RESOURCES} ${FORMS})
source_group("" FILES ${SRCS})

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} 
	Qt6::Widgets
	Qt6::Concurrent
	${CMAKE_SOURCE_DIR}/ffmpeg71/lib/avutil.lib 
	${CMAKE_SOURCE_DIR}/ffmpeg71/lib/avcodec.lib 
	${CMAKE_SOURCE_DIR}/ffmpeg71/lib/avformat.lib 
	${CMAKE_SOURCE_DIR}/ffmpeg71/lib/swscale.lib 
	${CMAKE_SOURCE_DIR}/ffmpeg71/lib/swresample.lib
)

