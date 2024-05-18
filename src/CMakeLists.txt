add_executable(syncPar syncPar/driver.cpp syncPar/flow.cpp)
target_include_directories(syncPar PRIVATE ${parlaylib_SOURCE_DIR} ${parlaylib_SOURCE_DIR}/include/)
target_include_directories(syncPar PRIVATE ${PROJECT_SOURCE_DIR}/include/syncPar)
target_include_directories(syncPar PRIVATE ${PROJECT_SOURCE_DIR}/parser)
target_link_libraries(syncPar PRIVATE ${JEMALLOC_LIBS} "-pthread")
target_link_libraries(syncPar PRIVATE parser)


add_executable(bk bk/driver.cpp)
target_include_directories(bk PRIVATE ${parlaylib_SOURCE_DIR} ${parlaylib_SOURCE_DIR}/include/)
target_include_directories(bk PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(bk PUBLIC ${JEMALLOC_LIBS} "-pthread")
target_link_libraries(bk PUBLIC Boost::program_options)


