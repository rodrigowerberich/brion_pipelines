cmake_minimum_required(VERSION 3.20)
project(brion_pipelines LANGUAGES CXX VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Wpedantic")
elseif (MSVC)
    set(CMAKE_CXX_FLAGS "/W4 /WX /permissive- /EHsc")
endif()

enable_testing()
add_subdirectory(components)

find_package(Doxygen)

if(DOXYGEN_FOUND)
    configure_file(Doxyfile ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_DINARY_DIR}
        COMMENT "Generating project documentation with Doxygen" VERBATIM
    )
    # add_custom_target(copy-images ALL
    #     COMMAND ${CMAKE_COMMAND} -E copy_directory
    #     ${CMAKE_SOURCE_DIR}/docs_base/images
    #     ${CMAKE_BINARY_DIR}/docs/html/images
    # )
    # ADD_DEPENDENCIES(docs copy-images)
ELSE()
    MESSAGE(WARNING "Doxygen not found, documentation won't be generated.")
ENDIF()