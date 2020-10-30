
# base
find_package(CURL REQUIRED)

# all source files
file(GLOB headers *.h)
file(GLOB sources *.cpp)

list(APPEND third-party CURL::libcurl)

# execute
add_library(${PROJ} STATIC ${headers} ${sources})
target_link_libraries(${PROJ} PRIVATE ${third-party})
target_include_directories(
    ${PROJ}
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}
)
