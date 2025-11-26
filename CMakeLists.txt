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




# cmake_minimum_required(VERSION 3.15)

# project(wxstc-editor VERSION 1.0.0 LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # --- Find wxWidgets (Cross-Platform & Robust) ---

# # Define the components we need. 'stc' is the key one.
# set(wxWidgets_USE_LIBS stc core base)

# # --- Platform-Specific Configuration ---
# if(WIN32)
#     # On Windows, we must specify the toolkit (msw).
#     # This helps FindwxWidgets locate the correct libraries.
#     set(wxWidgets_USE_TOOLKIT msvc)
# elseif(UNIX AND NOT APPLE)
#     # On Linux, we rely on the system's wx-config.
#     # We are explicitly building against the GTK version provided by the package manager.
#     # The previous issue was likely a CMake cache or a fluke with the module.
#     # We'll add a check to ensure the component was found.
#     message(STATUS "Configuring for Linux (GTK)")
# endif()

# # Find the package. REQUIRED will fail if wxWidgets isn't found at all.
# find_package(wxWidgets REQUIRED)

# # --- Defensive Check for Linux STC ---
# # This is the key to preventing the original linker error from happening silently.
# # We explicitly check that the STC library was found before proceeding.
# if(UNIX AND NOT APPLE)
#     list(FIND wxWidgets_LIBRARIES "wx_gtk3u_stc-3.2" STC_FOUND_INDEX)
#     if(STC_FOUND_INDEX EQUAL -1)
#         message(FATAL_ERROR "wxWidgets STC component not found in libraries: ${wxWidgets_LIBRARIES}. "
#                             "Ensure libwxgtk3.2-dev (or equivalent) is installed and provides the STC library.")
#     else()
#         message(STATUS "wxWidgets STC component found successfully.")
#     endif()
# endif()

# # --- Define the Executable ---

# # Include the helper file provided by the FindwxWidgets module.
# # This sets up include directories and compiler preprocessor definitions.
# include(${wxWidgets_USE_FILE})

# add_executable(wxstc-editor
#     src/main.cpp
# )

# # Link our executable against the wxWidgets libraries.
# target_link_libraries(wxstc-editor PRIVATE ${wxWidgets_LIBRARIES})

# # --- Platform-Specific Tweaks ---
# if(WIN32)
#     # On Windows, ensure we build a GUI application to avoid a console window.
#     set_property(TARGET wxstc-editor PROPERTY WIN32_EXECUTABLE TRUE)
# endif()




# cmake_minimum_required(VERSION 3.15)

# project(wxstc-editor VERSION 1.0.0 LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # --- Find wxWidgets Manually ---
# # This bypasses the FindwxWidgets module's component logic and directly
# # uses wx-config, which we know works.

# # 1. Find the wx-config tool itself.
# find_program(wxWidgets_CONFIG_EXECUTABLE wx-config)
# if(NOT wxWidgets_CONFIG_EXECUTABLE)
#     message(FATAL_ERROR "wx-config not found in PATH.")
# endif()
# message(STATUS "Found wx-config at: ${wxWidgets_CONFIG_EXECUTABLE}")

# # 2. Manually execute wx-config to get the exact flags.
# # This directly mimics your working g++ command.
# execute_process(
#     COMMAND ${wxWidgets_CONFIG_EXECUTABLE} --cxxflags
#     OUTPUT_VARIABLE _wx_cxx_flags
#     OUTPUT_STRIP_TRAILING_WHITESPACE
# )

# execute_process(
#     COMMAND ${wxWidgets_CONFIG_EXECUTABLE} --libs core,stc
#     OUTPUT_VARIABLE _wx_link_flags
#     OUTPUT_STRIP_TRAILING_WHITESPACE
# )

# message(STATUS "wxWidgets CXX Flags: ${_wx_cxx_flags}")
# message(STATUS "wxWidgets Link Flags: ${_wx_link_flags}")

# # --- Define the Executable ---

# add_executable(wxstc-editor
#     src/main.cpp
# )

# # --- Apply the flags directly to the target ---

# # The flags are strings. We need to convert them to a CMake list
# # so that each flag (-D, -I, -l, etc.) is treated as a separate item.
# separate_arguments(_wx_cxx_flags)
# separate_arguments(_wx_link_flags)

# # Apply the compiler flags (for include paths and definitions)
# target_compile_options(wxstc-editor PRIVATE ${_wx_cxx_flags})

# # Apply the linker flags (for libraries and library paths)
# target_link_libraries(wxstc-editor PRIVATE ${_wx_link_flags})





# cmake_minimum_required(VERSION 3.15)

# project(wxstc-editor VERSION 1.0.0 LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # --- Find wxWidgets ---

# # 1. Explicitly find the wx-config tool. This is the most reliable
# #    way to ensure CMake is looking at the correct wxWidgets installation.
# find_program(wxWidgets_CONFIG_EXECUTABLE wx-config)
# if(NOT wxWidgets_CONFIG_EXECUTABLE)
#     message(FATAL_ERROR "wx-config not found in PATH. Is wxWidgets installed correctly?")
# endif()
# message(STATUS "Found wx-config at: ${wxWidgets_CONFIG_EXECUTABLE}")

# # 2. Set the components we need. 'stc' automatically pulls in 'core' and 'base'.
# set(wxWidgets_USE_LIBS stc)

# # 3. Now, find the package. It will use the wx-config we found above.
# find_package(wxWidgets REQUIRED)

# # --- DEBUGGING OUTPUT ---
# # This will show us exactly what CMake found.
# message(STATUS "wxWidgets_FOUND: ${wxWidgets_FOUND}")
# message(STATUS "wxWidgets_INCLUDE_DIRS: ${wxWidgets_INCLUDE_DIRS}")
# message(STATUS "wxWidgets_LIBRARIES: ${wxWidgets_LIBRARIES}")
# # --- END DEBUGGING ---

# # --- Define the Executable ---

# # This helper file sets up include directories and compiler flags.
# include(${wxWidgets_USE_FILE})

# add_executable(wxstc-editor
#     src/main.cpp
# )

# # Link our executable against the wxWidgets libraries found above.
# target_link_libraries(wxstc-editor PRIVATE ${wxWidgets_LIBRARIES})


# cmake_minimum_required(VERSION 3.15)

# project(wxstc-editor VERSION 1.0.0 LANGUAGES CXX)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # --- Find wxWidgets ---
# # We need the 'stc' (Styled Text Control) component, which also pulls in 'core'.
# # We also explicitly add 'base' as it's a common dependency.
# # CMake will automatically find the GTK version installed in the container.
# set(wxWidgets_USE_LIBS stc core base)
# find_package(wxWidgets REQUIRED)

# # --- Define the Executable ---
# include(${wxWidgets_USE_FILE})

# add_executable(wxstc-editor
#     src/main.cpp
# )

# target_link_libraries(wxstc-editor PRIVATE ${wxWidgets_LIBRARIES})






# # Require a modern CMake version for good practices
# cmake_minimum_required(VERSION 3.15)

# # Define the project
# project(MyWxProject VERSION 1.0.0 LANGUAGES CXX)

# # Set the C++ standard to C++17
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# # --- Find wxWidgets ---
# # This is the most critical part. We need to tell CMake which wxWidgets
# # components we need and, on Linux, which toolkit to use.

# # For Linux, we explicitly request the Qt toolkit as requested.
# # On Windows, this variable is ignored, and it will find the default (msw) toolkit.
# if(UNIX AND NOT APPLE)
#     set(wxWidgets_USE_LIBS qt core base)
#     message(STATUS "Configuring for Linux with wxQt backend.")
# else()
#     # For Windows and other systems, we use the standard components.
#     # 'core' and 'base' are the most common denominator.
#     # Add others like 'stc', 'aui', 'propgrid' if you need them.
#     set(wxWidgets_USE_LIBS core base)
#     message(STATUS "Configuring for Windows/macOS with default backend.")
# endif()

# # Now, find the package. REQUIRED will make CMake fail if it's not found.
# find_package(wxWidgets REQUIRED)

# # --- Add Subdirectories ---
# # Add the 'src' directory which contains our application's source code.
# add_subdirectory(src)
