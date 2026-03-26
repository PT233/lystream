

add_executable(stream_data_deque_test stream_data_deque_test.cpp)
add_executable(stream_3pdata_deque_test stream_3pdata_deque_test.cpp)

target_link_libraries(stream_data_deque_test lystream pthread)
target_link_libraries(stream_3pdata_deque_test lystream pthread)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    install(TARGETS stream_data_deque_test RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/utils/mem/)
    install(TARGETS stream_3pdata_deque_test RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/utils/mem/)

endif ()