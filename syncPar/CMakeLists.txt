add_executable(syncPar flow.cpp)
target_include_directories(syncPar PRIVATE ${parlaylib_SOURCE_DIR} ${parlaylib_SOURCE_DIR}/include/)
target_include_directories(syncPar PRIVATE ${PROJECT_SOURCE_DIR}/syncPar)
target_include_directories(syncPar PRIVATE ${PROJECT_SOURCE_DIR}/parser)

target_link_libraries(syncPar PRIVATE ${JEMALLOC_LIBS} "-pthread")
