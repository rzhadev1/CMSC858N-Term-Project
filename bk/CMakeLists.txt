add_executable(bk flow.cpp)

# include parlay
target_include_directories(bk PUBLIC ${parlaylib_SOURCE_DIR} ${parlaylib_SOURCE_DIR}/include/)
target_include_directories(bk PUBLIC ${PROJECT_SOURCE_DIR}/bk)
target_include_directories(bk PUBLIC ${PROJECT_SOURCE_DIR}/parser)
target_include_directories(bk PUBLIC ${Boost_INCLUDE_DIR})

# link functions
target_link_libraries(bk PUBLIC ${JEMALLOC_LIBS} "-pthread")
target_link_libraries(bk PUBLIC Boost::program_options)