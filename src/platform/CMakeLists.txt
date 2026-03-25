# src/platform/CMakeLists.txt

target_sources(FractalEngineLib PRIVATE
    Platform.cpp
)

if (WIN32)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/win32/Win32Platform.cpp")
        target_sources(FractalEngineLib PRIVATE win32/Win32Platform.cpp)
    else()
        message(WARNING " [platform] win32/Win32Platform.cpp não encontrado — ignorando.")
    endif()
elseif (UNIX AND NOT APPLE)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/linux/LinuxPlatform.cpp")
        target_sources(FractalEngineLib PRIVATE linux/LinuxPlatform.cpp)
    endif()
elseif (APPLE)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/macos/MacPlatform.cpp")
        target_sources(FractalEngineLib PRIVATE macos/MacPlatform.cpp)
    endif()
endif()