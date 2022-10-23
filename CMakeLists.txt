cmake_minimum_required(VERSION 3.20)

project(hanzi-to-pinyin
        VERSION 2.0.0
        LANGUAGES CXX)

####################
# config for cmake #
####################

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ Standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "C++ Standard Requirement")
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "C++ Extensions")

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTING "Build Tests" OFF)

set(CONFIGURED_FILES_ROOT ${PROJECT_BINARY_DIR}/configured_files)

include(GNUInstallDirs)

##############
# dependency #
##############

# boost
find_package(Boost 1.70.0 REQUIRED COMPONENTS locale)

# curl
find_package(CURL REQUIRED)

# openssl
find_package(OpenSSL REQUIRED)

# protobuf
find_package(Protobuf REQUIRED)

# SQLite3
find_package(SQLite3 REQUIRED)

# RapidJSON
find_package(RapidJSON REQUIRED)

# BasicToolBox
find_package(BasicToolBox CONFIG REQUIRED)  

# cppjieba
find_package(cppjieba REQUIRED)

##################
# config for lib #
##################

set(SQLITE_FILE_PATH ${PROJECT_SOURCE_DIR}/database/pinyin.sqlite3)
configure_file(config/config.json.in
               ${CONFIGURED_FILES_ROOT}/config/config.json
               @ONLY)

set(DEFAULT_CONFIG_FILE_PATH ${CONFIGURED_FILES_ROOT}/config/config.json)
configure_file(source/${PROJECT_NAME}/config/config.h.in
               ${CONFIGURED_FILES_ROOT}/source/${PROJECT_NAME}/config/config.h
               @ONLY)

##########
# target #
##########

set(CPP_FILES
    source/${PROJECT_NAME}/config/config.cpp
    source/${PROJECT_NAME}/config/db_config/db_config.cpp
    source/${PROJECT_NAME}/hanzi_to_pinyin.cpp
    source/${PROJECT_NAME}/pinyin_db/pinyin_db.cpp)

add_library(${PROJECT_NAME} ${CPP_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# export macro
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
                       EXPORT_FILE_NAME source/${PROJECT_NAME}/export/${PROJECT_NAME}_export.h)
set(EXPORT_FILE_PATH ${PROJECT_BINARY_DIR}/source/${PROJECT_NAME}/export/${PROJECT_NAME}_export.h)

target_include_directories(${PROJECT_NAME}
                           PUBLIC # use PUBLIC instead of PRIVATE because tests use it
                               $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>
                               $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/source>
                               $<BUILD_INTERFACE:${CONFIGURED_FILES_ROOT}/source>
                           INTERFACE
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_FULL_INCLUDEDIR}>)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/source/${PROJECT_NAME}/pch/pch.h)

#target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::locale)

target_include_directories(${PROJECT_NAME} PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SQLite3_LIBRARIES})

target_include_directories(${PROJECT_NAME} PRIVATE ${RapidJSON_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC BasicToolBox::BasicToolBox)

target_link_libraries(${PROJECT_NAME} PUBLIC cppjieba::cppjieba)

########
# test #
########

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(BUILD_TESTING)
        include(CTest)
        enable_testing()
        include(test/test.cmake)
    endif()
endif()

###########
# install #
###########

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME})

install(EXPORT ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        NAMESPACE ${PROJECT_NAME}::
        FILE ${PROJECT_NAME}-config.cmake)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/source/" # source directory
        DESTINATION "include/" # target directory
        FILES_MATCHING
        PATTERN "*.h" # install only matched files
        PATTERN "pch" EXCLUDE) # exclude pch

install(FILES ${EXPORT_FILE_PATH}
        DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${PROJECT_NAME}/export/)

install(FILES ${PROJECT_SOURCE_DIR}/database/pinyin.sqlite3
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/database/)

set(SQLITE_FILE_PATH ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/database/pinyin.sqlite3)
configure_file(config/config.json.in
               ${CONFIGURED_FILES_ROOT}/config/config.json.for_install
               @ONLY)

install(FILES ${CONFIGURED_FILES_ROOT}/config/config.json.for_install
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/config
        RENAME config.json)

set(DEFAULT_CONFIG_FILE_PATH ${CMAKE_INSTALL_FULL_DATADIR}/config.json)
configure_file(source/${PROJECT_NAME}/config/config.h.in
               ${CONFIGURED_FILES_ROOT}/source/${PROJECT_NAME}/config/config.h.for_install
               @ONLY)

install(FILES ${CONFIGURED_FILES_ROOT}/source/${PROJECT_NAME}/config/config.h.for_install
        DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${PROJECT_NAME}/config/
        RENAME config.h)
