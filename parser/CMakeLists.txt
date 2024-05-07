add_executable(parser dimacsToCSR.cpp)
target_include_directories(parser PRIVATE ${parlaylib_SOURCE_DIR} ${parlaylib_SOURCE_DIR}/include/)
target_include_directories(parser PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(parser PRIVATE ${JEMALLOC_LIBS} "-pthread")
