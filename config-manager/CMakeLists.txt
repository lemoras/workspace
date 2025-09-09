cmake_minimum_required(VERSION 3.10)
project(ConfigManager C)

set(CMAKE_C_STANDARD 11)

# GTK3 ve JSON-C bulunması
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(JSONC REQUIRED json-c)

include_directories(${GTK3_INCLUDE_DIRS} ${JSONC_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS} ${JSONC_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER} ${JSONC_CFLAGS_OTHER})

add_executable(config_manager main.c)
target_link_libraries(config_manager ${GTK3_LIBRARIES} ${JSONC_LIBRARIES})
