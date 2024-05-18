add_library(parser parser.cpp)
target_include_directories(parser PRIVATE ${parlaylib_SOURCE_DIR} ${parlaylib_SOURCE_DIR}/include/)
target_link_libraries(parser PUBLIC ${JEMALLOC_LIBS} "-pthread")
