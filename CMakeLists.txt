cmake_minimum_required(VERSION 3.20)
project(brion_pipelines LANGUAGES CXX VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Werror")
# If we are not on Windows, add -Wextra
if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()

enable_testing()
add_subdirectory(components)

# FIND_PACKAGE(Doxygen)

# IF(DOXYGEN_FOUND)
#     CONFIGURE_FILE(Doxyfile ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
#     ADD_CUSTOM_TARGET(docs
#         ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
#         WORKING_DIRECTORY ${CMAKE_DINARY_DIR}
#         COMMENT "Generating project documentation with Doxygen" VERBATIM
#     )
#     # add_custom_target(copy-images ALL
#     #     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     #     ${CMAKE_SOURCE_DIR}/docs_base/images
#     #     ${CMAKE_BINARY_DIR}/docs/html/images
#     # )
#     # ADD_DEPENDENCIES(docs copy-images)
# ELSE()
#     MESSAGE(WARNING "Doxygen not found, documentation won't be generated.")
# ENDIF()