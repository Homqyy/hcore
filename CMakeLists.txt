cmake_minimum_required(VERSION 3.0.0)
project(hcore VERSION 1.0 LANGUAGES C)

# compile
aux_source_directory(src SOURCE)
add_library(hcore ${SOURCE})

option(USE_DEBUG "debug mode" ON)
configure_file(hcore_config.h.in hcore_config.h)
target_include_directories(hcore PUBLIC "include" "${PROJECT_BINARY_DIR}")

if(USE_DEBUG)
target_compile_definitions(hcore PUBLIC _HCORE_DEBUG)
endif()

# test
include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# install
install(TARGETS hcore ARCHIVE DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/hcore_config.h DESTINATION include/hcore)
install(DIRECTORY include/ DESTINATION include/hcore FILES_MATCHING PATTERN "*.h")
