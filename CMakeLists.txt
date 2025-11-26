cmake_minimum_required(VERSION 3.15)

project(wxstc-editor VERSION 1.0.0 LANGUAGES CXX)

option(QT_SWITCH "Enable wxQt" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Find wxWidgets (with a pragmatic fix) ---

# Define the components we need.
set(wxWidgets_USE_LIBS qt stc core base)

# Find the package. This will set up include paths, definitions, etc. correctly.
find_package(wxWidgets REQUIRED)

# --- LINUX-SPECIFIC FIX FOR BROKEN STC DETECTION ---
# This is a targeted fix for the bug where CMake's FindwxWidgets module
# fails to find the 'stc' component on some Linux installations.
if(UNIX AND NOT APPLE)
    # Check if the STC library is missing from the list CMake found.
    # We search for the substring "stc" to be robust against library name variations.
    string(FIND "${wxWidgets_LIBRARIES}" "stc" STC_FOUND_INDEX)
    if(STC_FOUND_INDEX EQUAL -1)
        message(STATUS "CMake's FindwxWidgets did not find the STC component. Applying a manual fix...")
        
        # Find the wx-config tool.
        find_program(wxWidgets_CONFIG_EXECUTABLE wx-config)
        if(NOT wxWidgets_CONFIG_EXECUTABLE)
            message(FATAL_ERROR "wx-config not found in PATH.")
        endif()

        set(WX_CONFIG_LIBS_ARGS "core" "stc")

        if(QT_SWITCH)
            list(APPEND WX_CONFIG_LIBS_ARGS "qt")
        else()
            message(STATUS "wxQt disabled, using GTK")
        endif()

        string(JOIN "," WX_CONFIG_LIBS_ARGS_JOINED "${WX_CONFIG_LIBS_ARGS}")

        # Manually get the correct linker flags from wx-config, just like the working g++ command.
        execute_process(
            COMMAND ${wxWidgets_CONFIG_EXECUTABLE} --libs ${WX_CONFIG_LIBS_ARGS_JOINED}
            OUTPUT_VARIABLE _wx_link_flags_override
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        # Convert the space-separated string into a CMake list.
        separate_arguments(_wx_link_flags_override)

        # Override the broken wxWidgets_LIBRARIES variable with the correct one.
        set(wxWidgets_LIBRARIES ${_wx_link_flags_override})
        
        message(STATUS "Successfully overrode wxWidgets_LIBRARIES with: ${wxWidgets_LIBRARIES}")
    endif()
endif()

# --- Define the Executable ---

# This helper file sets up include directories and compiler flags.
include(${wxWidgets_USE_FILE})

# include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(wxstc-editor
    src/main.cpp
    src/HttpEditor.cpp
)

# Link our executable against the (now corrected) wxWidgets libraries.
target_link_libraries(wxstc-editor PRIVATE ${wxWidgets_LIBRARIES})
target_include_directories(wxstc-editor PRIVATE include)

# --- Platform-Specific Tweaks ---
if(WIN32)
    set_property(TARGET wxstc-editor PROPERTY WIN32_EXECUTABLE TRUE)
endif()
