add_executable(parse_video_stream_para parse_video_stream_para.cpp)
add_executable(parse_jpeg_stream_para parse_jpeg_stream_para.cpp)
add_executable(parse_camera_stream_para parse_camera_stream_para.cpp)

target_link_libraries(parse_video_stream_para lystream ${YAML_CPP_LIBRARIES})
target_link_libraries(parse_jpeg_stream_para lystream ${YAML_CPP_LIBRARIES})
target_link_libraries(parse_camera_stream_para lystream ${YAML_CPP_LIBRARIES})

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    install(TARGETS parse_video_stream_para RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/utils/parse/)
    install(TARGETS parse_jpeg_stream_para RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/utils/parse/)
    install(TARGETS parse_camera_stream_para RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/utils/parse/)
endif ()