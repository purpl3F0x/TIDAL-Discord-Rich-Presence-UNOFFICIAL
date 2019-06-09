cmake_minimum_required(VERSION 3.10)


project(TIDAL-RPC)

include_directories(.)
add_executable(tidal-rpc main.cc)


set(CMAKE_PREFIX_PATH "/usr/local/opt/qt5/lib/cmake/Qt5Widgets/")


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-arcs -ftest-coverage")
target_link_libraries(tidal-rpc Qt5::Widgets)


# generate proper GUI program on specified platform
#if(WIN32) # Check if we are on Windows
#    if(MSVC) # Check if we are using the Visual Studio compiler
#        set_target_properties(tidal-rpc PROPERTIES
#                WIN32_EXECUTABLE YES
#                LINK_FLAGS "/ENTRY:mainCRTStartup"
#                )
#    elseif(CMAKE_COMPILER_IS_GNUCXX)
#        # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows") # Not tested
#    else()
#        message(SEND_ERROR "You are using an unsupported Windows compiler! (Not MSVC or GCC)")
#    endif(MSVC)
#elseif(APPLE)
#    set_target_properties(${PROJECT_NAME} PROPERTIES
#            MACOSX_BUNDLE YES
#            )
#elseif(UNIX)
#    # Nothing special required
#else()
#    message(SEND_ERROR "You are on an unsupported platform! (Not Win32, Mac OS X or Unix)")
#endif(WIN32)


if (WIN32)

    if (MSVC)
        STRING(REPLACE "/Od" "/O2" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    else ()
        set(CMAKE_CXX_FLAGS_RELEASE "-O2 -ffast-math")
    endif ()


    message("Building for Windows")
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64 bits
        message("\t64-bit")
        target_include_directories(tidal-rpc PRIVATE discord-rpc/win64-dynamic/include)
        target_link_libraries(tidal-rpc ${CMAKE_BINARY_DIR}/../discord-rpc/win64-dynamic/lib/discord-rpc.lib)

    elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
        # 32 bits
        message("\t32-bit")
        target_include_directories(tidal-rpc PRIVATE discord-rpc/win32-dynamic/include)
        target_link_libraries(tidal-rpc ${CMAKE_BINARY_DIR}/../discord-rpc/win32-dynamic/lib/discord-rpc.lib)
    endif ()

elseif (APPLE)
    message("Building for MacOS")
    include_directories(discord-rpc/osx-static/include)
    target_link_libraries(tidal-rpc ${CMAKE_BINARY_DIR}/../discord-rpc/osx-static/lib/libdiscord-rpc.a)
    set(CMAKE_CXX_FLAGS "-framework carbon -framework foundation -framework CoreFoundation")
endif ()
