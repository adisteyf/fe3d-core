cmake_minimum_required(VERSION 4.3.1)
project(fe3d)

find_package(Doxygen REQUIRED)
if (DOXYGEN_FOUND)
    set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
    set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out")
    configure_file("${DOXYGEN_IN}" "${DOXYGEN_OUT}" @ONLY)
    add_custom_target(
            docs
            COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYGEN_OUT}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "documentation using Doxygen"
            VERBATIM
    )
endif ()

add_library(dl-loader STATIC dl-loader/dl-loader.c)
add_library(fer-backend_debugblank SHARED backends/fer-backend_debugblank.c)
target_include_directories(dl-loader PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/dl-loader")
add_executable(test01 tests/test01.c)
target_link_libraries(test01 dl-loader)
