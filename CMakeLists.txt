cmake_minimum_required(VERSION 4.1.2)
project(fe3d)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES 
      ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(CMAKE_BUILD_PATH "\$ORIGIN")
set(CMAKE_INSTALL_RPATH "\$ORIGIN/.")

find_package(Doxygen REQUIRED)
if (DOXYGEN_FOUND)
    set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
    set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out")
    set(DOXYGEN_INPUT_DIR
            "${CMAKE_SOURCE_DIR}/dl-loader"
            "${CMAKE_SOURCE_DIR}/fe-api"
            "${CMAKE_SOURCE_DIR}/backends"
    )
    string(REPLACE ";" " " DOXYGEN_INPUT_DIR "${DOXYGEN_INPUT_DIR}")

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
add_library(fe3d-core SHARED fe-api/render/rhi.c)
target_include_directories(dl-loader PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/dl-loader")
target_link_libraries(fe3d-core dl-loader)

file(GLOB TEST_SOURCES tests/*.c)
list(FILTER TEST_SOURCES EXCLUDE REGEX "/_.*\\.c$")
foreach (TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME "${TEST_FILE}" NAME_WE)
    add_executable("${TEST_NAME}" "${TEST_FILE}")
    #if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      target_compile_options("${TEST_NAME}" PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
        -g
      )

      target_link_options("${TEST_NAME}" PRIVATE
        -fsanitize=address
        -rdynamic
      )
      #endif()

    target_link_libraries("${TEST_NAME}" fe3d-core)
    add_test(NAME "${TEST_NAME}" COMMAND "${TEST_NAME}")
endforeach ()

#add_executable(test01 tests/test01.c)
#target_link_libraries(test01 dl-loader)
