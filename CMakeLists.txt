cmake_minimum_required(VERSION 3.0.0)
project(hcore VERSION 1.0.0 LANGUAGES C)

# init

set(CMAKE_DEBUG_POSTFIX -debug)

aux_source_directory(src SOURCE)

# compile

add_library(hcore ${SOURCE})
set_target_properties(hcore PROPERTIES
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    VERSION "1.0.0"
    SOVERSION "1")

configure_file(hcore_config.h.in hcore_config.h)
target_include_directories(hcore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(hcore PRIVATE _HCORE_DEBUG)
endif()

# TODO: test
# include(CTest)
# enable_testing()

include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/HCoreConfig.cmake"
    INSTALL_DESTINATION "lib/cmake/hcore"
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO)

# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/HCoreConfigVersion.cmake"
    VERSION "${hcore_VERSION_MAJOR}.${hcore_VERSION_MINOR}"
    COMPATIBILITY AnyNewerVersion
)

# install
install(TARGETS hcore 
    EXPORT HCORE_ARCHIVE
    ARCHIVE DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/hcore_config.h
    DESTINATION include)
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h")

install(EXPORT HCORE_ARCHIVE
    FILE hcore.cmake
    DESTINATION lib/cmake/hcore)

## install the configuration file
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/HCoreConfig.cmake
    DESTINATION lib/cmake/hcore
    )

# pack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DIRECTORY packages)
include(CPack)