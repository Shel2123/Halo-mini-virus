cmake_minimum_required(VERSION 3.16)
project(Halo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC
    src/main.cpp
    src/autostart.cpp
    src/window.cpp
    src/app.rc
)
set(HDR
    src/autostart.h
    src/window.h
    src/app_constants.h
    src/win32_common.h
)

add_executable(Halo WIN32 ${SRC} ${HDR})
target_include_directories(Halo PRIVATE src)
target_compile_definitions(Halo PRIVATE UNICODE _UNICODE NOMINMAX)

target_link_libraries(Halo PRIVATE user32 gdi32 shell32 ole32 shlwapi)

if (MSVC)
  target_compile_options(Halo PRIVATE /W4 /EHsc /permissive- /Zc:preprocessor /Zc:inline)
  target_link_options(Halo PRIVATE /OPT:REF /OPT:ICF)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif (MINGW)
  target_compile_options(Halo PRIVATE -municode -ffunction-sections -fdata-sections)
  target_link_options(Halo PRIVATE -municode -static -static-libgcc -static-libstdc++ -Wl,--gc-sections)
  set_target_properties(Halo PROPERTIES LINK_SEARCH_START_STATIC ON LINK_SEARCH_END_STATIC ON)
  target_link_libraries(Halo PRIVATE winpthread)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(Halo PRIVATE -ffunction-sections -fdata-sections)
  target_link_options(Halo PRIVATE -Wl,--gc-sections)
endif()
