cmake_minimum_required(VERSION 3.10)
project(MilkDrop3)

# Requires C++17 or later depending on usage, but 11 is safe minimum
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set common definitions from the old vcxproj
add_definitions(
    -DWIN32
    -D_WINDOWS
    -DUSE_VIS_HDR_HWND
    -DSTRSAFE_NO_DEPRECATE
    -DNSEEL_REENTRANT_EXECUTION
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_NON_CONFORMING_SWPRINTFS
)

# Set source files
set(SOURCES
    audio/audiobuf.cpp
    audio/guid.cpp
    audio/log.cpp
    audio/loopback-capture.cpp
    audio/prefs.cpp
    ns-eel2/nseel-caltab.c
    ns-eel2/nseel-cfunc.c
    ns-eel2/nseel-compiler.c
    ns-eel2/nseel-eval.c
    ns-eel2/nseel-lextab.c
    ns-eel2/nseel-ram.c
    ns-eel2/nseel-yylex.c
    vis_milk2/dxcontext.cpp
    vis_milk2/fft.cpp
    vis_milk2/menu.cpp
    vis_milk2/Milkdrop2PcmVisualizer.cpp
    vis_milk2/milkdropfs.cpp
    vis_milk2/plugin.cpp
    vis_milk2/pluginshell.cpp
    vis_milk2/state.cpp
    vis_milk2/support.cpp
    vis_milk2/texmgr.cpp
    vis_milk2/textmgr.cpp
    vis_milk2/utility.cpp
    vis_milk2/wasabi.cpp
    vis_milk2/plugin.rc
)

# We want an executable
add_executable(MilkDrop3 WIN32 ${SOURCES})

# Include directories
target_include_directories(MilkDrop3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/ns-eel2
    ${CMAKE_CURRENT_SOURCE_DIR}/vis_milk2
)

# Handle DirectX 9 and Windows libraries
if(WIN32)
    # Search for DirectX SDK from environment variable DXSDK_DIR
    if(DEFINED ENV{DXSDK_DIR})
        file(TO_CMAKE_PATH "$ENV{DXSDK_DIR}" DXSDK_DIR_CMAKE)
        target_include_directories(MilkDrop3 PRIVATE ${DXSDK_DIR_CMAKE}/Include)
        
        # Check architecture to link correct DirectX library path
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_link_directories(MilkDrop3 PRIVATE ${DXSDK_DIR_CMAKE}/Lib/x64)
        else()
            target_link_directories(MilkDrop3 PRIVATE ${DXSDK_DIR_CMAKE}/Lib/x86)
        endif()
    else()
        message(WARNING "DXSDK_DIR environment variable not found. Please install the DirectX SDK (June 2010) or ensure the environment variable is set.")
    endif()

    target_link_libraries(MilkDrop3 PRIVATE
        d3d9
        d3dx9
        Shlwapi
        user32
    )
endif()
