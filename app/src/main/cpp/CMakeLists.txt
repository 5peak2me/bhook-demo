# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("bhookdemo")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        bhookdemo

        # Sets the library as a shared library.
        SHARED
        thread_hook.cpp
        # Provides a relative path to your source file(s).
        native-lib.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

find_package(bytehook REQUIRED CONFIG)
target_link_libraries(bhookdemo bytehook::bytehook)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        bhookdemo
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

# libhack.so
add_library(detour SHARED detour.c)
target_compile_features(detour PUBLIC c_std_11)
target_compile_options(detour PUBLIC -Weverything -Werror)
target_link_libraries(detour bytehook::bytehook log)