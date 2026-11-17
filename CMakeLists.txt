cmake_minimum_required(VERSION 3.20)

project(PowerSwitcher LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "PowerSwitcher is a Windows-only application.")
endif()

if(MSVC AND CMAKE_GENERATOR MATCHES "Ninja")
    message(WARNING
        "CMake ${CMAKE_VERSION} generates a broken RC rule for Ninja + MSVC "
        "(the command starts with a literal 'RC'). Configure with the Visual Studio "
        "generator instead.")
endif()

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(ICON_SOURCE "${RESOURCE_DIR}/app.ico")

# The resource compiler looks for these files next to the generated app.rc,
# so copy them into the binary directory at configure time.
if(EXISTS "${ICON_SOURCE}")
    configure_file("${ICON_SOURCE}" "${CMAKE_CURRENT_BINARY_DIR}/app.ico" COPYONLY)
    # Referenced relative to app.rc, which is generated in the same directory.
    set(APP_ICON_LINE "IDI_APPICON ICON \"app.ico\"")
else()
    message(WARNING "app.ico not found; building without an embedded icon.")
    set(APP_ICON_LINE "")
endif()

configure_file("${RESOURCE_DIR}/app.manifest"
               "${CMAKE_CURRENT_BINARY_DIR}/app.manifest" COPYONLY)
configure_file("${RESOURCE_DIR}/app.rc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/app.rc" @ONLY)

add_executable(PowerSwitcher WIN32
    "${SOURCE_DIR}/main.cpp"
    "${SOURCE_DIR}/app.cpp"
    "${SOURCE_DIR}/power.cpp"
    "${SOURCE_DIR}/config.cpp"
    "${SOURCE_DIR}/i18n.cpp"
    "${SOURCE_DIR}/ui.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/app.rc"
)

target_include_directories(PowerSwitcher PRIVATE
    "${SOURCE_DIR}"
    "${RESOURCE_DIR}"
)

target_compile_definitions(PowerSwitcher PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(PowerSwitcher PRIVATE
    user32
    gdi32
    shell32
    powrprof
    ole32
    comctl32
)

if(MSVC)
    target_compile_options(PowerSwitcher PRIVATE /W4 /utf-8 /permissive-)
    # Our resource script embeds the manifest; stop the linker from adding a second one.
    target_link_options(PowerSwitcher PRIVATE /MANIFEST:NO)
    set_property(TARGET PowerSwitcher PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_compile_options(PowerSwitcher PRIVATE -O2 -Wall -Wextra -municode)
    target_link_options(PowerSwitcher PRIVATE -static -municode)
endif()

set_target_properties(PowerSwitcher PROPERTIES OUTPUT_NAME "PowerSwitcher")

# Multi-config generators append the configuration name to the output dir by
# default; pin every configuration to the same bin/ folder.
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" CONFIG_UPPER)
    set_target_properties(PowerSwitcher PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/bin")
endforeach()
