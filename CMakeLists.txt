cmake_minimum_required(VERSION 3.14)
project(TicTacToeDisappear LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Fetch FTXUI ──────────────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG        v5.0.0
)

set(FTXUI_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS    OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(ftxui)

# ── Executable ───────────────────────────────────────────────────────────────
add_executable(tictac tic-tac-toe-diss.cpp)

target_link_libraries(tictac
    PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
)

# ── Platform tweaks ───────────────────────────────────────────────────────────
if(WIN32)
    # Minimal Windows version: 7+
    target_compile_definitions(tictac PRIVATE _WIN32_WINNT=0x0601)
    # Keep as a console app (no WinMain)
    set_target_properties(tictac PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

if(MSVC)
    target_compile_options(tictac PRIVATE /W4 /utf-8)
else()
    target_compile_options(tictac PRIVATE -Wall -Wextra -Wpedantic)
endif()