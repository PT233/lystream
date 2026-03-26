cmake_minimum_required(VERSION 3.10)  # 必须

project(lystream)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -Wno-unused-function")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -DNDEBUG -Wno-unused-function -Wno-unused-result")

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        "./utils"
        "/usr/local/cuda/targets/aarch64-linux/include"
        "/opt/lylive/include"
        "/opt/lyboost/include"
        "/opt/lynppi/include"
        "/usr/src/jetson_multimedia_api/include"
        "/opt/lycodec/include"
        "/opt/lycamera/include"
        "/opt/yaml-cpp/include"
        "/usr/src/jetson_multimedia_api/include"
        "/usr/local/cuda-10.2/targets/aarch64-linux/include"
        "/usr/local/cuda-10.2/samples/common/inc"
        "/opt/ffmpeg312/include"
)

link_directories(
        "/usr/local/cuda/lib64"
        "/usr/lib/aarch64-linux-gnu/"
        "/usr/lib/aarch64-linux-gnu/tegra"
        "/lib/aarch64-linux-gnu/"
        "/opt/opencv454/lib"
        "/opt/lynppi/lib/"
        "/opt/lycodec/lib/"
        "/opt/lyboost/lib/"
        "/opt/lylive/lib"
        "/opt/lycamera/lib"
        "/opt/yaml-cpp/lib/"
        "/opt/ffmpeg312/lib"
)


if (${DaHeng})
    add_definitions(-DDaHeng)
    include_directories("/opt/Galaxy_camera/inc/")
    link_directories("/opt/Galaxy_camera/inc/")
endif ()

if (${HIK})
    add_definitions(-DHIK)
    include_directories("/opt/MVS/include")
    link_directories("/opt/MVS/lib/aarch64/")
endif ()

if (${IKapC})
    add_definitions(-DIKapC)
    include_directories("/opt/Itek/IKapC/Include")
    link_directories("/opt/Itek/IKapC/Drivers/lib64")
endif ()

if (${IRay})
    add_definitions(-DIRay)
    link_directories("/opt/iraytek_sdk/libs")
endif ()

if (${MAGNITY})
    add_definitions(-DMAGNITY)
    include_directories("/opt/magnity")
endif ()

if (${DALSA})
    add_definitions(-DDALSA)
    link_directories("/opt/DALSA/GigeV/lib")
endif ()

if (${Pylon})
    add_definitions(-DPylon)
    link_directories("/opt/pylon/lib")
endif ()


set(CMAKE_PREFIX_PATH /opt/opencv454)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif ()
set(Boost_ROOT "/opt/boost_1_80_0")
find_package(Boost REQUIRED CONFIG COMPONENTS
        system
        log
        log_setup
        filesystem
        timer
        test_exec_monitor
)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "boost not found")
endif ()

set(CMAKE_PREFIX_PATH "/opt/yaml-cpp")
find_package(yaml-cpp REQUIRED)
include_directories(${YAML_INCLUDE_DIRS})

set(CMAKE_INSTALL_PREFIX "/opt/lystream")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib/")
set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include/")
set(CMAKE_INSTALL_CONFIG "${CMAKE_INSTALL_PREFIX}/config/")
set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin/")

set(ffmpeg_libs_DIR "/opt/ffmpeg312/lib")
add_library(avcodec SHARED IMPORTED)
#指定所添加依赖库的导入路径
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavcodec.so)
add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavfilter.so)
add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libswresample.so)
add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libswscale.so)
add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavformat.so)
add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavutil.so)


aux_source_directory(./ DIR_LIB_SRCS)
aux_source_directory(./handle/image DIR_LIB_SRCS)
aux_source_directory(./handle/video DIR_LIB_SRCS)
aux_source_directory(./algorithm DIR_LIB_SRCS)
aux_source_directory(./utils/mem/ DIR_LIB_SRCS)
aux_source_directory(./utils/system/ DIR_LIB_SRCS)
aux_source_directory(./utils/parse/ DIR_LIB_SRCS)
aux_source_directory(./utils/ffmpeg/ DIR_LIB_SRCS)

add_library(${PROJECT_NAME} SHARED ${DIR_LIB_SRCS})
add_library(${PROJECT_NAME}_static STATIC ${DIR_LIB_SRCS})
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 1.4 SOVERSION 1)
target_link_libraries(${PROJECT_NAME} lycamera utils_log lynppi lycodec ${YAML_CPP_LIBRARIES} ${Boost_LIBRARIES} stdc++fs atomic avformat avcodec avutil camera_ikapc camera_pylon camera_hik)

add_subdirectory(utils)
add_subdirectory(sample)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_subdirectory(config)
    add_subdirectory(handle/image)
    add_subdirectory(handle/video)
    add_subdirectory(algorithm)

    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    file(GLOB CURRENT_HEADER LIST_DIRECT false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
    install(FILES ${CURRENT_HEADER} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_DEBIAN_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION "1.0.24")
    string(TIMESTAMP COMPILE_TIME %Y%m%d)
    set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${COMPILE_TIME}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
    set(CPACK_PACKAGE_DIRECTORY "${PROJECT_BINARY_DIR}/deb")
    set(CPACK_PACKAGE_VENDOR "CMAKE")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multimedia stream architecture library")
    set(CPACK_PACKAGE_CONTACT "jianwei.yao@outlook.com")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "laoyao")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "Optional")
    include(CPack)
endif ()
